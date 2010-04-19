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
#include "hybris.h"
#include "common.h"
#include "vmem.h"
#include "context.h"
#include <time.h>
#include <sys/time.h>


void hyb_print_stacktrace( int force /* = 0 */ ){
    extern Context __context;

    if( (__context.args.stacktrace && __context.stack_trace.size()) || force ){
        int tail = __context.stack_trace.size() - 1;
        printf( "\nSTACK TRACE :\n\n" );
        for( int i = tail; i >= 0; i-- ){
            printf( "\t%.3d : %s\n", i, __context.stack_trace[i].c_str() );
        }
        printf( "\n" );
    }
}

void yyerror( char *error ){
    extern int yylineno;
    extern Context __context;

    /*
     * Make sure first character is uppercase.
     */
    error[0] = toupper( error[0] );

    fflush(stderr);
	if( strchr( error, '\n' ) ){
		fprintf( stderr, "[LINE %d] %s", yylineno, error );
		hyb_print_stacktrace();
        __context.release();
    	exit(-1);
	}
	else{
		fprintf( stderr, "[LINE %d] %s .\n", yylineno, error );
		hyb_print_stacktrace();
        __context.release();
    	exit(-1);
	}
}

void hyb_throw( H_ERROR_TYPE type, const char *format, ... ){
    char message[0xFF] = {0},
         error[0xFF] = {0};
    va_list ap;
    bool fault(false);
    extern int yylineno;

    va_start( ap, format );
        vsprintf( message, format, ap );
    va_end(ap);

    switch( type ){
        // simple warning, only print message and continue
        case H_ET_WARNING :
            sprintf( error, "\033[01;33mWARNING : %s .\n\033[00m", message );
            yyerror(error);
        break;
        // generic error (file not found, module not found, ecc), print, release and exit
        case H_ET_GENERIC :
            sprintf( error, "\033[22;31mERROR : %s .\n\033[00m", message );
            fault = true;
        break;
        // syntax error, same as generic one but print line number
        case H_ET_SYNTAX  :
            sprintf( error, "\033[22;31mSyntax error : %s .\n\033[00m", message );
            fault = true;
        break;
    }

    // print error message
    yyerror(error);

    if( fault ){
        extern Context __context;
        // print function stack trace
        hyb_print_stacktrace();
        // release the context
        __context.release();
        // exit
        exit(-1);
    }
}

int hyb_file_exists( char *filename ){
	struct stat sbuff;
	return ( stat(filename, &sbuff) == 0 );
}

ulong hyb_uticks(){
    timeval ts;
    gettimeofday(&ts,0);
    return ((ts.tv_sec * 1000000) + ts.tv_usec);
}

void hyb_timediff( ulong uticks, char *buffer ){
    #define MS_DELTA (1000.0)
    #define SS_DELTA (MS_DELTA * 1000.0)
    #define MM_DELTA (SS_DELTA * 60.0)
    #define HH_DELTA (MM_DELTA * 60.0)

    double ticks = (double)uticks;

    if( ticks < MS_DELTA ){
        sprintf( buffer, "%lf us", ticks );
    }
    else if( ticks < SS_DELTA ){
        sprintf( buffer, "%lf ms", ticks / MS_DELTA );
    }
    else if( ticks < MM_DELTA ){
        sprintf( buffer, "%lf s", ticks / SS_DELTA );
    }
    else if( ticks < HH_DELTA ){
        sprintf( buffer, "%lf m", ticks / MM_DELTA );
    }
    else{
        sprintf( buffer, "%lf h", ticks / HH_DELTA );
    }
}

