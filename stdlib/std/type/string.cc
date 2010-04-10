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

HYBRIS_DEFINE_FUNCTION(hstrlen);
HYBRIS_DEFINE_FUNCTION(hstrfind);
HYBRIS_DEFINE_FUNCTION(hsubstr);
HYBRIS_DEFINE_FUNCTION(hstrreplace);
HYBRIS_DEFINE_FUNCTION(hstrsplit);

extern "C" named_function_t hybris_module_functions[] = {
	{ "strlen", hstrlen },
	{ "strfind", hstrfind },
	{ "substr", hsubstr },
	{ "strreplace", hstrreplace },
	{ "strsplit", hstrsplit },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hstrlen){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'strlen' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

	return MK_INT_OBJ( HYB_ARGV(0)->value.m_string.size() );
}

HYBRIS_DEFINE_FUNCTION(hstrfind){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'strfind' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );
	HYB_TYPE_ASSERT( HYB_ARGV(1), H_OT_STRING );

	int found = HYB_ARGV(0)->value.m_string.find( HYB_ARGV(1)->value.m_string );

	return MK_INT_OBJ( found == string::npos ? -1 : found );
}

HYBRIS_DEFINE_FUNCTION(hsubstr){
	if( HYB_ARGC() != 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'substr' requires 3 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

	string sub = HYB_ARGV(0)->value.m_string.substr( HYB_ARGV(1)->lvalue(), HYB_ARGV(2)->lvalue() );

	return MK_STRING_OBJ( sub.c_str() );
}

HYBRIS_DEFINE_FUNCTION(hstrreplace){
	if( HYB_ARGC() != 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'strreplace' requires 3 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );
	HYB_TYPE_ASSERT( HYB_ARGV(1), H_OT_STRING );
	HYB_TYPE_ASSERT( HYB_ARGV(2), H_OT_STRING );

	string str  = HYB_ARGV(0)->value.m_string,
		   find = HYB_ARGV(1)->value.m_string,
		   repl = HYB_ARGV(2)->value.m_string;

	unsigned int i;
	for( ; (i = str.find( find )) != string::npos ; ){
		str.replace( i, find.length(), repl );
	}

	return MK_STRING_OBJ( str.c_str() );
}

HYBRIS_DEFINE_FUNCTION(hstrsplit){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'strsplit' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );
	HYB_TYPES_ASSERT( HYB_ARGV(1), H_OT_STRING, H_OT_CHAR );

	string str = HYB_ARGV(0)->value.m_string,
		   tok = (HYB_ARGV(1)->type == H_OT_STRING ? HYB_ARGV(1)->value.m_string : string("") + HYB_ARGV(1)->value.m_char);
	vector<string> parts;
    int start = 0, end = 0, i;

  	while( (end = str.find(tok,start)) != string::npos ){
		parts.push_back( str.substr( start, end - start ) );
		start = end + tok.size();
	}
	parts.push_back( str.substr(start) );

   	Object *array = MK_COLLECTION_OBJ();
	for( i = 0; i < parts.size(); i++ ){
		array->push_ref( MK_STRING_OBJ( parts[i].c_str() ) );
	}

	return array;
}
