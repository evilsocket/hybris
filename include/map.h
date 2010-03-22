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
#include "vmem.h"

#ifndef H_UNDEFINED
#   define H_UNDEFINED NULL
#endif

#include <vector>
#include <string>

using std::vector;
using std::string;

#define H_TEMPLATE_T template< typename value_t >

H_TEMPLATE_T class Map {
private :

    struct map_pair {
        string   label;
        value_t *value;

        map_pair( char *l, value_t *v ){
            label = l;
            value = v;
        }
    };

    unsigned int       m_elements;
    vector<map_pair *> m_map;
    hash_table_t      *m_table;

    inline int search_index( char *label ){
		unsigned int i, j, size = m_elements, send = size - 1;
		for( i = 0, j = send; i < size && j >= 0; ++i, --j ){
			if( m_map[i]->label == label ){
				return i;
			}else if( m_map[j]->label == label ){
				return j;
			}
		}
		return -1;
	}

	inline int search_index( value_t *value ){
		unsigned int i, j, size = m_elements, send = size - 1;
		unsigned long v_address = H_ADDRESS_OF(value);
		for( i = 0, j = send; i < size && j >= 0; ++i, --j ){
			if( H_ADDRESS_OF(m_map[i]->value) == v_address ){
				return i;
			}else if( H_ADDRESS_OF(m_map[j]->value) == v_address ){
				return j;
			}
		}
		return -1;
	}



public  :

    Map();
    ~Map();

    inline unsigned int size(){
		return m_elements;
	}

    inline value_t *at( unsigned int index ){
        return m_map[index]->value;
	}

	inline const char * label( unsigned int index ){
		return m_map[index]->label.c_str();
	}

	inline value_t *replace( char *label, value_t *old_value, value_t *new_value ){
        int idx = search_index( (value_t *)old_value );

        m_map[idx]->value = new_value;
        ht_insert( m_table, PTR_KEY( m_table, label ), (u_long)new_value );

        return old_value;
	}

    value_t *insert( char *label, value_t *value );
    value_t *set( char *label, value_t *value );
    value_t *find( char *label );
    int      index( char *label );

    void     pop();
    void     clear();
};

H_TEMPLATE_T Map<value_t>::Map(){
    m_table    = ht_alloc( 0, 1 );
    m_elements = 0;
}

H_TEMPLATE_T Map<value_t>::~Map(){
    clear();
}

H_TEMPLATE_T value_t * Map<value_t>::insert( char *label, value_t *value ){
    m_map.push_back( new map_pair(label, value) );
    ht_insert( m_table, PTR_KEY( m_table, label ), (u_long)value );
    m_elements++;
    return value;
}

H_TEMPLATE_T value_t * Map<value_t>::set( char *label, value_t *value ){
    int i = search_index(label);
	if( i != -1 ){
	    ht_insert( m_table, PTR_KEY( m_table, label ), (u_long)value );
		m_map[i]->label = label;
		m_map[i]->value = value;
		return value;
	}

    return H_UNDEFINED;
}

H_TEMPLATE_T value_t * Map<value_t>::find( char *label ){
    hash_item_t *item = ht_find( m_table, PTR_KEY( m_table, label ) );
    if( item ){
        return (value_t *)item->data;
    }
    return H_UNDEFINED;
}

H_TEMPLATE_T int Map<value_t>::index( char *label ){
	return search_index(label);
}

H_TEMPLATE_T void Map<value_t>::pop(){
    if( m_map.size() > 0 ){
        m_elements--;
        int index = m_map.size() - 1;
        ht_delete( m_table, PTR_KEY( m_table, m_map[index]->label ) );
        delete m_map[ index ];
        m_map.pop_back();
    }
}

H_TEMPLATE_T void Map<value_t>::clear(){
    unsigned int i;
    for( i = 0; i < m_map.size(); --i ){
        delete m_map[i];
    }
    m_map.clear();
    m_elements = 0;
    ht_clear( m_table );
}

#endif

