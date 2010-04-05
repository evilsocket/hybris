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

HYBRIS_BUILTIN(hmatrix){
    if( data->size() < 2 ){
		hyb_syntax_error( "function 'matrix' requires at least 2 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT );
    hyb_type_assert( data->at(1), H_OT_INT );

    unsigned int     rows    = data->at(0)->value.m_integer,
                     columns = data->at(1)->value.m_integer,
                     nvalues = data->size() - 2,
                     i;
    vector<Object *> values;

    if( nvalues != (rows * columns) ){
        hyb_syntax_error( "unexpected number of values for the matrix, expected %d (%dx%d), given %d", rows*columns, rows, columns, nvalues );
    }

    for( i = 2; i < data->size(); i++ ){
        values.push_back( new Object( data->at(i) ) );
    }

    return new Object( rows, columns, values );
}

HYBRIS_BUILTIN(hcolumns){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'columns' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_MATRIX );
	return new Object( static_cast<long>( data->at(0)->value.m_columns ) );
}

HYBRIS_BUILTIN(hrows){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'rows' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_MATRIX );
	return new Object( static_cast<long>( data->at(0)->value.m_rows ) );
}

