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

HYBRIS_BUILTIN(hprint){
    unsigned int i;
    for( i = 0; i < data->size(); i++ ){
        data->at(i)->print();
    }
    return NULL;
}

HYBRIS_BUILTIN(hprintln){
    if( data->size() ){
        unsigned int i;
        for( i = 0; i < data->size(); i++ ){
            data->at(i)->println();
        }
    }
    else{
        printf("\n");
    }
    return NULL;
}

HYBRIS_BUILTIN(hinput){
    Object *_return;
    if( data->size() == 2 ){
        data->at(0)->print();
        data->at(1)->input();
        _return = data->at(1);
    }
    else if( data->size() == 1 ){
        data->at(0)->input();
        _return = data->at(0);
    }
	else{
		hybris_syntax_error( "function 'input' requires 1 or 2 parameters (called with %d)", data->size() );
	}

    return _return;
}