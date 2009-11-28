/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _HMAP_H_
#	define _HMAP_H_

#include <vector>
#include <string>

using std::vector;
using std::string;

template< typename value_t >
class Map {
private :

    struct map_pair {
        string   label;
        value_t *value;

        map_pair( char *l, value_t *v ){
            label = l;
            value = v;
        }
    };

    vector<map_pair *> m_map;

public  :

    static value_t * null;

    Map();
    ~Map();

    inline unsigned int size(){
		return m_map.size();
	}

    inline value_t *at( unsigned int index ){
		return m_map[index]->value;
	}

	inline const char * label( unsigned int index ){
		return m_map[index]->label.c_str();
	}

	inline int quick_search( char *label ){
	    printf( "quick_search [ 0x%X ] ( %s ) :\n", this, label );
		unsigned int i, j, size = m_map.size(), send = size - 1;
		for( i = 0, j = send; i < size && j >= 0; i++, j-- ){
		    printf( "\t%s - %s\n", m_map[i]->label.c_str(), m_map[j]->label.c_str() );
			if( m_map[i]->label == label ){
				return i;
			}else if( m_map[j]->label == label ){
				return j;
			}
		}
		return -1;
	}

    value_t *insert( char *label, value_t *value );
    value_t *set( char *label, value_t *value );
    value_t *set( unsigned int index, value_t *value );
    value_t *setlabel( char *oldlabel, char *label );
    value_t *find( char *label );
    int      index( char *label );
    void     remove( char *label );
    void     pop();
    void     clear();

};

template<typename value_t>
value_t * Map<value_t>::null = (value_t *)0xFFFFFFFF;

template< typename value_t >
Map<value_t>::Map(){ }

template< typename value_t >
Map<value_t>::~Map(){
    clear();
}

template< typename value_t >
value_t * Map<value_t>::insert( char *label, value_t *value ){
    m_map.push_back( new map_pair(label, value) );
    return value;
}

template< typename value_t >
value_t * Map<value_t>::set( char *label, value_t *value ){
    int i = quick_search(label);
	if( i != -1 ){
		m_map[i]->label = label;
		m_map[i]->value = value;
		return value;
	}
    return null;
}

template< typename value_t >
value_t * Map<value_t>::set( unsigned int index, value_t *value ){
    if( index < m_map.size() && index >= 0 ){
        return m_map[index]->value = value;
    }
    return null;
}

template< typename value_t >
value_t * Map<value_t>::setlabel( char *oldlabel, char *label ){
    int i = quick_search(oldlabel);
	if( i != -1 ){
		m_map[i]->label = label;
        return m_map[i]->value;
	}
    return null;
}

template< typename value_t >
value_t * Map<value_t>::find( char *label ){
	int i = quick_search(label);
	if( i != -1 ){
        return m_map[i]->value;
	}
    return null;
}

template< typename value_t >
int Map<value_t>::index( char *label ){
	return quick_search(label);
}

template< typename value_t >
void Map<value_t>::remove( char *label ){
	int i = quick_search(label);
	if( i != -1 ){
		delete m_map[i];
		m_map.erase( m_map.begin() + i );
		return;
	}
}

template< typename value_t >
void Map<value_t>::pop(){
    if( m_map.size() > 0 ){
        int index = m_map.size() - 1;
        delete m_map[ index ];
        m_map.pop_back();
    }
}

template< typename value_t >
void Map<value_t>::clear(){
    unsigned int i;
    for( i = 0; i < m_map.size(); i++ ){
        delete m_map[i];
    }
    m_map.clear();
}

#endif

