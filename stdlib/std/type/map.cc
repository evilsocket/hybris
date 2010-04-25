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

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "map", hmap },
	{ "mapelements", hmapelements },
	{ "mappop", hmappop },
	{ "unmap", hunmap },
	{ "ismapped", hismapped },
	{ "haskey", hhaskey },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hmap){
	if( (ob_argc() % 2) != 0 ){
		hyb_error( H_ET_SYNTAX, "function 'map' requires an even number of parameters (called with %d)", ob_argc() );
	}
	unsigned int i;
	Object *map = ob_dcast( gc_new_map() );
	for( i = 0; i < data->size(); i += 2 ){
		ob_cl_set( map, ob_argv(i), ob_argv(i + 1) );
	}
	return map;
}

HYBRIS_DEFINE_FUNCTION(hmapelements){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'mapelements' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otMap );

    return ob_dcast( gc_new_integer( ob_map_ucast(ob_argv(0))->items ) );
}

HYBRIS_DEFINE_FUNCTION(hmappop){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'mappop' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otMap );

	return ob_cl_pop( ob_argv(0) );
}

HYBRIS_DEFINE_FUNCTION(hunmap){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'unmap' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otMap );

	return ob_cl_remove( ob_argv(0), ob_argv(1) );
}

HYBRIS_DEFINE_FUNCTION(hismapped){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'ismapped' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otMap );

	Object *map  = ob_argv(0),
           *find = ob_argv(1);
	unsigned int i;

	for( i = 0; i < ob_map_ucast(map)->items; ++i ){
		if( ob_cmp( ob_map_ucast(map)->values[i], find ) == 0 ){
			return ob_dcast( gc_new_integer(i) );
		}
	}

	return H_DEFAULT_ERROR;
}

HYBRIS_DEFINE_FUNCTION(hhaskey){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'haskey' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otMap );

	Object *map = ob_argv(0),
	       *key = ob_argv(1);
	unsigned int i;

	for( i = 0; i < ob_map_ucast(map)->items; ++i ){
		if( ob_cmp( ob_map_ucast(map)->keys[i], key ) == 0 ){
			return ob_dcast( gc_new_integer(i) );
		}
	}

	return H_DEFAULT_ERROR;
}
