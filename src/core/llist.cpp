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
#include "llist.h"

void ll_append( llist_t *ll, void *data ){
	ll_item_t *item = (ll_item_t *)calloc( 1, sizeof(ll_item_t) );

	item->data = data;

	if( ll->head == NULL ){
		ll->head = item;
	}
	else{
		ll->tail->next = item;
		item->prev 	   = ll->tail;
	}

	ll->tail = item;
	++ll->items;
}

void ll_prepend( llist_t *ll, void *data ){
	ll_item_t *item = (ll_item_t *)calloc( 1, sizeof(ll_item_t) );

	item->data = data;

	if( ll->head == NULL ){
		ll->tail = item;
	}
	else{
		ll->head->prev = item;
		item->next 	   = ll->head;
	}

	ll->head = item;
	++ll->items;
}

void ll_move( llist_t *from, llist_t *to, ll_item_t *item ){
	if( item->prev == NULL ){
		from->head = item->next;
	}
	else{
		item->prev->next = item->next;
	}
	if( item->next == NULL ){
		from->tail = item->prev;
	}
	else{
		item->next->prev = item->prev;
	}

	if( to->head == NULL ){
		to->head = item;
		item->prev = NULL;
	}
	else{
		to->tail->next = item;
		item->prev 	   = to->tail;
	}

	to->tail   = item;
	item->next = NULL;

	--from->items;
	++to->items;
}

void ll_copy( llist_t *from, llist_t *to ){
	ll_item_t *item;
	for( item = from->head; item; item = item->next ){
		ll_append( to, item->data );
	}
}

void ll_merge( llist_t *dest, llist_t *source ){
	ll_item_t *item = source->head,
			  *next;

	dest->items  += source->items;
	source->items = 0;

	while(item){
		next = item->next;

		if( item->prev == NULL ){
			source->head = next;
		}
		else{
			item->prev->next = next;
		}
		if( next == NULL ){
			source->tail = item->prev;
		}
		else{
			next->prev = item->prev;
		}

		if( dest->head == NULL ){
			dest->head = item;
			item->prev = NULL;
		}
		else{
			dest->tail->next = item;
			item->prev 	     = dest->tail;
		}

		dest->tail = item;
		item->next = NULL;

		item = next;
	}
}

void *ll_remove( llist_t *ll, ll_item_t *item ){
	void *data = item->data;

	if( item->prev == NULL ){
		ll->head = item->next;
	}
	else{
		item->prev->next = item->next;
	}
	if( item->next == NULL ){
		ll->tail = item->prev;
	}
	else{
		item->next->prev = item->prev;
	}

	--ll->items;

	free(item);

	return data;
}

void ll_clear( llist_t *ll ){
	ll_item_t *item = ll->head,
			  *next;

	while(item){
		next = item->next;
		free(item);
		item = next;
	}

	ll_init(ll);
}
