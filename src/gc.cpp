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

/*
 * Lock the gc mutex.
 */
__force_inline void gc_lock(){
	pthread_mutex_lock( &__gc.mutex );
}
/*
 * Unlock the gc mutex.
 */
__force_inline void gc_unlock(){
	pthread_mutex_unlock( &__gc.mutex );
}
/*
 * Append 'item' to 'list'.
 */
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
/*
 * Remove 'item' from 'list' and free the structure that holds it.
 */
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
/*
 * Move 'item' from 'src' list to 'dst' list.
 */
__force_inline void gc_pool_migrate( gc_list_t *src, gc_list_t *dst, gc_item_t *item ) {
	if( item->prev == NULL ){
		src->head = item->next;
	}
	else{
		item->prev->next = item->next;
	}
	if( item->next == NULL ){
		src->tail = item->prev;
	}
	else{
		item->next->prev = item->prev;
	}

	if( dst->head == NULL ){
		dst->head = item;
		item->prev = NULL;
	}
	else{
		dst->tail->next = item;
		item->prev 	    = dst->tail;
	}

	dst->tail  = item;
	item->next = NULL;

	src->items--;
	src->usage -= item->size;
	dst->items++;
	dst->usage += item->size;
}
/*
 * Free 'item' and remove it from 'list'.
 */
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
/*
 * Set collection threshold.
 */
size_t gc_set_collect_threshold( size_t threshold ){
	size_t old = __gc.gc_threshold;

	gc_lock();
	__gc.gc_threshold = threshold;
	gc_unlock();

	return old;
}
/*
 * Set allowed memory threshold.
 */
size_t gc_set_mm_threshold( size_t threshold ){
	size_t old = __gc.mm_threshold;

	gc_lock();
	__gc.mm_threshold = threshold;
	gc_unlock();

	return old;
}
/*
 * Add an object to the gc pool and start to track
 * it for reference changes.
 * Size must be passed explicitly due to the downcasting
 * possibility.
 */
struct _Object *gc_track( struct _Object *o, size_t size ){
	/*
	 * We assume that 'o' was previously allocated with one of the gc_new_*
	 * macros, therefore, if its pointer is null, most of it there was a memory
	 * allocation problem.
	 */
	if( o == NULL ){
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
    gc_pool_append( &__gc.heap, new gc_item_t(o,size) );

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
/*
 * Recursively mark an object (and its inner items) as alive.
 */
void gc_mark( Object *o ){
	/*
	 * Already marked?
	 */
	if( !o->gc_mark ){
		/*
		 * This object is not collectable right now.
		 */
		GC_SET_ALIVE(o);
		/*
		 * Loop all the objects it 'contains' (such as vector items) and
		 * call gc_mark recursively on each one.
		 */
		Object *child = NULL;
		int i = 0;
		while( (child = ob_traverse( o, i++ )) != NULL ){
			gc_mark(child);
		}
	}
}
/*
 * Sweep dead objects from a given generation list.
 */
void gc_sweep_generation( gc_list_t *generation ){
	gc_item_t *item;
	Object    *o;

	/*
	 * Loop each object on the generation list.
	 */
	for( item = generation->head; item; item = item->next ){
		o = item->pobj;
		/*
		 * Constant object, move it from the heap structure to the
		 * appropriate one to be freed at the end.
		 * This will make heap list less fragmented and smaller.
		 */
		if( (o->attributes & H_OA_CONSTANT) == H_OA_CONSTANT ){
			#ifdef GC_DEBUG
				fprintf( stdout, "[GC DEBUG] Migrating %p [%s] to constants list.\n", item->pobj, item->pobj->type->name );
			#endif

			gc_pool_migrate( generation, &__gc.constants, item );
		}
		else{
			/*
			 * This object was marked as alive so it's not garbage.
			 * Reset its gc_marked flag to false.
			 */
			if( o->gc_mark ){
				GC_RESET(o);
				/*
				 * If this generation is not the lag space, check if the object
				 * has to be moved to the lag space.
				 */
				if( generation != &__gc.lag && GC_IS_LAGGING(++item->gc_count) ){
					#ifdef GC_DEBUG
						fprintf( stdout, "[GC DEBUG] Migrating %p (collected %d times) to the lag space.\n", item->pobj, item->gc_count );
					#endif
					/*
					 * Migrate the object.
					 */
					gc_pool_migrate( generation, &__gc.lag, item );
				}
			}
			/*
			 * Object not marked (dead object).
			 * This object is not reachable anymore from any of the memory frames,
			 * therefore is garbage and will be freed.
			 */
			else{
				#ifdef GC_DEBUG
					fprintf( stdout, "[GC DEBUG] Releasing %p [%s] .\n", item->pobj, item->pobj->type->name );
				#endif

				gc_free( generation, item );
			}
		}
	}
}

/*
 * The main collection routine.
 */
void gc_collect( vm_t *vm ){
    /**
     * Execute garbage collection loop only if used memory has reaced the
     * threshold.
     */
    if( __gc.usage >= __gc.gc_threshold ){
		std::list<vframe_t *>::iterator i;
    	vframe_t *frame;
    	size_t j, size;

    	/*
    	 * Lock the virtual machine to prevent new frames to be added.
    	 */
    	vm_mm_lock( vm );

		#ifdef GC_DEBUG
			printf( "[GC DEBUG] GC quota (%d bytes) reached with %d bytes, collecting ...\n", __gc.gc_threshold, __gc.usage );
		#endif

		/*
		 * Loop each active memory frame and mark alive objects.
		 */
		for( i = vm->frames.begin(); i != vm->frames.end(); i++ ){
			frame = *i;
			size  = frame->size();
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
		 * New collection, increment global collections counter.
		 */
		__gc.collections++;
		/*
		 * The lag space is bigger than the heap, let's sweep it first.
		 */
		if( __gc.lag.items > __gc.heap.items ){
			#ifdef GC_DEBUG
				fprintf( stdout, "[GC DEBUG] Lag space (%d items) is bigger than heap space (%d items), collecting it.\n",
								 __gc.lag.items,
								 __gc.heap.items );
			#endif

			gc_sweep_generation( &__gc.lag );
		}
		/*
		 * Sweep younger objects in the heap space.
		 */
		gc_sweep_generation( &__gc.heap );

		/*
		 * Unlock the virtual machine frames vector.
		 */
		vm_mm_unlock( vm );
    }
}
/*
 * Release every object (heap objects and constants), called
 * when program ends.
 */
void gc_release(){
	gc_item_t *item;
	int i;

	gc_lock();

	/*
	 * Free every object in the heap.
	 */
	for( item = __gc.heap.head; item; item = item->next ){
		gc_free( &__gc.heap, item );
	}
	/*
	 * Free every object in the lag space.
	 */
	for( item = __gc.lag.head; item; item = item->next ){
		gc_free( &__gc.lag, item );
	}
	/*
	 * Free every constant object if any.
	 */
	for( item = __gc.constants.head; item; item = item->next ){
		gc_free( &__gc.constants, item );
	}
	gc_unlock();
}
