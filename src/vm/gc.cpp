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
/*
 * The main garbage collector global structure.
 */
static gc_t __gc;

#ifdef GC_DEBUG
#	define DEBUG( fmt, ... ) fprintf( stdout, fmt, __VA_ARGS__ )
#else
#	define DEBUG //
#endif

/*
 * Lock the gc mutex.
 */
INLINE void gc_lock(){
	pthread_mutex_lock( &__gc.mutex );
}
/*
 * Unlock the gc mutex.
 */
INLINE void gc_unlock(){
	pthread_mutex_unlock( &__gc.mutex );
}
/*
 * Free 'item' and remove it from 'list'.
 */
void gc_free( llist_t *list, ll_item_t *item ){
	Object *obj = ll_data( Object *, item );

    __gc.usage -= obj->gc_size;
    /*
     * If the object is a collection, ob_free is needed to free its elements,
     * because gc_free isn't applied recursively on each object as gc_mark, so
     * basically each root object has to deallocate its elements if any.
     */
	ob_free( obj );
	/*
	 * Finally delete the object pointer itself.
	 */
	delete obj;
    /*
     * Remove the item from the gc pool.
     */
	ll_remove( list, item );
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
Object *gc_track( Object *o, size_t size ){
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

    DEBUG( "[GC DEBUG] Tracking new object at %p [%d bytes].\n", o, size );

    /*
     * Increment memory usage counter.
     */
    __gc.usage += size;
    /*
     * Update the gc_size inner descriptor.
     */
    o->gc_size = size;

	ll_append( &__gc.heap, o );

    gc_unlock();

    return o;
}

size_t gc_mm_items(){
	return __gc.heap.items + __gc.lag.items + __gc.constants.items;
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
 * Recursively mark an object (and its inner items).
 */
void gc_mark( Object *o, bool mark /*= true*/ ){
	if( o ){
		DEBUG( "[GC DEBUG] Marking object at %p as %s.\n", o, (mark ? "alive" : "dead") );

		/*
		 * Mark the object.
		 */
		o->gc_mark = mark;
		/*
		 * Loop all the objects it 'contains' (such as vector items) and
		 * call gc_mark recursively on each one.
		 */
		Object *child;
		int i;
		for( i = 0, child = NULL; (child = ob_traverse( o, i )) != NULL; ++i ){
			gc_mark( child, mark );
		}
	}
}
/*
 * Sweep dead objects from a given generation list.
 */
void gc_sweep_generation( llist_t *generation ){
	ll_item_t *ll_item = generation->head,
			  *ll_next;
	Object    *o;

	/*
	 * Loop each object on the generation list.
	 */
	while(ll_item) {
		/*
		 * Probably one or more objects are going to be removed from the list,
		 * so we need to save the 'next' instance of the item now to be used
		 * at the end of the loop to switch the pointers.
		 */
		ll_next = ll_item->next;
		o 		= ll_data( Object *, ll_item );

		/*
		 * Constant object, move it from the heap structure to the
		 * appropriate one to be freed at the end.
		 * This will make heap list less fragmented and smaller.
		 */
		if( (o->attributes & H_OA_CONSTANT) == H_OA_CONSTANT ){
			DEBUG( "[GC DEBUG] Migrating %p [%s] to constants list.\n", o, ob_typename(o) );

			ll_move( generation, &__gc.constants, ll_item );
		}
		else{
			/*
			 * This object was marked as alive so it's not garbage.
			 * Reset its gc_marked flag to false.
			 */
			if( o->gc_mark ){
				o->gc_mark = false;
				/*
				 * If this generation is not the lag space, check if the object
				 * has to be moved to the lag space.
				 */
				if( generation != &__gc.lag && GC_IS_LAGGING(++o->gc_count) ){
					DEBUG( "[GC DEBUG] Migrating %p (collected %d times) to the lag space.\n", o, o->gc_count );
					/*
					 * Migrate the object.
					 */
					ll_move( generation, &__gc.lag, ll_item );
				}
			}
			/*
			 * Object not marked (dead object).
			 * This object is not reachable anymore from any of the memory frames,
			 * therefore is garbage and will be freed.
			 */
			else{
				DEBUG( "[GC DEBUG] Releasing %p [%s] .\n", o, ob_typename(o) );

				gc_free( generation, ll_item );
			}
		}

		ll_item = ll_next;
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
		ll_item_t *item;
    	vframe_t  *frame;
    	size_t j, size;

    	/*
    	 * Lock the virtual machine to prevent new frames to be added.
    	 */
    	vm_mm_lock( vm );
    	/*
    	 * Get the scope to collect.
    	 */
    	vm_scope_t *scope = vm_find_scope(vm);

    	DEBUG( "[GC DEBUG] GC quota (%d bytes) reached with %d bytes, collecting thread %p scope ...\n", __gc.gc_threshold, __gc.usage, pthread_self() );

		/*
		 * Loop each active main memory frame and mark alive objects.
		 */
		for( item = scope->head; item; item = item->next ){
			frame = ll_data( vframe_t *, item );
			size  = frame->size();
			/*
			 * Loop each object defined into this frame.
			 */
			for( j = 0; j < size; ++j ){
				/*
				 * Mark the object and its referenced objects as live objects.
				 */
				gc_set_alive( frame->at(j) );
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
			DEBUG( "[GC DEBUG] Lag space (%d items) is bigger than heap space (%d items), collecting it.\n", __gc.lag.items, __gc.heap.items );

			gc_sweep_generation( &__gc.lag );
		}
		/*
		 * Sweep younger objects in the heap space.
		 */
		gc_sweep_generation( &__gc.heap );

		DEBUG( "[GC DEBUG] Garbage collection cycle done, %d collections done.\n", __gc.collections );

		/*
		 * Unlock the virtual machine frames vector.
		 */
		vm_mm_unlock( vm );
    }
}

INLINE void gc_free_generation( llist_t *generation ){
	ll_item_t *ll_item = generation->head,
			  *ll_next;

	while(ll_item) {
		/*
		 * Objects are going to be removed from the list,
		 * so we need to save the 'next' instance of the item now to be used
		 * at the end of the loop to switch the pointers.
		 */
		ll_next = ll_item->next;

		gc_free( generation, ll_item );

		ll_item = ll_next;
	}
}

/*
 * Release every object (heap objects and constants), called
 * when program ends.
 */
void gc_release(){
	ll_item_t *item;

	gc_free_generation( &__gc.heap );
	gc_free_generation( &__gc.lag );
	gc_free_generation( &__gc.constants );
}
