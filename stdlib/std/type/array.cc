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

extern "C" named_function_t hybris_module_functions[] = {
	{ "array", harray },
	{ "elements", helements },
	{ "pop", hpop },
	{ "remove", hremove },
	{ "contains", hcontains },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(harray){
	unsigned int i;
	Object *array = OB_DOWNCAST( MK_VECTOR_OBJ() );
	for( i = 0; i < data->size(); ++i ){
		ob_cl_push( array, HYB_ARGV(i) );
	}
	return array;
}

HYBRIS_DEFINE_FUNCTION(helements){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'elements' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otVector );

    return OB_DOWNCAST( MK_INT_OBJ( VECTOR_ARGV(0)->items ) );
}

HYBRIS_DEFINE_FUNCTION(hpop){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'pop' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otVector );

	return ob_cl_pop( HYB_ARGV(0) );
}

HYBRIS_DEFINE_FUNCTION(hremove){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'remove' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otVector );

	return ob_cl_remove( HYB_ARGV(0), HYB_ARGV(1) );
}

HYBRIS_DEFINE_FUNCTION(hcontains){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'contains' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otVector );

	Object *array = HYB_ARGV(0),
           *find  = HYB_ARGV(1);
	unsigned int i;

	for( i = 0; i < VECTOR_UPCAST(array)->items; i++ ){
		if( ob_cmp( VECTOR_UPCAST(array)->value[i], find ) == 0 ){
			return OB_DOWNCAST( MK_INT_OBJ(i) );
		}
	}

	return OB_DOWNCAST( MK_INT_OBJ(-1) );
}

