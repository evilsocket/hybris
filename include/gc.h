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
#include <vector>
#include <stdio.h>

using std::vector;

/*
 * Better be safe than sorry :P
 */
#ifndef size_t
	typedef unsigned int size_t;
#endif

#define GC_DEFAULT_MEMORY_THRESHOLD 100000

struct _Object;

typedef struct _gc_item {
    struct _Object *pobj;
    size_t          size;

    _gc_item( struct _Object *p, size_t s ) : pobj(p), size(s) { }
}
gc_item_t;

typedef vector<gc_item_t *> gc_pool_t;

typedef struct _gc {
    gc_pool_t pool;
    size_t    items;
    size_t    usage;
    size_t    threshold;

    _gc() : items(0), usage(0), threshold(GC_DEFAULT_MEMORY_THRESHOLD) { }
}
gc_t;

struct _Object *gc_track( struct _Object *o, size_t size );
void            gc_collect();
void            gc_release();

#endif
