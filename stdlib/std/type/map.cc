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
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hmap);
HYBRIS_DEFINE_FUNCTION(hmapelements);
HYBRIS_DEFINE_FUNCTION(hmappop);
HYBRIS_DEFINE_FUNCTION(hunmap);
HYBRIS_DEFINE_FUNCTION(hismapped);
HYBRIS_DEFINE_FUNCTION(hhaskey);

extern "C" named_function_t hybris_module_functions[] = {
	{ "map", hmap },
	{ "mapelements", hmapelements },
	{ "mappop", hmappop },
	{ "unmap", hunmap },
	{ "ismapped", hismapped },
	{ "haskey", hhaskey },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hmap){
	if( (data->size() % 2) != 0 ){
		hyb_syntax_error( "function 'map' requires an even number of parameters (called with %d)", data->size() );
	}
	unsigned int i;
	Object *map = new Object();
	for( i = 0; i < data->size(); i += 2 ){
		map->map( data->at(i), data->at(i + 1) );
	}
	return map;
}

HYBRIS_DEFINE_FUNCTION(hmapelements){
	if( data->size() != 1 ){
		hyb_syntax_error( "function 'mapelements' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_MAP );

	return new Object( static_cast<long>( data->at(0)->value.m_map.size() ) );
}

HYBRIS_DEFINE_FUNCTION(hmappop){
	if( data->size() != 1 ){
		hyb_syntax_error( "function 'mappop' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_MAP );

	return new Object( data->at(0)->mapPop() );
}

HYBRIS_DEFINE_FUNCTION(hunmap){
	if( data->size() != 2 ){
		hyb_syntax_error( "function 'unmap' requires 2 parameters (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_MAP );

	return new Object( data->at(0)->unmap(data->at(1)) );
}

HYBRIS_DEFINE_FUNCTION(hismapped){
	if( data->size() != 2 ){
		hyb_syntax_error( "function 'ismapped' requires 2 parameters (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_MAP );
	Object *map   = data->at(0),
		    *find  = data->at(1);
	unsigned int i;

	for( i = 0; i < map->value.m_array.size(); i++ ){
		if( map->value.m_array[i]->equals(find) ){
			return new Object( static_cast<long>(i) );
		}
	}

	return new Object( static_cast<long>(-1) );
}

HYBRIS_DEFINE_FUNCTION(hhaskey){
	if( data->size() != 2 ){
		hyb_syntax_error( "function 'haskey' requires 2 parameters (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_MAP );

	Object *map = data->at(0),
           *key = data->at(1);
	unsigned int i;

	for( i = 0; i < map->value.m_map.size(); i++ ){
		if( map->value.m_map[i]->equals(key) ){
			return new Object( static_cast<long>(i) );
		}
	}

	return new Object( static_cast<long>(-1) );
}
