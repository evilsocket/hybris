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
#ifndef _HDARRAY_H_
#   define _HDARRAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

/*
 * Number of items to pre allocate when da_init is called.
 */
#define DARRAY_PREALLOC_ITEMS     10
/*
 * How many new items to create every time we have to call realloc.
 */
#define DARRAY_REALLOC_ITEMS_STEP 10

typedef struct {
	/*
	 * Total avaiable items, busy + preallocated.
	 */
	size_t items;
	/*
	 * Memory usage.
	 */
	size_t usage;
	/*
	 * Number of busy items.
	 */
	size_t busy;
	/*
	 * Size of a single item.
	 */
	size_t size;
	/*
	 * The dynamic buffer.
	 */
	byte  *data;
}
darray_t;

/*
 * Get the number of items of the array.
 */
#define da_size(da) ((da)->busy)
/*
 * Get a pointer to the 'n'th item of the arary.
 */
#define da_at(da,n)	(&(da)->data[n])
/*
 * Initialize the array, preallocating 'DARRAY_PREALLOC_ITEMS' items.
 */
void 	da_init( darray_t *da, size_t size, size_t prealloc = DARRAY_PREALLOC_ITEMS );
/*
 * Get the next free item and mark it as busy.
 */
void   *da_next( darray_t *da );
/*
 * Remove an item from the array given its address.
 */
void 	da_remove( darray_t *da, void *item );
/*
 * Free the array.
 */
void 	da_free( darray_t *da );

#endif

