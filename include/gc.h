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
#ifndef _HGC_H_
#	define _HGC_H_

#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "llist.h"
#include "config.h"


/*
 * Force specified functions to be inlined by the compiler.
 */
#ifndef INLINE
#	define INLINE __inline__ __attribute__((always_inline))
#endif

typedef unsigned long ulong;

/*
 * This is a mark-and-sweep garbage collector implementation.
 *
 * When the global memory usage is >= the threshold defined
 * down below, or with command line parameter, the gc will be
 * triggered and will mark all alive objects (the ones in alive
 * memory frames) and their children if any (for collections),
 * then it will loop every object in the heap and free unmarked
 * objects.
 *
 * NOTE:
 * To make this work, ALL the new objects should be passed
 * instantly to the gc_track function, so use the gc_new_* macros instead
 * of new operator or malloc C function to create an object.
 * No manually deletion is necessary.
 */
#define GC_DEFAULT_MEMORY_THRESHOLD 2048000
/*
 * Maximum allowed memory size usage, if this threshold is reached
 * by __gc.usage counter, a fatal error will be triggered.
 *
 * Default value: 128M
 */
#define GC_ALLOWED_MEMORY_THRESHOLD	134217728

typedef struct _Object Object;
typedef struct _vm_t   vm_t;
/*
 * Threshold upon which an object is moved from the heap
 * space to the lag space, where 0.7 is 70% object collections
 * on the total collections counter.
 */
#define GC_LAGGING_THRESHOLD 		  0.7f
/*
 * Determine if an object has to be moved to the lag space.
 */
#define GC_IS_LAGGING(v)     		  v / (double)__gc.collections >= GC_LAGGING_THRESHOLD
/*
 * Main gc structure, kind of the "head" of the pool.
 *
 * constants    : Constant objects list (will be freed at the end).
 * lag			: When an object remains alive in the heap for a given
 * 				  amount of collections, it's going to be moved to this
 * 				  lag space.
 * heap         : Heap objects list.
 * collections  : Collection cycles counter.
 * usage	    : Global memory usage, in bytes.
 * gc_threshold : If usage >= this, the gc is triggered.
 * mm_threshold : If usage >= this, a memory exhausted error is triggered.
 * mutex        : Mutex to lock the pool while collecting.
 */
typedef struct _gc {
	llist_t			constants;
	llist_t			lag;
	llist_t			heap;
	size_t			collections;
    size_t     		usage;
    size_t     		gc_threshold;
    size_t			mm_threshold;
	pthread_mutex_t mutex;

	_gc(){
		collections  = 0;
		usage        = 0;
		gc_threshold = GC_DEFAULT_MEMORY_THRESHOLD;
		mm_threshold = GC_ALLOWED_MEMORY_THRESHOLD;
		//mutex        = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init( &mutex, NULL );

		ll_init( &constants );
		ll_init( &lag );
		ll_init( &heap );
	}
}
gc_t;

/*
 * Set the 'gc_threshold' attribute of the gc structure.
 * Return the old threshold value.
 */
size_t			gc_set_collect_threshold( size_t threshold );
/*
 * Set the 'mm_threshold' attribute of the gc structure.
 * Return the old threshold value.
 */
size_t			gc_set_mm_threshold( size_t threshold );
/* 
 * Add an object to the gc pool and start to track
 * it for reference changes.
 * Size must be passed explicitly due to the downcasting
 * possibility.
 */
Object 		   *gc_track( Object *o, size_t size );
/*
 * Return the number of objects tracked by the gc.
 */
size_t			gc_mm_items();
/*
 * Return the actual memory usage in bytes.
 */
size_t          gc_mm_usage();
/*
 * Return the threshold value upon which the gc the collect routine
 * will be triggered.
 */
size_t			gc_collect_threshold();
/*
 * Return the maximum allowed memory usage threshold.
 */
size_t			gc_mm_threshold();
/*
 * Recursively mark an object (and its inner items).
 */
void 			gc_mark( Object *o, bool mark = true );
/*
 * Set the object and all the objects referenced by him
 * as non collectable.
 */
#define			gc_set_alive(o) gc_mark( o, true )
/*
 * Set the object and all the objects referenced by him
 * as collectable.
 */
#define 		gc_set_dead(o)  gc_mark( o, false )
/*
 * Fire the collection routines if the memory usage is
 * above the threshold.
 */
void            gc_collect( vm_t *vm );
/*
 * Release all the pool and its contents, should be
 * used when the program is exiting, not before.
 */
void            gc_release();

/*
 * Object allocation macros.
 *
 * 1 .: Alloc new specialized type pointer.
 * 2 .: Downcast to Object * and let the gc track it.
 * 3 .: Upcast back to specialized type pointer and return to user.
 */
#define gc_new_boolean(v)    (Boolean *)   gc_track( (Object *)( new Boolean( static_cast<bool>(v) ) ), 	 sizeof(Boolean) )
#define gc_new_integer(v)    (Integer *)   gc_track( (Object *)( new Integer( static_cast<long>(v) ) ), 	 sizeof(Integer) )
#define gc_new_alias(v)      (Alias *)     gc_track( (Object *)( new Alias( static_cast<long>(v) ) ),   	 sizeof(Alias) )
#define gc_new_extern(v)     (Extern *)    gc_track( (Object *)( new Extern( static_cast<long>(v) ) ),  	 sizeof(Extern) )
#define gc_new_float(v)      (Float *)     gc_track( (Object *)( new Float( static_cast<double>(v) ) ), 	 sizeof(Float) )
#define gc_new_char(v)       (Char *)      gc_track( (Object *)( new Char( static_cast<char>(v) ) ),    	 sizeof(Char) )
#define gc_new_string(v)     (String *)    gc_track( (Object *)( new String( (char *)(v) ) ),           	 sizeof(String) )
#define gc_new_binary(d)     (Binary *)    gc_track( (Object *)( new Binary(d) ),                       	 sizeof(Binary) )
#define gc_new_vector()      (Vector *)    gc_track( (Object *)( new Vector() ),                        	 sizeof(Vector) )
#define gc_new_map()         (Map *)       gc_track( (Object *)( new Map() ),                           	 sizeof(Map) )
#define gc_new_struct()      (Structure *) gc_track( (Object *)( new Structure() ),                     	 sizeof(Structure) )
#define gc_new_class()       (Class *)     gc_track( (Object *)( new Class() ),                              sizeof(Class) )
#define gc_new_reference(o)  (Reference *) gc_track( (Object *)( new Reference(o) ),                         sizeof(Reference) )
#define gc_new_handle(o)     (Handle *)    gc_track( (Object *)( new Handle( reinterpret_cast<void *>(o)) ), sizeof(Handle) )

#endif
