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

void hprint_stacktrace( int force /* = 0 */ ){
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

    fflush(stderr);
	if( strchr( error, '\n' ) ){
		fprintf( stderr, "[LINE %d] %s", yylineno, error );
		hprint_stacktrace();
        __context.release(1);
    	exit(-1);
	}
	else{
		fprintf( stderr, "[LINE %d] %s .\n", yylineno, error );
		hprint_stacktrace();
        __context.release(1);
    	exit(-1);
	}
}

void hybris_generic_warning( const char *format, ... ){
    char message[0xFF] = {0},
         error[0xFF] = {0};
    va_list ap;

    va_start( ap, format );
    vsprintf( message, format, ap );
    va_end(ap);

    sprintf( error, "\033[01;33mWARNING : %s .\n\033[00m", message );
    yyerror(error);
}

void hybris_generic_error( const char *format, ... ){
    char message[0xFF] = {0},
         error[0xFF] = {0};
    va_list ap;
    extern Context __context;

    va_start( ap, format );
    vsprintf( message, format, ap );
    va_end(ap);

    sprintf( error, "\033[22;31mERROR : %s .\n\033[00m", message );
    yyerror(error);
    hprint_stacktrace();
    __context.release(1);
    exit(-1);
}

void hybris_syntax_error( const char *format, ... ){
    char message[0xFF] = {0},
         error[0xFF] = {0};
    va_list ap;
    extern int yylineno;
    extern Context __context;

    va_start( ap, format );
    vsprintf( message, format, ap );
    va_end(ap);

    sprintf( error, "\033[22;31mSyntax error on line %d : %s .\n\033[00m", yylineno, message );
    yyerror(error);
    hprint_stacktrace();
    __context.release(1);
    exit(-1);
}

void htype_assert( Object *o, H_OBJECT_TYPE type ){
	if( o->type != type ){
		hybris_syntax_error( "'%s' is not a valid variable type", Object::type_name(o) );
	}
}

void htype_assert( Object *o, H_OBJECT_TYPE type1, H_OBJECT_TYPE type2 ){
	if( o->type != type1 && o->type != type2 ){
		hybris_syntax_error( "'%s' is not a valid variable type", Object::type_name(o) );
	}
}


int h_file_exists( char *filename ){
    FILE *fp = fopen( filename, "r" );
    if( fp ){
        fclose(fp);
        return 1;
    }
    else{
        return 0;
    }
}

unsigned long h_uticks(){
    timeval ts;
    gettimeofday(&ts,0);
    return ((ts.tv_sec * 1000000) + ts.tv_usec);
}

const char * h_dtime( unsigned long uticks ){
    #define MS_DELTA (1000.0)
    #define SS_DELTA (MS_DELTA * 1000.0)
    #define MM_DELTA (SS_DELTA * 60.0)
    #define HH_DELTA (MM_DELTA * 60.0)

    char delta[0xFF] = {0};

    double ticks = (double)uticks;

    if( ticks < MS_DELTA ){
        sprintf( delta, "%lf us", ticks );
    }
    else if( ticks < SS_DELTA ){
        sprintf( delta, "%lf ms", ticks / MS_DELTA );
    }
    else if( ticks < MM_DELTA ){
        sprintf( delta, "%lf s", ticks / SS_DELTA );
    }
    else if( ticks < HH_DELTA ){
        sprintf( delta, "%lf m", ticks / MM_DELTA );
    }
    else{
        sprintf( delta, "%lf h", ticks / HH_DELTA );
    }

    return delta;
}

