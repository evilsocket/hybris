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
#include "memory.h"
#include "vm.h"
#include <time.h>
#include <sys/time.h>

#ifndef MAX_STRING_SIZE
#	define MAX_STRING_SIZE 1024
#endif

#define MAX_MESSAGE_SIZE MAX_STRING_SIZE + 0xFF

void yyerror( char *error ){
    extern int yylineno;
    extern vm_t *__hyb_vm;

    /*
     * Make sure first character is uppercase.
     */
    error[0] = toupper( error[0] );

    /*
     * If we are in CGI mode (stderr redirected to stdout), remove
     * all color bytes from the error string.
     */
    if( __hyb_vm->args.cgi_mode && strchr( error, '\033' ) ){
    	error += strlen( "\033[0133m" ) + 1;
    	*strrchr( error, '\033' ) = 0x00;
    }

    fflush(stderr);
    /*
     * Print line number only for syntax errors.
     */
    if( strstr( error, "Syntax error" ) ){
    	fprintf( stderr, "[LINE %d] %s%c", yylineno, error, (strchr( error, '\n' ) ? 0x00 : '\n') );
    }
    else{
    	fprintf( stderr, "%s%c", error, (strchr( error, '\n' ) ? 0x00 : '\n') );
    }

   /*
	* If the error was triggered by a SIGSEGV signal, force
	* the stack trace to printed.
	*/
	vm_print_stack_trace( __hyb_vm, (strstr( error, "SIGSEGV Signal Catched" ) != NULL) );
	/*
	 * If an error occurred during releasing phase, an error in one class destructor
	 * called from gc_release for instance, prevent to call those methods recursively.
	 * because that would cause a stack corruption.
	 *
	 * Fixes #575284
	 */
    if( __hyb_vm->releasing == false ){

		vm_fclose( __hyb_vm );
		vm_release( __hyb_vm );

		delete __hyb_vm;
	}

	exit(-1);
}

void hyb_error( H_ERROR_TYPE type, const char *format, ... ){
    char message[MAX_MESSAGE_SIZE] = {0},
         error[MAX_MESSAGE_SIZE] = {0};
    va_list ap;
    extern int yylineno;

    va_start( ap, format );
        vsnprintf( message, MAX_MESSAGE_SIZE, format, ap );
    va_end(ap);

    switch( type ){
        // simple warning
        case H_ET_WARNING :
            fprintf( stderr, "\033[01;33mWARNING : %s .\033[00m\n", message );
            return;
        break;
        // generic error (file not found, module not found, ecc)
        case H_ET_GENERIC :
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

