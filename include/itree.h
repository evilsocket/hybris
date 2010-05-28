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
#ifndef _ITREE_H_
#	define _ITREE_H_

#include "asciitree.h"

/* from vmem.h */
#ifndef H_UNDEFINED
/* helper macro to obtain the address of a pointer */
#   define H_ADDRESS_OF(o)      reinterpret_cast<ulong>(o)
/* default null value for an Object pointer */
#   define H_UNDEFINED          NULL
#endif

#ifndef INLINE
#	define INLINE __inline__ __attribute__((always_inline))
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;

#define H_TEMPLATE_T template< typename value_t >
/*
 * A macro to easily loop itrees.
 */
#define itree_foreach( INNER_TYPE, ITERATOR, ITREE ) vv_foreach( ITree<INNER_TYPE>, ITERATOR, ITREE )
/*
 * This class is the base for all the lookup tables inside Hybris.
 * It's used by MemorySegment, CodeSegment, cache tables and so on.
 *
 * ITree has two main containers.
 *
 * m_map   : A vector to have items fast access by index.
 * m_table : An ascii tree to have fast access by label.
 *
 * They points to the same objects so, changing the value of an item in
 * the vector will change that value inside the table, and viceversa.
 * With those two references, the class represent an indexed ascii tree,
 * adding by-index access capabilities to the ascii tree data type.
 */
H_TEMPLATE_T class ITree {
protected :
	/*
	 * Structure rapresentation for an item the the map.
	 */
    typedef struct map_pair {
        string        label;
        value_t      *value;

        map_pair( char *l, value_t *v ) : label(l), value(v) {

        }
    }
    pair_t;

    unsigned int     m_elements;
    vector<pair_t *> m_map;
    ascii_tree_t     m_tree;

public  :

    typedef typename vector<map_pair *>::iterator               iterator;
    typedef typename vector<map_pair *>::const_iterator         const_iterator;
    typedef typename vector<map_pair *>::reverse_iterator       reverse_iterator;
    typedef typename vector<map_pair *>::const_reverse_iterator const_reverse_iterator;

    INLINE iterator begin(){
        return m_map.begin();
    }

    INLINE const_iterator begin() const {
        return m_map.begin();
    }

    INLINE iterator end() {
        return m_map.end();
    }

    INLINE const_iterator end() const {
        return m_map.end();
    }

    INLINE reverse_iterator rbegin(){
        return m_map.rbegin();
    }

    INLINE const_reverse_iterator rbegin() const {
        return m_map.rbegin();
    }

    INLINE reverse_iterator rend() {
        return m_map.rend();
    }

    INLINE const_reverse_iterator rend() const {
        return m_map.rend();
    }

    ITree();
    virtual ~ITree();

    /* Get the number of items mapped here. f*/
    INLINE unsigned int size(){
		return m_elements;
	}
    /* Get the value of the item at 'index' position */
    INLINE value_t *at( unsigned int index ){
        return m_map[index]->value;
	}
    /* Get the label of the item at 'index' position */
    INLINE const char * label( unsigned int index ){
		return m_map[index]->label.c_str();
	}
	/* Insert the value if it's not already mapped, otherwise change the old reference to this. */
    value_t *insert( char *label, value_t *value );
    /* Remove an object from the tree */
    value_t	*remove( char *label );
    /* Find the item mappeb with 'label', or return NULL if it's not here */
    value_t *find( char *label );
    /* Replace the value if it already exists */
    value_t *replace( char *label, value_t *old_value, value_t *new_value );
    /* Clear the whole table */
    void     clear();
};

H_TEMPLATE_T ITree<value_t>::ITree(){
	at_init_tree(m_tree);
    m_elements = 0;
}

H_TEMPLATE_T ITree<value_t>::~ITree(){
	clear();
}

H_TEMPLATE_T value_t * ITree<value_t>::insert( char *label, value_t *value ){
	pair_t *pair = new pair_t( label, value );
	m_map.push_back( pair );

	at_insert( &m_tree, label, strlen(label), pair );

    m_elements++;

    return value;
}

H_TEMPLATE_T value_t *ITree<value_t>::remove( char *label ){
	pair_t  *item = (pair_t *)at_remove( &m_tree, label, strlen(label) );
	value_t *pval = NULL;

	if( item ){
		pval = item->value;
		size_t i, size(m_elements);
		for( i = 0; i < size; ++i ){
			if( m_map[i] == item ){
				delete m_map[i];
				m_map.erase( m_map.begin() + i );
				m_elements--;
				break;
			}
		}
	}
	return pval;
}

H_TEMPLATE_T value_t * ITree<value_t>::find( char *label ){
	pair_t *item = (pair_t *)at_find( &m_tree, label, strlen(label) );
    if( item ){
        return item->value;
    }
    return H_UNDEFINED;
}

H_TEMPLATE_T value_t * ITree<value_t>::replace( char *label, value_t *old_value, value_t *new_value ){
	pair_t *item;

	if( (item = (pair_t *)at_find( &m_tree, label, strlen(label) )) != NULL ){
		item->value = new_value;
	}

	return old_value;
}

H_TEMPLATE_T void ITree<value_t>::clear(){
    unsigned int i, size(m_elements);
    for( i = 0; i < size; ++i ){
        delete m_map[i];
    }
    m_map.clear();
    m_elements = 0;
    at_clear( &m_tree );
}

#endif
