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
#include <dlfcn.h>

HYBRIS_BUILTIN(hdllopen){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'dllopen' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

    return new Object( (int)dlopen( data->at(0)->xstring.c_str(), RTLD_LAZY ) );
}

HYBRIS_BUILTIN(hdlllink){
    if( data->size() != 2 ){
		hybris_syntax_error( "function 'dlllink' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT    );
	htype_assert( data->at(1), H_OT_STRING );

    void *hdll = (void *)data->at(0)->xint;

    return new Object( (int)dlsym( hdll, data->at(1)->xstring.c_str() ), 1 );
}

HYBRIS_BUILTIN(hdllcall){
    if( data->size() < 1 ){
        hybris_syntax_error( "function 'dllopen' requires at least 1 parameter (called with %d)", data->size() );
    }
    htype_assert( data->at(0), H_OT_INT );

    typedef int (* function_t)(void);
    function_t function = (function_t)data->at(0)->xint;

    unsigned int ulval;
    vector<unsigned char *> garbage;
    unsigned long * stack = new unsigned long[ data->size() - 1 ];
    int    iv;
    double fv;
    char   cv;
    int i, j;
    /* push in reverse order */
    for( i = data->size() - 1, j = 0; i >= 1; i--, j++ ){
        Object        *arg  = data->at(i);
        unsigned char *raw  = arg->serialize();
        garbage.push_back(raw);
        switch( arg->xtype ){
            case H_OT_INT    : memcpy( &iv, raw, sizeof(int) );    ulval = (unsigned long)iv; break;
            case H_OT_FLOAT  : memcpy( &fv, raw, sizeof(double) ); ulval = (unsigned long)fv; break;
            case H_OT_CHAR   : memcpy( &cv, raw, sizeof(char) );   ulval = (unsigned long)cv; break;
            case H_OT_STRING : ulval = (unsigned long)raw; 									  break;
			case H_OT_MAP    : ulval = (unsigned long)raw; 									  break;
			case H_OT_ALIAS  : memcpy( &ulval, raw, sizeof(unsigned int) ); 				  break;

            default :
               hybris_syntax_error( "could not use '%s' type for dllcall function", Object::type(arg) );
        }
        /* fill the stack vector */
        stack[j] = ulval;
    }

    /* save all the registers and the stack pointer */
    unsigned long esp;
    asm __volatile__ ( "pusha" );
    asm __volatile__ ( "mov %%esp, %0" :"=m" (esp));

    int loop = data->size() - 1;
    for( i = 0; i < loop; i++ ){
        ulval = stack[i];
        asm __volatile__ ( "push %0" :: "m"(ulval) );
    }

    ulval = function();

    /* restore registers and stack pointer */
    asm __volatile__ ( "mov %0, %%esp" :: "m" (esp) );
    asm __volatile__ ( "popa" );

    delete[] stack;
    for( int i = 0; i < garbage.size(); i++ ){
        delete[] garbage[i];
    }

    return new Object((int)ulval);
}

HYBRIS_BUILTIN(hdllclose){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'dllclose' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );

	dlclose( (void *)data->at(0)->xint );

    return new Object(0);
}
