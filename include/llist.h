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
#ifndef _HLLIST_H_
#	define _HLLIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Linked list generic item container.
 */
typedef struct ll_item {
    ll_item *next;
    ll_item *prev;
    void    *data;
}
ll_item_t;
/*
 * The linked list container itself.
 */
typedef struct {
	ll_item_t *head;
	ll_item_t *tail;
	size_t 	   items;
}
llist_t;

/*
 * Macro to easily loop the list.
 * NOTE: This macro has to be used only for read only loops,
 * if during a loop the list is modified (i.e. an element is removed),
 * you have to MANUALLY fix the pointers.
 */
#define ll_foreach( ll, item ) for( ll_item_t *item = (ll)->head; item; item = item->next )

/*
 * Initialize a linked list.
 */
#define ll_init( ll ) (ll)->head = NULL; \
					  (ll)->tail = NULL; \
					  (ll)->items = 0
/*
 * Get the number of items in the list.
 */
#define ll_size( ll ) (ll)->items
/*
 * Remove the last element from the list.
 */
#define ll_pop( ll )  ll_remove( (ll), (ll)->tail )
/*
 * Get a pointer to the first element data.
 */
#define ll_head( ll ) (ll)->head->data
/*
 * Get a pointer to the last element data.
 */
#define ll_back( ll ) (ll)->tail->data
/*
 * Add an element to the end of the list.
 */
void 	ll_append( llist_t *ll, void *data );
/*
 * Add an element on top of the list.
 */
void 	ll_prepend( llist_t *ll, void *data );
/*
 * Move the item from one list to the end of another.
 */
void 	ll_move( llist_t *from, llist_t *to, ll_item_t *item );
/*
 * Remove an element from the list.
 */
void 	ll_remove( llist_t *ll, ll_item_t *item );
/*
 * Clear and deallocate the list.
 */
void 	ll_free( llist_t *ll );

#endif
