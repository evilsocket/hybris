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
#include "vm.h"

extern void hyb_error( H_ERROR_TYPE type, const char *format, ... );

/*
 * The main garbage collector global structure.
 */
gc_t __gc;

__force_inline void gc_lock(){
	pthread_mutex_lock( &__gc.mutex );
}

__force_inline void gc_unlock(){
	pthread_mutex_unlock( &__gc.mutex );
}

__force_inline void gc_pool_append( gc_list_t *list, gc_item_t *item ){
	if( list->head == NULL ){
		list->head = item;
		item->prev = NULL;
	}
	else{
		list->tail->next = item;
		item->prev 	     = list->tail;
	}

	list->tail = item;
	item->next = NULL;

	list->items++;
	list->usage += item->size;
}

__force_inline void gc_pool_remove( gc_list_t *list, gc_item_t *item ) {
	if( item->prev == NULL ){
		list->head = item->next;
	}
	else{
		item->prev->next = item->next;
	}
	if( item->next == NULL ){
		list->tail = item->prev;
	}
	else{
		item->next->prev = item->prev;
	}

	list->items--;
	list->usage -= item->size;

	delete item;
}


void gc_free( gc_list_t *list, gc_item_t *item ){
    __gc.items--;
    __gc.usage -= item->size;

    /*
     * If the object is a collection, ob_free is needed to free its elements,
     * because gc_free isn't applied recursively on each object as gc_mark, so
     * basically each root object has to deallocate its elements if any.
     */
	ob_free( item->pobj );
	/*
	 * Finally delete the object pointer itself.
	 */
    delete item->pobj;
    /*
     * And remove the item from the gc pool.
     */
    gc_pool_remove( list, item );
}

size_t gc_set_collect_threshold( size_t threshold ){
	size_t old = __gc.gc_threshold;

	gc_lock();
	__gc.gc_threshold = threshold;
	gc_unlock();

	return old;
}

size_t gc_set_mm_threshold( size_t threshold ){
	size_t old = __gc.mm_threshold;

	gc_lock();
	__gc.mm_threshold = threshold;
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
    /*
     * Check if maximum memory usage is reached.
     */
    else if( __gc.usage >= __gc.mm_threshold ){
    	hyb_error( H_ET_GENERIC, "Reached max allowed memory usage (%d bytes)", __gc.mm_threshold );
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
    gc_pool_append( &__gc.list, new gc_item_t(o,size) );

    gc_unlock();

    return o;
}

size_t gc_mm_items(){
	return __gc.items;
}

size_t gc_mm_usage(){
	return __gc.usage;
}

size_t gc_collect_threshold(){
	return __gc.gc_threshold;
}

size_t gc_mm_threshold(){
	return __gc.mm_threshold;
}

void gc_mark( Object *o ){
	/*
	 * Already marked?
	 */
	if( !o->gc_mark ){
		/*
		 * This object is not collectable right now.
		 */
		GC_SET_REFERENCED(o);
		/*
		 * Loop all the objects it 'contains' (such as vector items) and
		 * call gc_mark recursively on each one.
		 */
		Object *referenced = NULL;
		int i = 0;
		while( (referenced = ob_traverse( o, i++ )) != NULL ){
			gc_mark(referenced);
		}
	}
}

void gc_collect( vm_t *vm ){
    /**
     * Execute garbage collection loop only if used memory has reaced the
     * threshold.
     */
    if( __gc.usage >= __gc.gc_threshold ){
    	/*
    	 * Lock the virtual machine to prevent new frames to be added.
    	 */
    	vm_mm_lock( vm );

		#ifdef MEM_DEBUG
			printf( "[MEM DEBUG] GC quota (%d bytes) reached with %d bytes, collecting ...\n", __gc.gc_threshold, __gc.usage );
		#endif

		/*
		 * Loop each active memory frame.
		 */
		list<vframe_t *>::iterator i;
		for( i = vm->frames.begin(); i != vm->frames.end(); i++ ){
			vframe_t *frame = *i;
			size_t j, size( frame->size() );
			/*
			 * Loop each object defined into this frame.
			 */
			for( j = 0; j < size; ++j ){
				/*
				 * Mark the object and its referenced objects as live objects.
				 */
				gc_mark( frame->at(j) );
			}
		}

		/*
		 * Loop each object on the heap.
		 */
		gc_item_t *item;
		for( item = __gc.list.head; item; item = item->next ){
			/*
			 * Skip constant objects because they are part of the execution tree nodes.
			 */
			struct _Object *o = item->pobj;
			if( (o->attributes & H_OA_CONSTANT) != H_OA_CONSTANT ){
				/*
				 * This object was marked so it's not garbage.
				 * Reset its gc_marked flag to false.
				 */
				if( o->gc_mark ){
					GC_RESET(o);
				}
				/*
				 * Not marked object!
				 * This object is not reachable anymore from any of the memory frames,
				 * therefore is garbage and will be freed.
				 */
				else{
					#ifdef MEM_DEBUG
						fprintf( stdout, "[MEM DEBUG] Releasing %p [%s] .\n", item->pobj, item->pobj->type->name );
					#endif

					gc_free( &__gc.list, item );
				}
			}
		}
		/*
		 * Unlock the virtual machine frames vector.
		 */
		vm_mm_unlock( vm );
    }
}

void gc_release(){
	gc_item_t *item;
	int i;

	gc_lock();

	for( item = __gc.list.head; item; item = item->next ){
		gc_free( &__gc.list, item );
	}

	gc_unlock();
}
