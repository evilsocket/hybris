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
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

	return OB_DOWNCAST( MK_INT_OBJ( STRING_ARGV(0).size() ) );
}

HYBRIS_DEFINE_FUNCTION(hstrfind){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'strfind' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );
	HYB_TYPE_ASSERT( HYB_ARGV(1), otString );

	int found = STRING_ARGV(0).find( STRING_ARGV(1) );

	return OB_DOWNCAST( MK_INT_OBJ( found == string::npos ? -1 : found ) );
}

HYBRIS_DEFINE_FUNCTION(hsubstr){
	if( HYB_ARGC() != 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'substr' requires 3 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

	string sub = STRING_ARGV(0).substr( ob_ivalue( HYB_ARGV(1) ), ob_ivalue( HYB_ARGV(2) ) );

	return OB_DOWNCAST( MK_STRING_OBJ( sub.c_str() ) );
}

HYBRIS_DEFINE_FUNCTION(hstrreplace){
	if( HYB_ARGC() != 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'strreplace' requires 3 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );
	HYB_TYPE_ASSERT( HYB_ARGV(1), otString );
	HYB_TYPE_ASSERT( HYB_ARGV(2), otString );

	string str  = STRING_ARGV(0),
		   find = STRING_ARGV(1),
		   repl = STRING_ARGV(2);

	unsigned int i;
	for( ; (i = str.find( find )) != string::npos ; ){
		str.replace( i, find.length(), repl );
	}

	return OB_DOWNCAST( MK_STRING_OBJ( str.c_str() ) );
}

HYBRIS_DEFINE_FUNCTION(hstrsplit){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'strsplit' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );
	HYB_TYPES_ASSERT( HYB_ARGV(1), otString, otChar );

	string str = STRING_ARGV(0),
		   tok = ob_svalue( HYB_ARGV(1) );
	vector<string> parts;
    int start = 0, end = 0, i;

  	while( (end = str.find(tok,start)) != string::npos ){
		parts.push_back( str.substr( start, end - start ) );
		start = end + tok.size();
	}
	parts.push_back( str.substr(start) );

   	Object *array = OB_DOWNCAST( MK_VECTOR_OBJ() );
	for( i = 0; i < parts.size(); ++i ){
		ob_cl_push_reference( array, OB_DOWNCAST( MK_STRING_OBJ( parts[i].c_str() ) ) );
	}

	return OB_DOWNCAST( array );
}
