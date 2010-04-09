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
#ifndef _HCOMMON_H_
#	define _HCOMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include "object.h"

#define HMAXARGS 200

typedef struct {
    char function[0xFF];
    int  argc;
    char argv[HMAXARGS][0xFF];
}
function_decl_t;

typedef struct {
    char source[0xFF];
    char rootpath[0xFF];
    int  stacktrace;

    int           tm_timer;
    ulong tm_start;
    ulong tm_end;
}
h_args_t;

void yyerror( char *error );

void  hyb_generic_warning( const char *format, ... );
void  hyb_generic_error( const char *format, ... );
void  hyb_syntax_error( const char *format, ... );
void  hyb_type_assert( Object *o, H_OBJECT_TYPE type );
void  hyb_type_assert( Object *o, H_OBJECT_TYPE type1, H_OBJECT_TYPE type2 );
void  hyb_print_stacktrace( int force = 0 );
int   hyb_file_exists ( char *filename );
ulong hyb_uticks();
void  hyb_timediff( ulong uticks, char *buffer );

#endif
