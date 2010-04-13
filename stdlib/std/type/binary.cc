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

        if( HYB_ARGV(i)->type == otInteger ){
            stream.push_back( (unsigned char)(long)(*HYB_ARGV(i)) );
        }
        else{
            stream.push_back( (unsigned char)(char)(*HYB_ARGV(i)) );
        }
	}

    return new Object(stream);
}

void do_simple_packing( vector<unsigned char>& stream, Object *o, int size ){
	unsigned int i;

	if( size > o->size ){
		hyb_throw( H_ET_SYNTAX, "could not pack more bytes than the object owns (trying to pack type '%s' of %d bytes to %d bytes)", Object::type_name(o), o->size, size );
	}
	switch( o->type ){
		case otInteger    :
			for( i = 0; i < size; ++i ){
				stream.push_back( ((unsigned char *)&(o->value.m_integer))[i] );
			}
		break;
		case otChar   :
			stream.push_back( (unsigned char)(char)(*o) );
		break;
		case Float_Type  :
			for( i = 0; i < size; ++i ){
				stream.push_back( ((unsigned char *)&(o->value.m_double))[i] );
			}
		break;
		case otString :
			for( i = 0; i < size; ++i ){
				stream.push_back( (unsigned char)((const char *)(*o))[i] );
			}
		break;
		case H_OT_BINARY :
			for( i = 0; i < size; ++i ){
				stream.push_back( (unsigned char)(char)(*o->value.m_array[i]) );
			}
		break;

		default:
			hyb_throw( H_ET_SYNTAX, "function 'pack' does not support nested structured types" );
	}
}

HYBRIS_DEFINE_FUNCTION(hpack){
	if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'pack' requires at least 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(1), otInteger );

	unsigned char	   	  byte;
	vector<unsigned char> stream;
	unsigned int 		  i, j, size( HYB_ARGV(1)->value.m_integer );
	Object               *o = HYB_ARGV(0);

	switch( o->type ){
		case otInteger    :
		case otChar   :
		case Float_Type  :
		case otString :
		case H_OT_BINARY :
			do_simple_packing( stream, o, size );
		break;
		case otVector  :
			if( (HYB_ARGC() - 1) != o->value.m_array.size() ){
				hyb_throw( H_ET_SYNTAX, "not enough parameters to pack an array of %d elements (given %d)", o->value.m_array.size(), HYB_ARGC() );
			}
			for( i = 1, j = 0; i < HYB_ARGC(); ++i, ++j ){
				HYB_TYPE_ASSERT( HYB_ARGV(i), otInteger );

				size = (long)(*HYB_ARGV(i));
				do_simple_packing( stream, o->value.m_array[j], size );
			}
		break;
		case H_OT_STRUCT :
			if( (HYB_ARGC() - 1) != o->value.m_struct.size() ){
				hyb_throw( H_ET_SYNTAX, "not enough parameters to pack a structure with %d attributes (given %d)", o->value.m_struct.size(), HYB_ARGC() );
			}
			for( i = 1, j = 0; i < HYB_ARGC(); ++i, ++j ){
				HYB_TYPE_ASSERT( HYB_ARGV(i), otInteger );

				size = (long)(*HYB_ARGV(i));
				do_simple_packing( stream, o->value.m_struct[j].value, size );
			}
		break;

		default:
			hyb_throw( H_ET_SYNTAX, "unsupported %s type in pack function", Object::type_name(o) );
	}

	return new Object(stream);
}



