#ifndef _HCOMMON_H_
#	define _HCOMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "object.h"

#define HMAXARGS 200

typedef struct {
    char function[0xFF];
    int  argc;
    char argv[HMAXARGS][0xFF];
}
function_decl_t;

typedef enum {
    H_NONE,
    H_EXECUTE,
    H_COMPILE,
    H_RUN
}
H_ACTION;

typedef unsigned char byte;

typedef struct {
    byte magic[7];
    byte compressed;
}
hybris_header_t;

typedef struct {
    H_ACTION action;
    char     source[0xFF];
    char     destination[0xFF];
    char     rootpath[0xFF];
    FILE    *compiled;
    int      stacktrace;
}
hybris_globals_t;

void yyerror( char *error );
void hybris_generic_warning( const char *format, ... );
void hybris_generic_error( const char *format, ... );
void hybris_syntax_error( const char *format, ... );
void htype_assert( Object *o, H_OBJECT_TYPE type );
void htype_assert( Object *o, H_OBJECT_TYPE type1, H_OBJECT_TYPE type2 );

#endif
