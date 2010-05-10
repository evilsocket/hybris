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
#include <algorithm>

HYBRIS_DEFINE_FUNCTION(hstrlen);
HYBRIS_DEFINE_FUNCTION(hstrfind);
HYBRIS_DEFINE_FUNCTION(hsubstr);
HYBRIS_DEFINE_FUNCTION(hstrreplace);
HYBRIS_DEFINE_FUNCTION(hstrsplit);
HYBRIS_DEFINE_FUNCTION(htrim);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "strlen", hstrlen },
	{ "strfind", hstrfind },
	{ "substr", hsubstr },
	{ "strreplace", hstrreplace },
	{ "strsplit", hstrsplit },
	{ "trim",     htrim },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hstrlen){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'strlen' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

	return ob_dcast( gc_new_integer( string_argv(0).size() ) );
}

HYBRIS_DEFINE_FUNCTION(hstrfind){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'strfind' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );

	int found = string_argv(0).find( string_argv(1) );

	return ob_dcast( gc_new_integer( found == string::npos ? -1 : found ) );
}

HYBRIS_DEFINE_FUNCTION(hsubstr){
	if( ob_argc() < 2 ){
		hyb_error( H_ET_SYNTAX, "function 'substr' requires at least 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

	string sub;
	if( ob_argc() == 2 ){
		sub = string_argv(0).substr( ob_ivalue( ob_argv(1) ) );
	}
	else{
		sub = string_argv(0).substr( ob_ivalue( ob_argv(1) ), ob_ivalue( ob_argv(2) ) );
	}

	return ob_dcast( gc_new_string( sub.c_str() ) );
}

HYBRIS_DEFINE_FUNCTION(hstrreplace){
	if( ob_argc() != 3 ){
		hyb_error( H_ET_SYNTAX, "function 'strreplace' requires 3 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );
	ob_type_assert( ob_argv(2), otString );

	string str  = string_argv(0),
		   find = string_argv(1),
		   repl = string_argv(2);

	unsigned int i;
	for( ; (i = str.find( find )) != string::npos ; ){
		str.replace( i, find.length(), repl );
	}

	return ob_dcast( gc_new_string( str.c_str() ) );
}

HYBRIS_DEFINE_FUNCTION(hstrsplit){
	if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'strsplit' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );
	ob_types_assert( ob_argv(1), otString, otChar );

	string str = string_argv(0),
		   tok = ob_svalue( ob_argv(1) );
	vector<string> parts;
    int start = 0, end = 0, i;

  	while( (end = str.find(tok,start)) != string::npos ){
		parts.push_back( str.substr( start, end - start ) );
		start = end + tok.size();
	}
	parts.push_back( str.substr(start) );

   	Object *array = ob_dcast( gc_new_vector() );
	for( i = 0; i < parts.size(); ++i ){
		ob_cl_push_reference( array, ob_dcast( gc_new_string( parts[i].c_str() ) ) );
	}

	return ob_dcast( array );
}

HYBRIS_DEFINE_FUNCTION(htrim){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'trim' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

	string s = ob_string_val( ob_argv(0) );

	// trim from start
	s.erase( s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))) );
	// trim from end
	s.erase( std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end() );

	return (Object *)gc_new_string(s.c_str());
}
