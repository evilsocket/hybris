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

HYBRIS_DEFINE_FUNCTION(hprint);
HYBRIS_DEFINE_FUNCTION(hprintln);
HYBRIS_DEFINE_FUNCTION(hinput);

extern "C" named_function_t hybris_module_functions[] = {
	{ "print", hprint },
	{ "println", hprintln },
	{ "input", hinput },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hprint){
    unsigned int i;
    for( i = 0; i < HYB_ARGC(); ++i ){
        ob_print( HYB_ARGV(i) );
    }
    return NULL;
}

HYBRIS_DEFINE_FUNCTION(hprintln){
    if( HYB_ARGC() ){
        unsigned int i;
        for( i = 0; i < HYB_ARGC(); ++i ){
            ob_print( HYB_ARGV(i) );
            printf( "\n" );
        }
    }
    else{
        printf("\n");
    }
    return NULL;
}

HYBRIS_DEFINE_FUNCTION(hinput){
    Object *_return;
    if( HYB_ARGC() == 2 ){
        ob_print( HYB_ARGV(0) );
        ob_input( HYB_ARGV(1) );
        _return = HYB_ARGV(1);
    }
    else if( HYB_ARGC() == 1 ){
        ob_input( HYB_ARGV(0) );
        _return = HYB_ARGV(0);
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'input' requires 1 or 2 parameters (called with %d)", HYB_ARGC() );
	}

    return _return;
}
