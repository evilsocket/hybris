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
#ifndef _HMAP_H_
#	define _HMAP_H_

#include "hashtable.h"

/* from vmem.h */
#ifndef H_UNDEFINED
/* helper macro to obtain the address of a pointer */
#   define H_ADDRESS_OF(o)      reinterpret_cast<ulong>(o)
/* default null value for an Object pointer */
#   define H_UNDEFINED          NULL
#endif

#ifndef __force_inline
#	define __force_inline __inline__ __attribute__((always_inline))
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;

#define H_TEMPLATE_T template< typename value_t >

/*
 * This class is the base for all the lookup tables inside Hybris.
 * It's used by MemorySegment, CodeSegment, cache tables and so on.
 *
 * HashMap has two main containers.
 *
 * m_map   : A vector to have items fast access by index.
 * m_table : An hash table to have fast access by label.
 *
 * They points to the same objects so, changing the value of an item in
 * the vector will change that value inside the table, and viceversa.
 */
H_TEMPLATE_T class HashMap {
protected :
	/*
	 * Structure rapresentation for an item the the map.
	 */
    typedef struct map_pair {
        string        label;
        value_t      *value;

        map_pair( char *l, value_t *v ) :
            label(l),
            value(v) {

        }
    }
    pair_t;

    unsigned int     m_elements;
    vector<pair_t *> m_map;
    hash_table_t    *m_table;

public  :

    typedef typename vector<map_pair *>::iterator               iterator;
    typedef typename vector<map_pair *>::const_iterator         const_iterator;
    typedef typename vector<map_pair *>::reverse_iterator       reverse_iterator;
    typedef typename vector<map_pair *>::const_reverse_iterator const_reverse_iterator;

    __force_inline iterator begin(){
        return m_map.begin();
    }

    __force_inline const_iterator begin() const {
        return m_map.begin();
    }

    __force_inline iterator end() {
        return m_map.end();
    }

    __force_inline const_iterator end() const {
        return m_map.end();
    }

    __force_inline reverse_iterator rbegin(){
        return m_map.rbegin();
    }

    __force_inline const_reverse_iterator rbegin() const {
        return m_map.rbegin();
    }

    __force_inline reverse_iterator rend() {
        return m_map.rend();
    }

    __force_inline const_reverse_iterator rend() const {
        return m_map.rend();
    }

    HashMap();
    ~HashMap();

    /* Get the number of items mapped here. f*/
    __force_inline unsigned int size(){
		return m_elements;
	}
    /* Get the value of the item at 'index' position */
    __force_inline value_t *at( unsigned int index ){
        return m_map[index]->value;
	}
    /* Get the label of the item at 'index' position */
    __force_inline const char * label( unsigned int index ){
		return m_map[index]->label.c_str();
	}
	/* Insert the value if it's not already mapped, otherwise change the old reference to this. */
    value_t *insert( char *label, value_t *value );
    /* Find the item mappeb with 'label', or return NULL if it's not here */
    value_t *find( char *label );
    /* Replace the value if it already exists */
    value_t *replace( char *label, value_t *old_value, value_t *new_value );
    /* Clear the whole table */
    void     clear();
};

H_TEMPLATE_T HashMap<value_t>::HashMap(){
    m_table    = ht_alloc( 0, 1 );
    m_elements = 0;
}

H_TEMPLATE_T HashMap<value_t>::~HashMap(){
	/*
	 * No need to clear here.
	 * Table will be freed by ht_free and objects will be
	 * garbage collected.
	 */
    // clear();
	unsigned int i, size(m_elements);
	for( i = 0; i < size; ++i ){
		delete m_map[i];
	}
    ht_free(m_table);
}

H_TEMPLATE_T value_t * HashMap<value_t>::insert( char *label, value_t *value ){
	pair_t *pair = new pair_t( label, value );
	m_map.push_back( pair );
    ht_insert( m_table, (u_long)label, (u_long)pair );
    m_elements++;
    return value;
}

H_TEMPLATE_T value_t * HashMap<value_t>::find( char *label ){
    hash_item_t *item = ht_find( m_table, (u_long)label );
    if( item ){
        return ((pair_t *)item->data)->value;
    }
    return H_UNDEFINED;
}

H_TEMPLATE_T value_t * HashMap<value_t>::replace( char *label, value_t *old_value, value_t *new_value ){
	hash_item_t *item;

	if( (item = ht_find( m_table, (u_long)label )) != NULL ){
		((pair_t *)item->data)->value = new_value;
	}

	return old_value;
}

H_TEMPLATE_T void HashMap<value_t>::clear(){
    unsigned int i, size(m_elements);
    for( i = 0; i < size; ++i ){
        delete m_map[i];
    }
    m_map.clear();
    m_elements = 0;
    ht_clear( m_table );
}

#endif
