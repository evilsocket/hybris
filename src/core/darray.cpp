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
#include "darray.h"


void da_init( darray_t *da, size_t size, size_t prealloc /*= DARRAY_PREALLOC_ITEMS*/ ){
	da->items = prealloc;
	da->size  = size;
	da->usage = prealloc * size;
	da->busy  = 0;
	da->data  = (byte *)malloc( da->usage );
}

void *da_next( darray_t *da ){
	if( da->busy >= da->items ){
		da->items += DARRAY_REALLOC_ITEMS_STEP;
		da->usage  = da->items * da->size;
		da->data   = (byte *)realloc( da->data, da->usage );
	}
	return &da->data[ da->busy++ ];
}

void da_remove( darray_t *da, void *item ){
	register size_t offset = ((byte *)item - da->data);

	--da->busy;
	--da->items;

	da->usage -= da->size;

	memmove( &da->data[offset], &da->data[offset + 1], da->usage );
}

void da_free( darray_t *da ){
	free(da->data);
	da->items = 0;
	da->usage = 0;
	da->busy  = 0;
	da->data  = NULL;
}
