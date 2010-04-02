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
#include <pcrecpp.h>

HYBRIS_BUILTIN(hrex_match){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'rex_match' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );

	string rawreg  = data->at(0)->value.m_string,
		   subject = data->at(1)->value.m_string,
		   regex;
	int    opts;

	Object::parse_pcre( rawreg, regex, opts );

	pcrecpp::RE_Options OPTS(opts);
	pcrecpp::RE         REGEX( regex.c_str(), OPTS );

	return new Object( (long)REGEX.PartialMatch(subject.c_str()) );
}

HYBRIS_BUILTIN(hrex_matches){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'rex_matches' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );

	string rawreg  = data->at(0)->value.m_string,
		   subject = data->at(1)->value.m_string,
		   regex;
	int    opts, i = 0;

	Object::parse_pcre( rawreg, regex, opts );

	pcrecpp::RE_Options  OPTS(opts);
	pcrecpp::RE          REGEX( regex.c_str(), OPTS );
	pcrecpp::StringPiece SUBJECT( subject.c_str() );
	string   match;
	Object *matches = new Object();

    while( REGEX.FindAndConsume( &SUBJECT, &match ) == true ){
		if( i++ > H_PCRE_MAX_MATCHES ){
			hybris_generic_error( "something of your regex is forcing infinite matches" );
		}
		matches->push( new Object((char *)match.c_str()) );
	}

	return matches;
}

HYBRIS_BUILTIN(hrex_replace){
	if( data->size() != 3 ){
		hybris_syntax_error( "function 'rex_replace' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );
	htype_assert( data->at(2), H_OT_STRING, H_OT_CHAR );

	string rawreg  = data->at(0)->value.m_string,
		   subject = data->at(1)->value.m_string,
		   replace = (data->at(2)->type == H_OT_STRING ? data->at(2)->value.m_string : string("") + data->at(2)->value.m_char),
		   regex;
	int    opts;

	Object::parse_pcre( rawreg, regex, opts );

	pcrecpp::RE_Options OPTS(opts);
	pcrecpp::RE         REGEX( regex.c_str(), OPTS );

	REGEX.GlobalReplace( replace.c_str(), &subject );

	return new Object( (char *)subject.c_str() );
}
