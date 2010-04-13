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

#include "config.h"
#include <memory.h>
#include <stdio.h>
#ifdef MT_SUPPORT
#	include <pthread.h>
#endif

/*
 * This is the new (and hopefully the last one) garbage
 * collector implementation.
 * The algorithm it's easy and to its job, simply tracks
 * every referencing to an object or deferencing to it,
 * thant increments or decrements the 'ref' value of the
 * object_type_t structure.
 *
 * When the global memory usage is >= the threshold down 
 * below, the gc will be triggered and will loop the entire 
 * allocated objects collection searching for objects with
 * "ref" <= 0 that are not constants, then the gc dealloc's
 * them and removes them from its pool.
 *
 * NOTE:
 * To make this work, ALL the new objects should be passed
 * instantly to the gc_track function.
 * No manually deletion is necessary.
 */
#define GC_DEFAULT_MEMORY_THRESHOLD 2048000

/*
 * Better be safe than sorry :P
 */
#ifndef size_t
	typedef unsigned int size_t;
#endif

struct _Object;
/*
 * This structure represent an item in the gc 
 * pool.
 *
 * pobj : Is a pointer to the object to track.
 * size : The size of the object itself.
 * next : Pointer to the next item in the pool.
 * prev : Pointer to the previous item in the pool.
 */
typedef struct _gc_item {
    struct _Object *pobj;
    size_t          size;

    _gc_item       *next;
    _gc_item       *prev;

    _gc_item( struct _Object *p, size_t s ) : pobj(p), size(s) { }
}
gc_item_t;

/*
 * Main gc structure, kinda of the "head" of the pool.
 *
 * pool_*    : Tracked objects list head and tail pointers .
 * items	 : Number of items in the pool.
 * usage	 : Global memory usage, in bytes.
 * threshold : If usage >= this, the gc is triggered.
 * mutex     : Mutex to lock the pool while collecting.
 */
typedef struct _gc {
	gc_item_t 	   *pool_head;
    gc_item_t 	   *pool_tail;
    size_t     		items;
    size_t     		usage;
    size_t     	 	threshold;
	#ifdef MT_SUPPORT
	pthread_mutex_t mutex;
	#endif

    _gc() : pool_head(NULL),
    		pool_tail(NULL),
			items(0),
			usage(0),
			threshold(GC_DEFAULT_MEMORY_THRESHOLD)
			#ifdef MT_SUPPORT
			,mutex(PTHREAD_MUTEX_INITIALIZER)
			#endif
	{ }
}
gc_t;

/* 
 * Add an object to the gc pool and start to track
 * it for reference changes.
 * Size must be passed explicitly due to the downcasting
 * possibility.
 */
struct _Object *gc_track( struct _Object *o, size_t size );
/*
 * Fire the collection routines if the memory usage is
 * above the threshold.
 */
void            gc_collect();
/*
 * Release all the pool and its contents, should be
 * used when the program is exiting, not before.
 */
void            gc_release();

#endif
