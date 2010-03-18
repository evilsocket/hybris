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
#include "hybris.h"
#include "common.h"
#include "vmem.h"
#include "builtin.h"

void hprint_stacktrace( int force /* = 0 */ ){
    extern h_context_t HCTX;

    if( (HCTX.HARGS.stacktrace && HCTX.HSTACKTRACE.size()) || force ){
        int tail = HCTX.HSTACKTRACE.size() - 1;
        printf( "\nSTACK TRACE :\n\n" );
        for( int i = tail; i >= 0; i-- ){
            printf( "\t%.3d : %s\n", i, HCTX.HSTACKTRACE[i].c_str() );
        }
        printf( "\n" );
    }
}

void yyerror( char *error ){
    extern int yylineno;
    extern h_context_t HCTX;

    fflush(stderr);
	if( strchr( error, '\n' ) ){
		fprintf( stderr, "[LINE %d] %s", yylineno, error );
		hprint_stacktrace();
        h_env_release( &HCTX, 1 );
    	exit(-1);
	}
	else{
		fprintf( stderr, "[LINE %d] %s .\n", yylineno, error );
		hprint_stacktrace();
        h_env_release( &HCTX, 1 );
    	exit(-1);
	}
}

void hybris_generic_warning( const char *format, ... ){
    char message[0xFF] = {0},
         error[0xFF] = {0};
    va_list ap;
    extern h_context_t HCTX;

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
    extern h_context_t HCTX;

    va_start( ap, format );
    vsprintf( message, format, ap );
    va_end(ap);

    sprintf( error, "\033[22;31mERROR : %s .\n\033[00m", message );
    yyerror(error);
    hprint_stacktrace();
    h_env_release( &HCTX, 1 );
    exit(-1);
}

void hybris_syntax_error( const char *format, ... ){
    char message[0xFF] = {0},
         error[0xFF] = {0};
    va_list ap;
    extern int yylineno;
    extern h_context_t HCTX;

    va_start( ap, format );
    vsprintf( message, format, ap );
    va_end(ap);

    sprintf( error, "\033[22;31mSyntax error on line %d : %s .\n\033[00m", yylineno, message );
    yyerror(error);
    hprint_stacktrace();
    h_env_release( &HCTX, 1 );
    exit(-1);
}

void htype_assert( Object *o, H_OBJECT_TYPE type ){
	if( o->xtype != type ){
		hybris_syntax_error( "'%s' is not a valid variable type", Object::type(o) );
	}
}

void htype_assert( Object *o, H_OBJECT_TYPE type1, H_OBJECT_TYPE type2 ){
	if( o->xtype != type1 && o->xtype != type2 ){
		hybris_syntax_error( "'%s' is not a valid variable type", Object::type(o) );
	}
}
