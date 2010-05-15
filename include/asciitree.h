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
#ifndef _HASCIITREE_H_
#	define _HASCIITREE_H_

#include <stdlib.h>
#include <string.h>
/*
 * Implementation of an n-ary tree, where each link is represented
 * by a char of a key and its links by next chars.
 */
typedef struct _ascii_tree {
	/*
	 * The byte value of this link.
	 */
	char    	  ascii;
	/*
	 * Value of the link (end marker of a chain).
	 */
	void*   	  e_marker;
	/*
	 * Number of sub links.
	 */
	char    	  n_links;
	/*
	 * Sub links dynamic array.
	 */
	_ascii_tree **links;
}
ascii_tree_t;

typedef ascii_tree_t ascii_item_t;

#ifndef __force_inline
#	define __force_inline __inline__ __attribute__((always_inline))
#endif

/*
 * Initialize the head of the tree.
 */
#define at_init_tree( t )    t.n_links  = 0; \
						     t.e_marker = 0; \
						     t.links    = NULL
/*
 * Allocate and initialize a link of the tree.
 */
#define at_init_link( l, k ) l = (ascii_tree_t *)calloc( 1, sizeof(ascii_tree_t) ); \
						     l->ascii = k[0]
/*
 * Append a link to 'at' tree, realloc its links and
 * increment link counter.
 */
#define at_append_link( at, l ) at->links = (ascii_tree_t **)realloc( at->links, sizeof(ascii_tree_t **) * at->n_links + 1 ); \
								at->links[ at->n_links++ ] = l

#define at_clear at_free

/*
 * Insert 'value' inside 'at' ascii tree, mapped by
 * the given 'key' of 'len' bytes, if the value already is mapped
 * return its old value, otherwise return the new one.
 */
void *at_insert( ascii_tree_t *at, char *key, int len, void *value );
/*
 * Find the object mapped mapped by the given 'key'
 * of 'len' bytes inside 'at' ascii tree.
 */
void *at_find( ascii_tree_t *at, char *key, int len );
/*
 * Free the tree nodes.
 */
void  at_free( ascii_tree_t *at );

#endif
