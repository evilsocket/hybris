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
    { "pcre_replace", hpcre_replace, H_REQ_ARGC(3), { H_REQ_TYPES(otString), H_REQ_TYPES(otString), H_REQ_TYPES(otChar,otString) } },
    { "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hpcre_replace){
	string rawreg,
		   subject,
		   replace,
		   pattern;
	int    		 opts, i, ccount, rc,
				*offsets,
				 eoffset,
				 offset(0);
	const char  *error;
	pcre 		*compiled;

	vm_parse_argv( "sss", &rawreg, &subject, &replace );

	string_parse_pcre( rawreg, pattern, opts );

	compiled = pcre_compile( pattern.c_str(), opts, &error, &eoffset, 0 );
	if( !compiled ){
		hyb_error( H_ET_GENERIC, "error during regex evaluation at offset %d (%s)", eoffset, error );
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
