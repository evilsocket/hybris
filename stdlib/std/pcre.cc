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
#include <pcre.h>

HYBRIS_DEFINE_FUNCTION(hpcre_replace);

HYBRIS_EXPORTED_FUNCTIONS() {
    { "pcre_replace", hpcre_replace },
    { "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hpcre_replace){
	if( ob_argc() != 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'pcre_replace' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );
	ob_types_assert( ob_argv(2), otString, otChar );

	string rawreg  = string_argv(0).c_str(),
		   subject = string_argv(1).c_str(),
		   replace = ob_is_string( ob_argv(2) ) ? string_argv(2).c_str() : string("") + (char)ob_ivalue(ob_argv(2)),
		   pattern;
	int    		 opts, i, ccount, rc,
				*offsets,
				 eoffset,
				 offset(0);
	const char  *error;
	pcre 		*compiled;

	string_parse_pcre( rawreg, pattern, opts );

	compiled = pcre_compile( pattern.c_str(), opts, &error, &eoffset, 0 );
	if( !compiled ){
		hyb_throw( H_ET_GENERIC, "error during regex evaluation at offset %d (%s)", eoffset, error );
	}

	rc = pcre_fullinfo( compiled, 0, PCRE_INFO_CAPTURECOUNT, &ccount );

	offsets = new int[ 3 * (ccount + 1) ];

	while( (rc = pcre_exec( compiled, 0, subject.c_str(), subject.length(), offset, 0, offsets, 3 * (ccount + 1) )) > 0 ){
		const char *data;
		for( i = 1; i < rc; ++i ){
			pcre_get_substring( subject.c_str(), offsets, rc, i, &data );
			subject.replace( offsets[2*i], strlen(data), replace );
		}
		offset = offsets[1];
	}
	delete[] offsets;

	return ob_dcast( gc_new_string( subject.c_str() ) );
}
