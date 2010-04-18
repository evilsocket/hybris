%{
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

#include "node.h"
#include "common.h"
#include "parser.hpp"
#include "context.h"
#include <stdio.h>
#include <string.h>

void             yyerror(char *);
// handle one line comments
void             hyb_lex_skip_comment();
// handle multi line comments
void             hyb_lex_skip_line();
// handle string constants
void             hyb_lex_string( char delimiter, char *buffer );
// handle char constants
char             hyb_lex_char( char delimiter );
// handle function prototypes declarations
function_decl_t *hyb_lex_function( char * text );
// handle method prototypes declarations
method_decl_t   *hyb_lex_method( char * text );

extern int     yylineno;
extern Context __context;

%}

%option noyywrap
%option batch

%x T_INCLUSION

exponent   [eE][-+]?[0-9]+
spaces     [ \n\t]+
identifier [a-zA-Z\_][a-zA-Z0-9\_]*
%%

[ \t]+ ;
[\n]             { yylineno++; }

include          BEGIN(T_INCLUSION);

<T_INCLUSION>[ \t]*      /* eat the whitespace */
<T_INCLUSION>[^ \t\n]+ {
    /* extract file name */
    char *ptr = yytext;
    while( *ptr != ' ' && *ptr != ';' ){
        ptr++;
    }
    *ptr = 0x00;

    string mod = yytext;
    if( (yyin = fopen( mod.c_str(), "r" )) == NULL ){
        /* attempt to load from default include path */
        if( (yyin = fopen( (INC_PATH + mod).c_str(), "r" )) == NULL ){
            hyb_throw( H_ET_GENERIC, "Could not open '%s' for inclusion", yytext );
        }
    }

    yypush_buffer_state( yy_create_buffer( yyin, YY_BUF_SIZE ) );

    BEGIN(INITIAL);
}
<<EOF>> {
    yypop_buffer_state();

    if( !YY_CURRENT_BUFFER ){
        yyterminate();
    }
}

"import"[ \n\t]+({identifier}?\.?\*?)*[ \n\t]*";" {
    char *sptr;
    string module("");

    for( sptr = yytext + strlen( "import " ); *sptr != ';'; sptr++ ){
         module += *sptr;
    }

    yytext = sptr;

    __context.loadModule( (char *)module.c_str() );
}

"#"             { hyb_lex_skip_line();    }
"/*"		    { hyb_lex_skip_comment(); }
"//"            { hyb_lex_skip_line();    }

";" 		    return T_EOSTMT;

".."            return T_DDOT;
"."             return T_DOT;
".="            return T_DOTE;

"="		        return T_ASSIGN;
"+"             return T_PLUS;
"+="            return T_PLUSE;
"-"             return T_MINUS;
"-="            return T_MINUSE;
"/"             return T_DIV;
"/="            return T_DIVE;
"*"             return T_MUL;
"*="            return T_MULE;
"%"             return T_MOD;
"%="            return T_MODE;

"++"            return T_INC;
"--"            return T_DEC;

"^"             return T_XOR;
"^="            return T_XORE;
"~"             return T_NOT;
"&"             return T_AND;
"&="            return T_ANDE;
"|"             return T_OR;
"|="            return T_ORE;
"<<"            return T_SHIFTL;
"<<="           return T_SHIFTLE;
">>"            return T_SHIFTR;

"!"             return T_LNOT;
"<"             return T_LESS;
">"             return T_GREATER;
">="            return T_GE;
"<="            return T_LE;
"=="            return T_EQ;
"!="            return T_NE;
"&&"            return T_LAND;
"||"            return T_LOR;

"~="            return T_REGEX_OP;

[\(|\)|\{|\}|\[|\]|\,\?\:] return yytext[0];

"$"             return T_DOLLAR;
"->"            return T_MAPS;
"while"         return T_WHILE;
"do"            return T_DO;
"for"		    return T_FOR;
"foreach"       return T_FOREACH;
"of"            return T_OF;
"if"            return T_IF;
"else"          return T_ELSE;

"switch"        return T_SWITCH;
"case"          return T_CASE;
"break"         return T_BREAK;
"default"       return T_DEFAULT;

"new"			return T_NEW;
"struct"        return T_STRUCT;
"class"         return T_CLASS;

"return"        return T_RETURN;

"extends"		return T_EXTENDS;

"private"       return T_PRIVATE;
"protected"     return T_PROTECTED;
"public"        return T_PUBLIC;

"function"[ \n\t]+{identifier}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*")" {
	yylval.function   = hyb_lex_function(yytext);
	return T_FUNCTION_PROTOTYPE;
}

"method"[ \n\t]+{identifier}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*")" {
	yylval.method   = hyb_lex_method(yytext);
	return T_METHOD_PROTOTYPE;
}

{identifier}                           { strncpy( yylval.identifier, yytext, 0xFF ); return T_IDENT; }

-?[0-9]+                               { yylval.integer = atol(yytext);         return T_INTEGER; }
-?0x[A-Fa-f0-9]+                       { yylval.integer = strtol(yytext,0,16);  return T_INTEGER; }
-?([0-9]+|([0-9]*\.[0-9]+){exponent}?) { yylval.real    = atof(yytext);         return T_REAL; }
"'"                                    { yylval.byte    = hyb_lex_char('\'');   return T_CHAR; }
"\""                                   { hyb_lex_string( '"', yylval.string );  return T_STRING; }

. { hyb_throw( H_ET_SYNTAX, "Unexpected token '%s'", yytext ); }

%%



int hyb_lex_isspace(char c){
    return (strchr( " \r\n\t", c ) != NULL);
}

void hyb_lex_skip_comment(){
    char c, c1;

loop:
    while ((c = yyinput()) != '*' && c != 0){
        if( c == '\n' ){
            yylineno++;
        }
    }

    if ((c1 = yyinput()) != '/' && c != 0){
        if( c1 == '\n' ){ yylineno++; }
        unput(c1);
        goto loop;
    }
}

void hyb_lex_skip_line(){
    char c;
    while( (c = yyinput()) != '\n' && c != EOF );
    yylineno++;
}

char  hyb_lex_char( char delimiter ){
    char ch;

	ch = yyinput();
	if( ch != '\\' ){
		yyinput();
		return ch;
	}
	else{
		ch = yyinput();
		yyinput();
		switch(ch){
			/* newline */
			case 'n' : return '\n'; break;
			/* carriage return */
			case 'r' : return '\r'; break;
			/* horizontal tab */
			case 't' : return '\t'; break;
			/* vertical tab */
			case 'v' : return '\v'; break;
			/* audible bell */
			case 'a' : return '\a'; break;
			/* backspace */
			case 'b' : return '\b'; break;
			/* formfeed */
			case 'f' : return '\f'; break;

			case '\'' : return '\''; break;
		}
	}
}

void hyb_lex_string( char delimiter, char *buffer ){
    char *ptr;
    int c, prev = 0x00;

    memset( buffer, 0x00, 0xFF );

    ptr = buffer;

	for(;;){
		/* break on non-escaped delimiter */
		if( (c = yyinput()) == delimiter && prev != '\\' ){
			break;
		}
		else{
			*ptr++ = prev = c;
		}
	}
    *ptr = 0x00;
}

function_decl_t *hyb_lex_function( char * text ){
    function_decl_t *declaration = new function_decl_t;
    char *sptr = text + strlen( "function " ),
         *dptr = declaration->function,
         var[0xFF] = {0};
    int  end = 0;

    memset( declaration->function, 0x00, 0xFF );
    while( hyb_lex_isspace(*sptr) ){ sptr++; };

    while( !hyb_lex_isspace(*sptr) && *sptr != '(' ){
        *dptr = *sptr;
        sptr++;
        dptr++;
    }

    declaration->argc = 0;
    sptr++;
    dptr = var;
    while( end == 0 ){
        switch( *sptr ){
            /* skip whitespaces */
            case '\r' :
            case '\n' :
            case '\t' :
            case ' '  : break;
            /* end of variable name declaration */
            case ',' :
                strcpy( declaration->argv[declaration->argc], var );
                memset( var, 0x00, 0xFF );
                declaration->argc++;
                dptr = var;
                break;
                /* end of function declaration, end last parameter without comma */
            case ')' :
                if( var[0] != 0x00 ){
                    strcpy( declaration->argv[declaration->argc], var );
                    memset( var, 0x00, 0xFF );
                    declaration->argc++;
                }
                end = 1;
                break;
                /* still in variable name declaration */
            default :
                *dptr = *sptr;
                dptr++;
        }
        sptr++;
    }

    return declaration;
}

method_decl_t *hyb_lex_method( char * text ){
	method_decl_t *declaration = new method_decl_t;
    char *sptr = text + strlen( "method " ),
         *dptr = declaration->method,
         var[0xFF] = {0};
    int  end = 0;

    memset( declaration->method, 0x00, 0xFF );
    while( hyb_lex_isspace(*sptr) ){ sptr++; };

    while( !hyb_lex_isspace(*sptr) && *sptr != '(' ){
        *dptr = *sptr;
        sptr++;
        dptr++;
    }

    declaration->argc = 0;
    sptr++;
    dptr = var;
    while( end == 0 ){
        switch( *sptr ){
            /* skip whitespaces */
            case '\r' :
            case '\n' :
            case '\t' :
            case ' '  : break;
            /* end of variable name declaration */
            case ',' :
                strcpy( declaration->argv[declaration->argc], var );
                memset( var, 0x00, 0xFF );
                declaration->argc++;
                dptr = var;
                break;
                /* end of function declaration, end last parameter without comma */
            case ')' :
                if( var[0] != 0x00 ){
                    strcpy( declaration->argv[declaration->argc], var );
                    memset( var, 0x00, 0xFF );
                    declaration->argc++;
                }
                end = 1;
                break;
                /* still in variable name declaration */
            default :
                *dptr = *sptr;
                dptr++;
        }
        sptr++;
    }

    return declaration;
}
