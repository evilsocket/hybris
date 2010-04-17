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

HYBRIS_DEFINE_FUNCTION(hmatrix);
HYBRIS_DEFINE_FUNCTION(hcolumns);
HYBRIS_DEFINE_FUNCTION(hrows);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "matrix", hmatrix },
	{ "columns", hcolumns },
	{ "rows", hrows },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hmatrix){
    if( ob_argc() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'matrix' requires at least 2 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger );
    ob_type_assert( ob_argv(1), otInteger );

    unsigned int     rows    = int_argv(0),
                     columns = int_argv(1),
                     nvalues = ob_argc() - 2,
                     i;
    vector<Object *> values;

    if( nvalues != (rows * columns) ){
        hyb_throw( H_ET_SYNTAX, "unexpected number of values for the matrix, expected %d (%dx%d), given %d", rows*columns, rows, columns, nvalues );
    }

    for( i = 2; i < data->size(); ++i ){
        values.push_back( ob_clone( ob_argv(i) ) );
    }

    return ob_dcast( gc_new_matrix( rows, columns, values ) );
}

HYBRIS_DEFINE_FUNCTION(hcolumns){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'columns' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otMatrix );

	return ob_dcast( gc_new_integer( matrix_argv(0)->columns ) );
}

HYBRIS_DEFINE_FUNCTION(hrows){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'rows' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otMatrix );

	return ob_dcast( gc_new_integer( matrix_argv(0)->rows ) );
}

