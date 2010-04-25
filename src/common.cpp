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
#include "mseg.h"
#include "vm.h"
#include <time.h>
#include <sys/time.h>


void hyb_print_stacktrace( int force /* = 0 */ ){
    extern VM __hyb_vm;

    if( (__hyb_vm.args.stacktrace && __hyb_vm.stack_trace.size()) || force ){
        int tail = __hyb_vm.stack_trace.size() - 1;
        printf( "\nSTACK TRACE :\n\n" );
        for( int i = tail; i >= 0; i-- ){
            printf( "\t%.3d : %s\n", i, __hyb_vm.stack_trace[i].c_str() );
        }
        printf( "\n" );
    }
}

void yyerror( char *error ){
    extern int yylineno;
    extern VM __hyb_vm;

    /*
     * Make sure first character is uppercase.
     */
    error[0] = toupper( error[0] );

    fflush(stderr);
    /*
     * Print line number only for syntax errors.
     */
    if( strstr( error, "Syntax error : " ) ){
    	fprintf( stderr, "[LINE %d] %s%c", yylineno, error, (strchr( error, '\n' ) ? 0x00 : '\n') );
    }
    else{
    	fprintf( stderr, "%s%c", error, (strchr( error, '\n' ) ? 0x00 : '\n') );
    }

	/*
	 * If the error was triggered by a SIGSEGV signal, force
	 * the stack trace to printed.
	 */
	if( strstr( error, "SIGSEGV Signal Catched" ) ){
		hyb_print_stacktrace(1);
	}
	/*
	 * Otherwise, print it only if cmd line arguments are configured
	 * to do so.
	 */
	else{
		hyb_print_stacktrace();
	}

	__hyb_vm.closeFile();
	__hyb_vm.release();

	exit(-1);
}

void hyb_error( H_ERROR_TYPE type, const char *format, ... ){
    char message[0xFF] = {0},
         error[0xFF] = {0};
    va_list ap;
    extern int yylineno;

    va_start( ap, format );
        vsprintf( message, format, ap );
    va_end(ap);

    switch( type ){
        // simple warning
        case H_ET_WARNING :
            sprintf( error, "\033[01;33mWARNING : %s .\033[00m", message );
        break;
        // generic error (file not found, module not found, ecc)
        case H_ET_GREATER_EQNERIC :
            sprintf( error, "\033[22;31mERROR : %s .\033[00m", message );
        break;
        // same as generic one but for syntax specific errors with line number printing
        case H_ET_SYNTAX  :
            sprintf( error, "\033[22;31mSyntax error : %s .\033[00m", message );
        break;
    }

	// print error message
	yyerror(error);
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

