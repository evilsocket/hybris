/*
 * This file is part of the Hybris programming language interpreter.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * Hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "common.h"
#include "gc.h"

extern void hyb_error( H_ERROR_TYPE type, const char *format, ... );

/*
 * The main garbage collector global structure.
 */
gc_t __gc;

__force_inline void gc_lock(){
	#ifdef MT_SUPPORT
	pthread_mutex_lock( &__gc.mutex );
	#endif
}

__force_inline void gc_unlock(){
	#ifdef MT_SUPPORT
	pthread_mutex_unlock( &__gc.mutex );
	#endif
}

__force_inline void gc_pool_append( gc_item_t *item ){
	if( __gc.pool_head == NULL ){
		__gc.pool_head = item;
		item->prev     = NULL;
	}
	else{
		__gc.pool_tail->next = item;
		item->prev 			 = __gc.pool_tail;
	}

	__gc.pool_tail = item;
	item->next 	   = NULL;
}

__force_inline void gc_pool_remove( gc_item_t *item ) {
	if( item->prev == NULL ){
		__gc.pool_head = item->next;
	}
	else{
		item->prev->next = item->next;
	}
	if( item->next == NULL ){
		__gc.pool_tail = item->prev;
	}
	else{
		item->next->prev = item->prev;
	}
	delete item;
}


void gc_free( gc_item_t *item ){
    #ifdef MEM_DEBUG
        printf( "[MEM DEBUG] Releasing %p [%s] [%d bytes] [%d references].\n",
        		item->pobj,
        		item->pobj->type->name,
        		ob_get_size(item->pobj),
        		item->pobj->ref );
    #endif

    __gc.items--;
    __gc.usage -= item->size;

    /*
     * If the object is a collection, ob_free is needed to
     * decrement its items reference counter too so, if they were
     * referenced only by this collection they will be freed upon
     * the next gc_collect trigger.
     */
	ob_free( item->pobj );
	/*
	 * Finally delete the object pointer itself.
	 */
    delete item->pobj;
    /*
     * And remove the item from the gc pool.
     */
    gc_pool_remove(item);
}

size_t gc_set_threshold( size_t threshold ){
	size_t old = __gc.threshold;

	gc_lock();
	__gc.threshold = threshold;
	gc_unlock();

	return old;
}

struct _Object *gc_track( struct _Object *o, size_t size ){
    if( o == NULL ){
        /*
         * We assume that 'o' was previously allocated with one of the gc_new_*
         * macros, therefore, if its pointer is null, most of it there was a memory
         * allocation problem.
         */
        hyb_error( H_ET_GENERIC, "out of memory" );
    }

    gc_lock();

    /*
     * Increment item number and memory usage counters.
     */
    __gc.items++;
    __gc.usage += size;
    /*
     * Append the item to the gc pool.
     */
    gc_pool_append( new gc_item_t(o,size) );

    gc_unlock();

    return o;
}

size_t gc_mm_items(){
	return __gc.items;
}

size_t gc_mm_usage(){
	return __gc.usage;
}

size_t gc_mm_threshold(){
	return __gc.threshold;
}

void gc_collect(){
    /**
     * Execute garbage collection loop only if used memory has reaced the
     * threshold.
     */
    if( __gc.usage >= __gc.threshold && !__gc.collecting ){
    	gc_lock();
    	__gc.collecting = 1;

		#ifdef MEM_DEBUG
			printf( "[MEM DEBUG] GC quota (%d bytes) reached with %d bytes, collecting ...\n", __gc.threshold, __gc.usage );
		#endif
        gc_item_t *item;
        for( item = __gc.pool_head; item; item = item->next ){
			/*
			 * Skip constant objects because they are part of the execution tree nodes.
			 */
			struct _Object *o = item->pobj;
			if( (o->attributes & H_OA_CONSTANT) != H_OA_CONSTANT ){
				/*
				 * Skip objects that are still referenced somewhere.
				 */
				if( o->ref <= 0 ){
					/**
					 * Finally execute garbage collection
					 */
					gc_free( item );
				}
			}
        }
		__gc.collecting = 0;
		gc_unlock();
    }
}

void gc_release(){
	gc_item_t *item;

	gc_lock();
	for( item = __gc.pool_head; item; item = item->next ){
        gc_free( item );
    }
	gc_unlock();
}
