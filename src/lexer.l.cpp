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
#include <string>
#include <vector>
#include <pcre.h>

using std::vector;
using std::string;

typedef vector<string> matches_t;

#define IS_WHITESPACE(c) strchr( " \r\n\t", (c) )

#define INC_LINE_NO() __hyb_line_stack.back() = ++yylineno

void             yyerror(char *);
// handle one line comments
void             hyb_lex_skip_comment();
// handle multi line comments
void             hyb_lex_skip_line();
// handle string constants
void             hyb_lex_string( char delimiter, char *buffer );
// handle char constants
char             hyb_lex_char( char delimiter );
// extract tokens from a string give a regular expression
matches_t 		 hyb_pcre_matches( string pattern, char *subject );
// handle function prototypes declarations
function_decl_t *hyb_lex_function( char * text );
// handle method prototypes declarations
method_decl_t   *hyb_lex_method( char * text );
// handle operator overloading declaration
method_decl_t   *hyb_lex_operator( char *text );
/*
 * A stack container for files being parsed.
 */
vector<string> __hyb_file_stack;
/*
 * A stack container for correct line numbering upon inclusion.
 */
vector<int>	   __hyb_line_stack;

extern int     yylineno;
extern Context __context;

%}

%option noyywrap
%option batch

%x T_INCLUSION

exponent   [eE][-+]?[0-9]+
spaces     [ \n\t]+
identifier [a-zA-Z\_][a-zA-Z0-9\_]*
operators  "[]"|"[]="|"[]<"|".."|"+"|"+="|"-"|"-="|"/"|"/="|"*"|"*="|"%"|"%="|"++"|"--"|"^"|"^="|"~"|"&"|"&="|"|"|"|="|"<<"|"<<="|">>"|"!"|"<"|">"|">="|"<="|"=="|"!="|"&&"|"||"|"~="
%%

[ \t]+ ;
[\n]             { INC_LINE_NO(); }

include          BEGIN(T_INCLUSION);

<T_INCLUSION>[ \t]*      /* eat the whitespace */
<T_INCLUSION>[^ \t\n]+ {
    /* extract file name */
    char *ptr = yytext;
    while( *ptr != ' ' && *ptr != ';' ){
        ptr++;
    }
    *ptr = 0x00;

    string file = yytext;

    /*
	 * First of all, try to include a file in the same directory
	 * of the script.
	 */
    if( (yyin = fopen( file.c_str(), "r" )) != NULL ){
    	__hyb_file_stack.push_back(file);
    }
    /*
	 * Secondly, try adding the .hy extension.
	 */
    else if( (yyin = fopen( (file + ".hy").c_str(), "r" )) != NULL ){
    	__hyb_file_stack.push_back((file + ".hy"));
    }
    else {
    	/*
		 * Try to load from default include path.
		 * In this case replace '.' with '/' so that :
		 *
		 * include std.io.network.Socket;
		 *
		 * would include :
		 *
		 * /usr/lib/hybris/include/std/io/network/Socket.hy
		 */
    	string_replace( file, ".", "/" );

    	if( (yyin = fopen( (INC_PATH + file + ".hy").c_str(), "r" )) != NULL ){
    		__hyb_file_stack.push_back((INC_PATH + file + ".hy"));
    	}
		else{
			hyb_throw( H_ET_GENERIC, "Could not open '%s' for inclusion", yytext );
		}
    }

    __hyb_line_stack.push_back(yylineno);
    yylineno = 0;
    yypush_buffer_state( yy_create_buffer( yyin, YY_BUF_SIZE ) );

    BEGIN(INITIAL);
}
<<EOF>> {
	__hyb_file_stack.pop_back();
	__hyb_line_stack.pop_back();
	yylineno = __hyb_line_stack.back();

    yypop_buffer_state();

    if( !YY_CURRENT_BUFFER ){
        yyterminate();
    }
}

"import"[ \n\t]+({identifier}?\.?\*?)*[ \n\t]*";" {
    char *sptr = yytext + strlen( "import " );
    string module("");

    /*
     * Skip whitespaces.
     */
    while( IS_WHITESPACE(*sptr) && *sptr != ';' ){
    	sptr++;
    }

    for( ; *sptr != ';'; sptr++ ){
		 if( !IS_WHITESPACE(*sptr) ){
			 module += *sptr;
		 }
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
"next"			return T_NEXT;
"default"       return T_DEFAULT;

"new"			return T_NEW;

"struct"        return T_STRUCT;

"class"         return T_CLASS;
"extends"		return T_EXTENDS;

"public"		return T_PUBLIC;
"private"		return T_PRIVATE;
"protected"		return T_PROTECTED;

"throw"			return T_THROW;
"try"			return T_TRY;
"catch"			return T_CATCH;
"finally"		return T_FINALLY;

"return"        return T_RETURN;

"function"[ \n\t]+{identifier}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*")" {
	yylval.function   = hyb_lex_function(yytext);
	return T_FUNCTION_PROTOTYPE;
}

"method"[ \n\t]+{identifier}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*")" {
	yylval.method = hyb_lex_method(yytext);
	return T_METHOD_PROTOTYPE;
}

"operator"[ \n\t]+{operators}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*")" {
	yylval.method = hyb_lex_operator(yytext);
	return T_METHOD_PROTOTYPE;
}

"__FILE__" {
	strcpy( yylval.string, __hyb_file_stack.back().c_str() );
	return T_STRING;
}

"__LINE__" {
	yylval.integer = __hyb_line_stack.back();
	return T_INTEGER;
}

{identifier}                           { strncpy( yylval.identifier, yytext, 0xFF ); return T_IDENT; }

-?[0-9]+                               { yylval.integer = atol(yytext);         return T_INTEGER; }
-?0x[A-Fa-f0-9]+                       { yylval.integer = strtol(yytext,0,16);  return T_INTEGER; }
-?([0-9]+|([0-9]*\.[0-9]+){exponent}?) { yylval.real    = atof(yytext);         return T_REAL; }
"'"                                    { yylval.byte    = hyb_lex_char('\'');   return T_CHAR; }
"\""                                   { hyb_lex_string( '"', yylval.string );  return T_STRING; }

. { hyb_throw( H_ET_SYNTAX, "Unexpected token '%s'", yytext ); }

%%

void hyb_lex_skip_comment(){
    char c, c1;

loop:
    while ((c = yyinput()) != '*' && c != 0){
        if( c == '\n' ){
        	INC_LINE_NO();
        }
    }

    if ((c1 = yyinput()) != '/' && c != 0){
        if( c1 == '\n' ){ INC_LINE_NO(); }
        unput(c1);
        goto loop;
    }
}

void hyb_lex_skip_line(){
    char c;
    while( (c = yyinput()) != '\n' && c != EOF );
    INC_LINE_NO();
}

char hyb_lex_char( char delimiter ){
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
			case 'n'  : return '\n'; break;
			/* carriage return */
			case 'r'  : return '\r'; break;
			/* horizontal tab */
			case 't'  : return '\t'; break;
			/* vertical tab */
			case 'v'  : return '\v'; break;
			/* audible bell */
			case 'a'  : return '\a'; break;
			/* backspace */
			case 'b'  : return '\b'; break;
			/* formfeed */
			case 'f'  : return '\f'; break;
			/* ' */
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

matches_t hyb_pcre_matches( string pattern, char *subject ){
	int    		 i, ccount, rc,
				*offsets, offset = 0,
				 eoffset;
	const char  *error;
	pcre 		*compiled;
	matches_t    matches;

	compiled = __context.pcre_compile( pattern, PCRE_CASELESS|PCRE_MULTILINE, &error, &eoffset );
	rc 		 = pcre_fullinfo( compiled, 0, PCRE_INFO_CAPTURECOUNT, &ccount );

	offsets = new int[ 3 * (ccount + 1) ];

	while( (rc = pcre_exec( compiled, 0, subject, strlen(subject), offset, 0, offsets, 3 * (ccount + 1) )) > 0 ){
		const char *data;
		for( i = 1; i < rc; ++i ){
			pcre_get_substring( subject, offsets, rc, i, &data );
			matches.push_back(data);
		}
		offset = offsets[1];
	}

	delete[] offsets;

	return matches;
}

function_decl_t *hyb_lex_function( char * text ){
    function_decl_t *declaration = new function_decl_t;
    string			 identifier  = "[a-zA-Z_][a-zA-Z0-9_]*",
					 pattern     = "function[\\s]+("+identifier+")[\\s]*\\(([^\\)]*)\\)";
	matches_t 		 tokens;
	int 			 i;

	tokens = hyb_pcre_matches( pattern, text );

	strcpy( declaration->function, tokens[0].c_str() );

	pattern = "("+identifier+")";

	tokens = hyb_pcre_matches( pattern, (char *)tokens[1].c_str() );

	declaration->argc = tokens.size();
	for( i = 0; i < declaration->argc; ++i ){
		strcpy( declaration->argv[i], tokens[i].c_str() );
	}

	return declaration;
}

method_decl_t *hyb_lex_method( char * text ){
	method_decl_t   *declaration = new method_decl_t;
	string			 identifier  = "[a-zA-Z_][a-zA-Z0-9_]*",
					 pattern     = "method[\\s]+("+identifier+")[\\s]*\\(([^\\)]*)\\)";
	matches_t 		 tokens;
	int 			 i;

	tokens = hyb_pcre_matches( pattern, text );

	strcpy( declaration->method, tokens[0].c_str() );

	pattern = "("+identifier+")";

	tokens = hyb_pcre_matches( pattern, (char *)tokens[1].c_str() );

	declaration->argc = tokens.size();
	for( i = 0; i < declaration->argc; ++i ){
		strcpy( declaration->argv[i], tokens[i].c_str() );
	}

	return declaration;
}

method_decl_t *hyb_lex_operator( char * text ){
	method_decl_t   *declaration = new method_decl_t;
	string			 identifier  = "[a-zA-Z_][a-zA-Z0-9_]*",
					 operators   = "[\\[\\]=\\<\\.\\+\\-\\/\\*\\%\\^\\~\\&\\|\\>\\!]+",
					 pattern     = "operator[\\s]+("+operators+")[\\s]*\\(([^\\)]*)\\)";
	matches_t 		 tokens;
	int 			 i;

	tokens = hyb_pcre_matches( pattern, text );

	/*
	 * Mangle operator name.
	 */
	strcpy( declaration->method, ("__op@" + tokens[0]).c_str() );

	pattern = "("+identifier+")";

	tokens = hyb_pcre_matches( pattern, (char *)tokens[1].c_str() );

	declaration->argc = tokens.size();
	for( i = 0; i < declaration->argc; ++i ){
		strcpy( declaration->argv[i], tokens[i].c_str() );
	}

    return declaration;
}
