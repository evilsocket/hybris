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

HYBRIS_DEFINE_FUNCTION(hbinary);
HYBRIS_DEFINE_FUNCTION(hpack);

extern "C" named_function_t hybris_module_functions[] = {
	{ "binary", hbinary },
	{ "pack", hpack },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hbinary){
    if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'binary' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
	vector<unsigned char> stream;
	unsigned int          i;

	for( i = 0; i < data->size(); ++i ){
        HYB_TYPES_ASSERT( HYB_ARGV(i), otInteger, otChar );

        if( IS_INTEGER_TYPE( HYB_ARGV(i) ) ){
            stream.push_back( (unsigned char)INT_ARGV(i) );
        }
        else{
            stream.push_back( (unsigned char)CHAR_ARGV(i) );
        }
	}

    return OB_DOWNCAST( MK_BINARY_OBJ(stream) );
}

void do_simple_packing( vector<byte>& stream, Object *o, size_t size ){
	byte  *buffer;
	size_t i;

	if( size > ob_get_size(o) ){
		hyb_throw( H_ET_SYNTAX, "could not pack more bytes than the object owns (trying to pack type '%s' of %d bytes to %d bytes)", o->type->name, ob_get_size(o), size );
	}

	buffer = ob_serialize( o, size );
	for( i = 0; i < size; ++i ){
		stream.push_back( buffer[i] );
	}
	delete[] buffer;
}

HYBRIS_DEFINE_FUNCTION(hpack){
	if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'pack' requires at least 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(1), otInteger );

	vector<byte> stream;
	size_t 		 i, j, size( INT_ARGV(1) );
	Object      *o = HYB_ARGV(0);

	switch( o->type->code ){
		case otInteger :
		case otChar    :
		case otFloat   :
		case otString  :
		case otBinary  :
			do_simple_packing( stream, o, size );
		break;
		case otVector  :
			if( (HYB_ARGC() - 1) != ob_get_size(o) ){
				hyb_throw( H_ET_SYNTAX, "not enough parameters to pack an array of %d elements (given %d)", ob_get_size(o), HYB_ARGC() );
			}
			for( i = 1, j = 0; i < HYB_ARGC(); ++i, ++j ){
				HYB_TYPE_ASSERT( HYB_ARGV(i), otInteger );
				do_simple_packing( stream, VECTOR_UPCAST(o)->value[j], INT_ARGV(i) );
			}
		break;
		case otStructure :
			if( (HYB_ARGC() - 1) != ob_get_size(o) ){
				hyb_throw( H_ET_SYNTAX, "not enough parameters to pack a structure with %d attributes (given %d)", ob_get_size(o), HYB_ARGC() );
			}
			for( i = 1, j = 0; i < HYB_ARGC(); ++i, ++j ){
				HYB_TYPE_ASSERT( HYB_ARGV(i), otInteger );
				do_simple_packing( stream, STRUCT_UPCAST(o)->values[j], INT_ARGV(i) );
			}
		break;

		default:
			hyb_throw( H_ET_SYNTAX, "unsupported %s type in pack function", o->type->name );
	}

	return OB_DOWNCAST( MK_BINARY_OBJ(stream) );
}



