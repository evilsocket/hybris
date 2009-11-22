%{
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

#include "tree.h"
#include "common.h"
#include "parser.hpp"
#include "builtin.h"
#include <stdio.h>
#include <string.h>

void yyerror(char *);

void              h_skip_comment();
void              h_skip_line();
char             *h_handle_string( char delimiter );
char              h_handle_char( char delimiter );
function_decl_t *h_handle_function( char * text );

extern int yylineno;
extern hybris_globals_t HGLOBALS;

%}

%option noyywrap
%option batch

%x INCLUSION

exponent   [eE][-+]?[0-9]+
spaces     [ \n\t]+
identifier [a-zA-Z][a-zA-Z0-9\_]*
%%

[ \t]+ ;
[\n]             { yylineno++; }

include          BEGIN(INCLUSION);

<INCLUSION>[ \t]*      /* eat the whitespace */
<INCLUSION>[^ \t\n]+ {
    /* extract file name */
    char *ptr = yytext;
    while( *ptr != ' ' && *ptr != ';' ){
        ptr++;
    }
    *ptr = 0x00;

    if ( (yyin = fopen( yytext, "r" )) == NULL ){
        hybris_generic_error( "Could not open '%s' for inclusion", yytext );
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

"import"[ \n\t]+{identifier}[ \n\t]*";" {
    char *sptr = yytext + strlen( "import " );
    string module = string(HGLOBALS.rootpath) + "/lib/";

    while( *sptr != ';' ){
        module += *sptr;
        sptr++;
    }
    module += ".so";

    yytext = sptr;

    hmodule_load( (char *)module.c_str() );
}

"#"             { h_skip_line();    }
"/*"		    { h_skip_comment(); }
"//"            { h_skip_line();    }

";" 		    return EOSTMT;

"."             return DOT;
".="            return DOTE;

"="		        return ASSIGN;
"+"             return PLUS;
"+="            return PLUSE;
"-"             return MINUS;
"-="            return MINUSE;
"/"             return DIV;
"/="            return DIVE;
"*"             return MUL;
"*="            return MULE;
"%"             return MOD;
"%="            return MODE;

"++"            return INC;
"--"            return DEC;

"^"             return XOR;
"^="            return XORE;
"~"             return NOT;
"&"             return AND;
"&="            return ANDE;
"|"             return OR;
"|="            return ORE;
"<<"            return SHIFTL;
"<<="           return SHIFTLE;
">>"            return SHIFTR;

"!"             return LNOT;
"<"             return LESS;
">"             return GREATER;
">="            return GE;
"<="            return LE;
"=="            return EQ;
"!="            return NE;
"&&"            return LAND;
"||"            return LOR;

"~="            return REGEX_OP;

[\(|\)|\{|\}|\[|\]|\,\?\:] return yytext[0];

"$"             return DOLLAR;
"->"            return MAPS;
"while"         return WHILE;
"for"		    return FOR;
"foreach"       return FOREACH;
"of"            return OF;
"if"            return IF;
"else"          return ELSE;

"return"        return RETURN;

"function"[ \n\t]+{identifier}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*")" {
	yylval.function   = h_handle_function(yytext);
	return FUNCTION_PROTOTYPE;
}

{identifier}                           { yylval.identifier = strdup(yytext);    return IDENT; }

-?[0-9]+                               { yylval.integer = atol(yytext);         return INTEGER; }
-?([0-9]+|([0-9]*\.[0-9]+){exponent}?) { yylval.real    = atof(yytext);         return REAL; }
"'"                                    { yylval.byte    = h_handle_char('\'');  return CHAR; }
"\""                                   { yylval.string  = h_handle_string('"'); return STRING; }

. { hybris_syntax_error( "Unexpected token '%s'", yytext ); }

%%



int h_isspace(char c){
    return (strchr( " \r\n\t", c ) != NULL);
}

void h_skip_comment(){
    char c, c1;

loop:
    while ((c = yyinput()) != '*' && c != 0);

    if ((c1 = yyinput()) != '/' && c != 0){
        if( c1 == '\n' ){ yylineno++; }
        unput(c1);
        goto loop;
    }
}

void h_skip_line(){
    char c;
    while( (c = yyinput()) != '\n' && c != EOF );
    yylineno++;
}

char  h_handle_char( char delimiter ){
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

char *h_handle_string( char delimiter ){
    char buffer[ 0xFF ];
    char *ptr;
    int c, prev = 0x00;

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

    return strdup(buffer);
}

function_decl_t *h_handle_function( char * text ){
    function_decl_t *declaration = new function_decl_t;
    char *sptr = text + strlen( "function " ),
         *dptr = declaration->function,
         var[0xFF] = {0};
    int  end = 0;

    memset( declaration->function, 0x00, 0xFF );
    while( h_isspace(*sptr) ){ sptr++; };

    while( !h_isspace(*sptr) && *sptr != '(' ){
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