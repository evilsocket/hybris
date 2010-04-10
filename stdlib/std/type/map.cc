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
	if( (HYB_ARGC() % 2) != 0 ){
		hyb_throw( H_ET_SYNTAX, "function 'map' requires an even number of parameters (called with %d)", HYB_ARGC() );
	}
	unsigned int i;
	Object *map = MK_COLLECTION_OBJ();
	for( i = 0; i < data->size(); i += 2 ){
		map->map( HYB_ARGV(i), HYB_ARGV(i + 1) );
	}
	return map;
}

HYBRIS_DEFINE_FUNCTION(hmapelements){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'mapelements' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_MAP );

    return MK_INT_OBJ( HYB_ARGV(0)->value.m_map.size() );
}

HYBRIS_DEFINE_FUNCTION(hmappop){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'mappop' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_MAP );

	return HYB_ARGV(0)->mapPop();
}

HYBRIS_DEFINE_FUNCTION(hunmap){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'unmap' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_MAP );

	return HYB_ARGV(0)->unmap(HYB_ARGV(1));
}

HYBRIS_DEFINE_FUNCTION(hismapped){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'ismapped' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_MAP );
	Object *map   = HYB_ARGV(0),
           *find  = HYB_ARGV(1);
	unsigned int i;

	for( i = 0; i < map->value.m_array.size(); i++ ){
		if( map->value.m_array[i]->equals(find) ){
			return MK_INT_OBJ(i);
		}
	}

	return MK_INT_OBJ(-1);
}

HYBRIS_DEFINE_FUNCTION(hhaskey){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'haskey' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_MAP );

	Object *map = HYB_ARGV(0),
           *key = HYB_ARGV(1);
	unsigned int i;

	for( i = 0; i < map->value.m_map.size(); i++ ){
		if( map->value.m_map[i]->equals(key) ){
			return MK_INT_OBJ(i);
		}
	}

	return MK_INT_OBJ(-1);
}
