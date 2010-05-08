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
#include "asciitree.h"

/*
 * Find next link with 'ascii' byte.
 */
__force_inline ascii_item_t *at_find_next_link( ascii_tree_t *at, char ascii ){
	int i, n_links(at->n_links);
	ascii_item_t *link;

	for( i = 0; i < n_links; ++i ){
		link =  at->links[i];
		if( link->ascii == ascii ){
			return link;
		}
	}
	return NULL;
}

void at_insert( ascii_tree_t *at, char *key, int len, void *value ){
	/*
	 * End of the chain, set the marker value and exit the recursion.
	 */
	if(!len){ at->e_marker = value; return; }

	/*
	 * Has the item a link with given byte?
	 */
	ascii_item_t *link = at_find_next_link( at, key[0] );
	if( link ){
		/*
		 * Next recursion, search next byte,
		 */
		at_insert( link, key + 1, len - 1, value );
	}
	/*
	 * Nothing found.
	 */
	else{
		/*
		 * Allocate, initialize and append a new link.
		 */
		at_init_link( link, key );
		at_append_link( at, link );
		/*
		 * Continue with next byte.
		 */
		at_insert( link, key + 1, len - 1, value );
	}
}

void *at_find( ascii_tree_t *at, char *key, int len ){
	/*
	 * End of the chain, if e_marker is NULL this chain is not complete,
	 * therefore 'key' does not map any alive object.
	 */
	if(!len){ return at->e_marker; }
	/*
	 * Find next link ad continue recursion.
	 */
	ascii_item_t *link = at_find_next_link( at, key[0] );
	if( link ){
		return at_find( link, key + 1, len - 1 );
	}
	/*
	 * Nothing found! Not an alive key.
	 */
	else{
		return NULL;
	}
}

void at_free( ascii_tree_t *at ){
	int i, n_links(at->n_links);
	/*
	 * Better be safe than sorry ;)
	 */
	if( at->links ){
		/*
		 * First of all, loop all the sub links.
		 */
		for( i = 0; i < n_links; ++i, --at->n_links ){
			/*
			 * Free this link sub-links.
			 */
			at_free( at->links[i] );
			/*
			 * Free the link itself.
			 */
			free( at->links[i] );
		}
		/*
		 * Finally free the array.
		 */
		free( at->links );
		at->links = NULL;
	}
}
