#include "common.h"
#include "vmem.h"

extern void h_env_release( int onerror );
extern vector<string> HSTACKTRACE;
extern hybris_globals_t     HGLOBALS;

void hprint_stacktrace(){
    if( HGLOBALS.stacktrace && HSTACKTRACE.size() ){
        int tail = HSTACKTRACE.size() - 1;
        printf( "\nSTACK TRACE :\n\n" );
        for( int i = tail; i >= 0; i-- ){
            printf( "\t%.3d : %s\n", i, HSTACKTRACE[i].c_str() );
        }
        printf( "\n" );
    }
}

void yyerror( char *error ){
    fflush(stderr);
	if( error[strlen(error) - 1] == '\n' ){
    	fprintf( stderr, "%s", error );
	}
	else{
		extern int yylineno;
		fprintf( stderr, "Line %d : %s .\n", yylineno, error );
		hprint_stacktrace();
		h_env_release(1);
    	exit(-1);
	}
}

void hybris_generic_error( const char *format, ... ){
    char message[0xFF] = {0},
            error[0xFF] = {0};
    va_list ap;
    extern vmem_t HVM;

    va_start( ap, format );
    vsprintf( message, format, ap );
    va_end(ap);

    sprintf( error, "Error : %s .\n", message );
    yyerror(error);
    hprint_stacktrace();
    h_env_release(1);
    exit(-1);
}

void hybris_syntax_error( const char *format, ... ){
    char message[0xFF] = {0},
            error[0xFF] = {0};
    va_list ap;
    extern int yylineno;
    extern vmem_t HVM;

    va_start( ap, format );
    vsprintf( message, format, ap );
    va_end(ap);

    sprintf( error, "Syntax error on line %d : %s .\n", yylineno, message );
    yyerror(error);
    hprint_stacktrace();
    h_env_release(1);
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
