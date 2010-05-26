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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <math.h>

#define MAX_STRING_SIZE 1024
#define MAX_IDENT_SIZE  100

#define HMAXARGS 200
/*
 * Macro to easily loop std::* collections.
 */
#define vv_foreach( iterator, vv ) for( iterator = (vv).begin(); iterator != (vv).end(); iterator++ )

/*
 * Function declaration descriptor.
 */
typedef struct {
    char function[0xFF];
    int  argc;
    char argv[HMAXARGS][0xFF];
    bool vargs;
}
function_decl_t;
/*
 * Method declaration descriptor.
 */
typedef struct {
    char method[0xFF];
    int  argc;
    char argv[HMAXARGS][0xFF];
    bool vargs;
}
method_decl_t;
/*
 * Command line arguments descriptor.
 */
typedef struct {
    char source[0xFF];
    char rootpath[0xFF];
    int  stacktrace;

    int   tm_timer;
    ulong tm_start;
    ulong tm_end;

	bool  cgi_mode;

    ulong gc_threshold;
    ulong mm_threshold;
}
vm_args_t;
/*
 * Describes what type a given error is.
 */
enum H_ERROR_TYPE {
    H_ET_WARNING = 0,
    H_ET_GENERIC,
    H_ET_SYNTAX
};
/*
 * Main flex error routine.
 */
void  yyerror( char *error );
/*
 * Throw an error with a given type and a give message,
 * if type == H_ET_WARNING the process will continue, otherwise
 * the process will be hard killed.
 */
void  hyb_error( H_ERROR_TYPE type, const char *format, ... );
/*
 * Check if the given file exists.
 */
int   hyb_file_exists ( char *filename );
/*
 * Get cpu ticks to compute execution time.
 */
ulong hyb_uticks();
/*
 * Perform time difference and print it in a human
 * readable form.
 */
void  hyb_timediff( ulong uticks, char *buffer );

#endif
