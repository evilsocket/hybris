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

HYBRIS_DEFINE_FUNCTION(harray);
HYBRIS_DEFINE_FUNCTION(helements);
HYBRIS_DEFINE_FUNCTION(hpop);
HYBRIS_DEFINE_FUNCTION(hremove);
HYBRIS_DEFINE_FUNCTION(hcontains);
HYBRIS_DEFINE_FUNCTION(hjoin);


HYBRIS_EXPORTED_FUNCTIONS() {
	{ "array", harray },
	{ "elements", helements },
	{ "pop", hpop },
	{ "remove", hremove },
	{ "contains", hcontains },
	{ "join", hjoin },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(harray){
	unsigned int i;
	Object *array = ob_dcast( gc_new_vector() );
	for( i = 0; i < data->size(); ++i ){
		ob_cl_push( array, ob_argv(i) );
	}
	return array;
}

HYBRIS_DEFINE_FUNCTION(helements){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'elements' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otVector );

    return ob_dcast( gc_new_integer( vector_argv(0)->items ) );
}

HYBRIS_DEFINE_FUNCTION(hpop){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'pop' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otVector );

	return ob_cl_pop( ob_argv(0) );
}

HYBRIS_DEFINE_FUNCTION(hremove){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'remove' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otVector );

	return ob_cl_remove( ob_argv(0), ob_argv(1) );
}

HYBRIS_DEFINE_FUNCTION(hcontains){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'contains' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otVector );

	Object *array = ob_argv(0),
           *find  = ob_argv(1);
	unsigned int i;

	for( i = 0; i < ob_vector_ucast(array)->items; ++i ){
		if( ob_cmp( ob_vector_ucast(array)->value[i], find ) == 0 ){
			return ob_dcast( gc_new_integer(i) );
		}
	}

	return H_DEFAULT_ERROR;
}

HYBRIS_DEFINE_FUNCTION(hjoin){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'join' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(1), otVector );

	Object *array = ob_argv(1);
	string  glue  = ob_svalue( ob_argv(0) ),
		    join;
	unsigned int i, items(ob_vector_ucast(array)->items);

	for( i = 0; i < items; ++i ){
		join += ob_svalue(ob_vector_ucast(array)->value[i]) + ( i < items - 1 ? glue : "");
	}

	return (Object *)gc_new_string(join.c_str());
}
