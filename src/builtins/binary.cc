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
#include "common.h"
#include "vmem.h"
#include "context.h"

HYBRIS_BUILTIN(hbinary){
    if( data->size() < 1 ){
		hybris_syntax_error( "function 'binary' requires at least 1 parameter (called with %d)", data->size() );
	}
	vector<unsigned char> stream;
	unsigned int          i;
	Object               *integer = H_UNDEFINED;

	for( i = 0; i < data->size(); ++i ){
        htype_assert( data->at(i), H_OT_INT, H_OT_CHAR );
        integer = data->at(i)->toInt();

        stream.push_back( (unsigned char)integer->value.m_integer );

        delete integer;
	}

    return new Object(stream);
}

void do_simple_packing( vector<unsigned char>& stream, Object *o, int size ){
	unsigned int i;
	
	if( size > o->size ){
		hybris_syntax_error( "could not pack more bytes than the object owns (trying to pack type '%s' of %d bytes to %d bytes)", Object::type_name(o), o->size, size );
	}
	switch( o->type ){
		case H_OT_INT    : 
			for( i = 0; i < size; ++i ){
				stream.push_back( ((unsigned char *)&(o->value.m_integer))[i] );
			}	
		break;
		case H_OT_CHAR   : 
			stream.push_back( (unsigned char)o->value.m_char );
		break;
		case H_OT_FLOAT  : 
			for( i = 0; i < size; ++i ){
				stream.push_back( ((unsigned char *)&(o->value.m_double))[i] );
			}
		break;
		case H_OT_STRING : 
			for( i = 0; i < size; ++i ){
				stream.push_back( (unsigned char)o->value.m_string[i] );
			}
		break;
		case H_OT_BINARY : 
			for( i = 0; i < size; ++i ){
				stream.push_back( (unsigned char)o->value.m_array[i]->value.m_char );
			}
		break;
		
		default:
			hybris_syntax_error( "function 'pack' does not support nested structured types" );
	}
}

HYBRIS_BUILTIN(hpack){
	if( data->size() < 2 ){
		hybris_syntax_error( "function 'pack' requires at least 2 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(1), H_OT_INT );
	
	unsigned char	   	  byte;
	vector<unsigned char> stream;
	unsigned int 		  i, j, size( data->at(1)->value.m_integer );
	Object               *o = data->at(0);
		
	switch( o->type ){
		case H_OT_INT    : 
		case H_OT_CHAR   : 
		case H_OT_FLOAT  : 
		case H_OT_STRING : 
		case H_OT_BINARY : 
			do_simple_packing( stream, o, size );
		break;
		case H_OT_ARRAY  : 
			if( (data->size() - 1) != o->value.m_array.size() ){
				hybris_syntax_error( "not enough parameters to pack an array of %d elements (given %d)", o->value.m_array.size(), data->size() );
			}
			for( i = 1, j = 0; i < data->size(); ++i, ++j ){
				htype_assert( data->at(i), H_OT_INT );
				size = data->at(i)->value.m_integer;
				do_simple_packing( stream, o->value.m_array[j], size );
			}
		break;
		case H_OT_STRUCT : 
			if( (data->size() - 1) != o->value.m_struct_values.size() ){
				hybris_syntax_error( "not enough parameters to pack a structure with %d attributes (given %d)", o->value.m_struct_values.size(), data->size() );
			}
			for( i = 1, j = 0; i < data->size(); ++i, ++j ){
				htype_assert( data->at(i), H_OT_INT );
				size = data->at(i)->value.m_integer;
				do_simple_packing( stream, o->value.m_struct_values[j], size );
			}
		break;
		
		default:
			hybris_syntax_error( "unsupported %s type in pack function", Object::type_name(o) );
	}
	
	return new Object(stream);
}



