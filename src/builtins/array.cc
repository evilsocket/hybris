/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "common.h"
#include "vmem.h"
#include "builtin.h"

HYBRIS_BUILTIN(harray){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < data->size(); i++ ){
		array->push( data->at(i) );
	}
	return array;
}

HYBRIS_BUILTIN(helements){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'elements' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );

	return new Object( static_cast<long>( data->at(0)->xarray.size() ) );
}

HYBRIS_BUILTIN(hpop){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'pop' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );

	return new Object( data->at(0)->pop() );
}

HYBRIS_BUILTIN(hremove){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'remove' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );

	return new Object( data->at(0)->remove(data->at(1)) );
}

HYBRIS_BUILTIN(hcontains){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'contains' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );
	Object *array = data->at(0),
		    *find  = data->at(1);
	unsigned int i;

	for( i = 0; i < array->xarray.size(); i++ ){
		Object *boolean = ((*find) == array->xarray[i]);
		if( boolean->lvalue() ){
			delete boolean;
			return new Object( static_cast<long>(i) );
		}
		delete boolean;
	}

	return new Object( static_cast<long>(-1) );
}

