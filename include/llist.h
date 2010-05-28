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
 * Macro to obtain item data and cast it to its original type.
 */
#define ll_data( t, llitem ) ((t)((llitem)->data))
/*
 * Macro to easily loop the list.
 * NOTE: This macro has to be used only for read only loops,
 * if during a loop the list is modified (i.e. an element is removed),
 * you have to MANUALLY fix the pointers.
 */
#define ll_foreach( ll, item ) for( ll_item_t *item = (ll)->head; item; item = item->next )
/*
 * Macro to easily loop the list until a certain item.
 * NOTE: This macro has to be used only for read only loops,
 * if during a loop the list is modified (i.e. an element is removed),
 * you have to MANUALLY fix the pointers.
 */
#define ll_foreach_to( ll, item, i, to ) for( i = 0, item = (ll)->head; i < to; ++i, item = item->next )
/*
 * Allocate and initialize a new linked list.
 */
#define ll_create() (llist_t *)calloc( 1, sizeof(llist_t) )
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
 * Add two elements to the end of the list.
 */
#define ll_append_pair( ll, a, b ) ll_append( (ll), b ); \
								   ll_append( (ll), a )
/*
 * Add an element on top of the list.
 */
void 	ll_prepend( llist_t *ll, void *data );
/*
 * Add two elements on top of the list.
 */
#define ll_prepend_pair( ll, a, b ) ll_prepend( (ll), b ); \
									ll_prepend( (ll), a )
/*
 * Move the item from one list to the end of another.
 */
void 	ll_move( llist_t *from, llist_t *to, ll_item_t *item );
/*
 * Copy the 'from' linked list to the 'to' linked list.
 */
void	ll_copy( llist_t *from, llist_t *to );
/*
 * Merge 'source' list inside 'dest' list, the elements of
 * 'dest' will be removed.
 */
void	ll_merge( llist_t *dest, llist_t *source );
/*
 * Same as ll_merge, plus destroy the 'source' list.
 */
#define ll_merge_destroy( dest, source ) ll_merge( (dest), (source) ); \
										 ll_destroy( (source) )
/*
 * Remove an element from the list.
 */
void   *ll_remove( llist_t *ll, ll_item_t *item );
/*
 * Clear and deallocate each item of the list.
 */
void 	ll_clear( llist_t *ll );
/*
 * Clear and deallocate the list itself previously
 * created with the ll_create macro.
 */
#define ll_destroy( ll ) ll_clear( (ll) ); \
						 free(ll)

#endif
