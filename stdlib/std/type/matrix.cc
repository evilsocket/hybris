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

extern "C" named_function_t hybris_module_functions[] = {
	{ "matrix", hmatrix },
	{ "columns", hcolumns },
	{ "rows", hrows },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hmatrix){
    if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'matrix' requires at least 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );
    HYB_TYPE_ASSERT( HYB_ARGV(1), H_OT_INT );

    unsigned int     rows    = HYB_ARGV(0)->value.m_integer,
                     columns = HYB_ARGV(1)->value.m_integer,
                     nvalues = HYB_ARGC() - 2,
                     i;
    vector<Object *> values;

    if( nvalues != (rows * columns) ){
        hyb_throw( H_ET_SYNTAX, "unexpected number of values for the matrix, expected %d (%dx%d), given %d", rows*columns, rows, columns, nvalues );
    }

    for( i = 2; i < data->size(); i++ ){
        values.push_back( new Object( HYB_ARGV(i) ) );
    }

    return new Object( rows, columns, values );
}

HYBRIS_DEFINE_FUNCTION(hcolumns){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'columns' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_MATRIX );
	return new Object( static_cast<long>( HYB_ARGV(0)->value.m_columns ) );
}

HYBRIS_DEFINE_FUNCTION(hrows){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'rows' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_MATRIX );
	return new Object( static_cast<long>( HYB_ARGV(0)->value.m_rows ) );
}

