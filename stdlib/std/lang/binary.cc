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

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "binary", hbinary, H_REQ_ARGC(1), { H_REQ_TYPES(otInteger,otChar) } },
	{ "pack",   hpack,   H_REQ_ARGC(2), { H_ANY_TYPE, H_REQ_TYPES(otInteger) } },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hbinary){
	vector<unsigned char> stream;
	unsigned int          i;

	for( i = 0; i < data->size(); ++i ){
		ob_argv_types_assert( i, otInteger, otChar, "binary" );
		stream.push_back( (unsigned char)ob_ivalue( vm_argv(i) ) );
	}

    return ob_dcast( gc_new_binary(stream) );
}

void do_simple_packing( vector<byte>& stream, Object *o, size_t size ){
	byte  *buffer;
	size_t i;

	if( size > ob_get_size(o) ){
		hyb_error( H_ET_SYNTAX, "could not pack more bytes than the object owns (trying to pack type '%s' of %d bytes to %d bytes)", o->type->name, ob_get_size(o), size );
	}

	buffer = ob_serialize( o, size );
	for( i = 0; i < size; ++i ){
		stream.push_back( buffer[i] );
	}
	delete[] buffer;
}

HYBRIS_DEFINE_FUNCTION(hpack){
	Object      *o;
	size_t 		 i, j, size;
	vector<byte> stream;

	vm_parse_argv( "Oi", &o, &size );

	switch( o->type->code ){
		case otInteger :
		case otChar    :
		case otFloat   :
		case otString  :
		case otBinary  :
			do_simple_packing( stream, o, size );
		break;
		case otVector  :
			if( (vm_argc() - 1) != ob_get_size(o) ){
				hyb_error( H_ET_SYNTAX, "not enough parameters to pack an array of %d elements (given %d)", ob_get_size(o), vm_argc() );
			}
			for( i = 1, j = 0; i < vm_argc(); ++i, ++j ){
				ob_argv_type_assert( i, otInteger, "pack" );
				do_simple_packing( stream, ob_vector_ucast(o)->value[j], ob_ivalue( vm_argv(i) ) );
			}
		break;
		case otStructure :
			if( (vm_argc() - 1) != ob_get_size(o) ){
				hyb_error( H_ET_SYNTAX, "not enough parameters to pack a structure with %d attributes (given %d)", ob_get_size(o), vm_argc() );
			}
			for( i = 1, j = 0; i < vm_argc(); ++i, ++j ){
				ob_argv_type_assert( i, otInteger, "pack" );
				do_simple_packing( stream, ob_struct_ucast(o)->s_attributes.at(j), ob_ivalue( vm_argv(i) ) );
			}
		break;

		default:
			hyb_error( H_ET_SYNTAX, "unsupported %s type in pack function", o->type->name );
	}

	return ob_dcast( gc_new_binary(stream) );
}



