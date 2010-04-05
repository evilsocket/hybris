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

HYBRIS_BUILTIN(hstrlen){
	if( data->size() != 1 ){
		hyb_syntax_error( "function 'strlen' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_STRING );

	return new Object( static_cast<long>( data->at(0)->value.m_string.size() ) );
}

HYBRIS_BUILTIN(hstrfind){
	if( data->size() != 2 ){
		hyb_syntax_error( "function 'strfind' requires 2 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_STRING );
	hyb_type_assert( data->at(1), H_OT_STRING );

	int found = data->at(0)->value.m_string.find( data->at(1)->value.m_string );

	return new Object( static_cast<long>(found == string::npos ? -1 : found) );
}

HYBRIS_BUILTIN(hsubstr){
	if( data->size() != 3 ){
		hyb_syntax_error( "function 'substr' requires 3 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_STRING );

	string sub = data->at(0)->value.m_string.substr( data->at(1)->lvalue(), data->at(2)->lvalue() );

	return new Object( (char *)sub.c_str() );
}

HYBRIS_BUILTIN(hstrreplace){
	if( data->size() != 3 ){
		hyb_syntax_error( "function 'strreplace' requires 3 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_STRING );
	hyb_type_assert( data->at(1), H_OT_STRING );
	hyb_type_assert( data->at(2), H_OT_STRING );

	string str  = data->at(0)->value.m_string,
		   find = data->at(1)->value.m_string,
		   repl = data->at(2)->value.m_string;

	unsigned int i;
	for( ; (i = str.find( find )) != string::npos ; ){
		str.replace( i, find.length(), repl );
	}

	return new Object( (char *)str.c_str() );
}

HYBRIS_BUILTIN(hstrsplit){
	if( data->size() != 2 ){
		hyb_syntax_error( "function 'strsplit' requires 2 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_STRING );
	hyb_type_assert( data->at(1), H_OT_STRING, H_OT_CHAR );

	string str = data->at(0)->value.m_string,
		   tok = (data->at(1)->type == H_OT_STRING ? data->at(1)->value.m_string : string("") + data->at(1)->value.m_char);
	vector<string> parts;
    int start = 0, end = 0, i;

  	while( (end = str.find(tok,start)) != string::npos ){
		parts.push_back( str.substr( start, end - start ) );
		start = end + tok.size();
	}
	parts.push_back( str.substr(start) );

   	Object *array = new Object();
	for( i = 0; i < parts.size(); i++ ){
		Object tmp( (char *)parts[i].c_str() );
		array->push(&tmp);
	}

	return array;
}
