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
#include <pcrecpp.h>

HYBRIS_DEFINE_FUNCTION(hrex_match);
HYBRIS_DEFINE_FUNCTION(hrex_matches);
HYBRIS_DEFINE_FUNCTION(hrex_replace);

extern "C" named_function_t hybris_module_functions[] = {
    { "rex_match", hrex_match },
    { "rex_matches", hrex_matches },
    { "rex_replace", hrex_replace },
    { "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hrex_match){
	if( ob_argc() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'rex_match' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );

	string rawreg  = string_argv(0),
		   subject = string_argv(1),
		   regex;
	int    opts;

	string_parse_pcre( rawreg, regex, opts );

	pcrecpp::RE_Options OPTS(opts);
	pcrecpp::RE         REGEX( regex.c_str(), OPTS );

	return ob_dcast( gc_new_integer( REGEX.PartialMatch(subject.c_str()) ) );
}

HYBRIS_DEFINE_FUNCTION(hrex_matches){
	if( ob_argc() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'rex_matches' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );

	string rawreg  = string_argv(0),
		   subject = string_argv(1),
		   regex;
	int    opts, i = 0;

	string_parse_pcre( rawreg, regex, opts );

	pcrecpp::RE_Options  OPTS(opts);
	pcrecpp::RE          REGEX( regex.c_str(), OPTS );
	pcrecpp::StringPiece SUBJECT( subject.c_str() );
	string   match;
	Object  *matches = ob_dcast( gc_new_vector() );

    while( REGEX.FindAndConsume( &SUBJECT, &match ) == true ){
		if( i++ > H_PCRE_MAX_MATCHES ){
			hyb_throw( H_ET_GENERIC, "something of your regex is forcing infinite matches" );
		}
		ob_cl_push_reference( matches, ob_dcast( gc_new_string(match.c_str()) ) );
	}

	return matches;
}

HYBRIS_DEFINE_FUNCTION(hrex_replace){
	if( ob_argc() != 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'rex_replace' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );
	ob_types_assert( ob_argv(2), otString, otChar );

	string rawreg  = string_argv(0).c_str(),
		   subject = string_argv(1).c_str(),
		   replace = ob_is_string( ob_argv(2) ) ? string_argv(2).c_str() : string("") + (char)ob_ivalue(ob_argv(2)),
		   regex;
	int    opts;

	string_parse_pcre( rawreg, regex, opts );

	pcrecpp::RE_Options OPTS(opts);
	pcrecpp::RE         REGEX( regex.c_str(), OPTS );

	REGEX.GlobalReplace( replace.c_str(), &subject );

	return ob_dcast( gc_new_string( subject.c_str() ) );
}
