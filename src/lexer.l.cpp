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
#include "parser.h"
#include "vm.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <pcre.h>

using std::vector;
using std::string;

typedef vector<string> matches_t;

#define IS_WHITESPACE(c) strchr( " \r\n\t", (c) )

#define LEX_NEXT()	   yyinput()
#define LEX_FETCH(c)   (c = LEX_NEXT())
#define LEX_UNFETCH(c) unput(c)

void             yyerror(char *);
// check if a give name is a directory or a file
bool  			 hyb_is_dir( const char *filename );
// handle one line comments
void             hyb_lex_skip_comment();
// handle multi line comments
void             hyb_lex_skip_line();
// handle string escaping
char 			*hyb_lex_parse_string( char *str );
// handle string constants
char *           hyb_lex_string( char delimiter, char *buffer );
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
/*
 * Extern VM reference.
 */
extern vm_t   *__hyb_vm;

int yyparse(void);

union hyb_token_value {
	/* base types */
	bool    boolean;
	long    integer;
	double  real;
	char    byte;
	char    string[MAX_STRING_SIZE];
	/* variable identifier */
	char    identifier[MAX_STRING_SIZE];
	/* function prototype declaration */
	function_decl_t *function;
	/* method prototype declaration */
	method_decl_t *method;
	/* node list for multiple nodes on one statement */
	llist_t *list;
	/* not reduced node */
	Node *node;
	/* access specifiers */
	access_t access;
};

#define YYSTYPE hyb_token_value

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif
/*
 * We got a pure parser here, so we have to define our
 * yylex prototype with correct yylval and yyloc types.
 */
#define YY_DECL int yylex( hyb_token_value* yylval, YYLTYPE *yylloc )
/*
 * Make sure every rule yylloc is correctly updated.
 */
#define YY_USER_ACTION yylloc->first_line = vm_get_lineno(__hyb_vm);

%}

%x T_INCLUSION

%option noyywrap
%option batch
%option stack

exponent   [eE][-+]?[0-9]+
spaces     [ \n\t]+
identifier [a-zA-Z\_][a-zA-Z0-9\_]*
operators  "[]"|"[]="|"[]<"|".."|"+"|"+="|"-"|"-="|"/"|"/="|"*"|"*="|"%"|"%="|"++"|"--"|"^"|"^="|"~"|"&"|"&="|"|"|"|="|"<<"|"<<="|">>"|"!"|"<"|">"|">="|"<="|"=="|"!="|"&&"|"||"|"~="

%%

[ \t]+ ;
[\n]             { vm_inc_lineno(__hyb_vm); }

include          BEGIN(T_INCLUSION);

<T_INCLUSION>[ \t]*      /* eat the whitespace */
<T_INCLUSION>[^ \t\n]+ {
    /* extract file name */
    char *ptr = yytext;
    while( *ptr != ' ' && *ptr != ';' ){
        ptr++;
    }
    *ptr = 0x00;

    string file 	= yytext,
		   lcl_file = file,
		   ext_file = file + ".hy",
		   std_file;

    string_replace( file, ".", "/" );
    std_file = INC_PATH + file + ".hy";

    /*
	 * First of all, try to include a file in the same directory
	 * of the script.
	 */
    if( !hyb_is_dir(lcl_file.c_str()) && (yyin = fopen( lcl_file.c_str(), "r" )) != NULL ){
    	__hyb_file_stack.push_back(lcl_file);
    }
    /*
	 * Secondly, try adding the .hy extension.
	 */
    else if( !hyb_is_dir(ext_file.c_str()) && (yyin = fopen( ext_file.c_str(), "r" )) != NULL ){
    	__hyb_file_stack.push_back(ext_file);
    }
    /*
	 * Try to load from default include path.
	 * In this case all '.' are replaced with '/' so that :
	 *
	 * include std.io.network.Socket;
	 *
	 * would include :
	 *
	 * /usr/lib/hybris/include/std/io/network/Socket.hy
	 */
    else if( !hyb_is_dir(std_file.c_str()) && (yyin = fopen( std_file.c_str(), "r" )) != NULL ){
    	__hyb_file_stack.push_back(std_file);
    }
    /*
     * Nothing found :(
     */
    else {
    	hyb_error( H_ET_GENERIC, "Could not open '%s' for inclusion", yytext );
    }

    __hyb_line_stack.push_back( vm_get_lineno(__hyb_vm) );

    vm_set_lineno( __hyb_vm, 1 );

    yypush_buffer_state( yy_create_buffer( yyin, YY_BUF_SIZE ) );

    BEGIN(INITIAL);
}
<<EOF>> {
	if( __hyb_file_stack.size() ){
		__hyb_file_stack.pop_back();
	}
	if( __hyb_line_stack.size() ){
		__hyb_line_stack.pop_back();
		vm_set_lineno( __hyb_vm, __hyb_line_stack.back() );
	}

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

    vm_load_module( __hyb_vm, (char *)module.c_str() );
}

"#"             { hyb_lex_skip_line();    }
"/*"		    { hyb_lex_skip_comment(); }
"//"            { hyb_lex_skip_line();    }

";" 		    return T_EOSTMT;

"@"				return T_VARGS;
".."            return T_RANGE;

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

"!"             return T_L_NOT;
"<"             return T_LESS;
">"             return T_GREATER;
">="            return T_GREATER_EQ;
"<="            return T_LESS_EQ;
"=="            return T_SAME;
"!="            return T_NOT_SAME;
"&&"            return T_L_AND;
"||"            return T_L_OR;

"~="            return T_REGEX_OP;

[\(|\)|\{|\}|\[|\]|\,\?\:`] return yytext[0];

"$"             return T_DOLLAR;
"."             return T_GET_MEMBER;
"->"			return T_MAPS;
"while"         return T_WHILE;
"do"            return T_DO;
"for"		    return T_FOR;
"foreach"       return T_FOREACH;
"of"            return T_OF;
"unless"		return T_UNLESS;
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

"static"		return T_STATIC;
"public"		return T_PUBLIC;
"private"		return T_PRIVATE;
"protected"		return T_PROTECTED;

"throw"			return T_THROW;
"try"			return T_TRY;
"catch"			return T_CATCH;
"finally"		return T_FINALLY;

"return"        return T_RETURN;

"function"[ \n\t]+{identifier}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*([ \n\t]*\.\.\.)?[ \n\t]*")" {
	yylval->function   = hyb_lex_function(yytext);
	return T_FUNCTION_PROTOTYPE;
}

"method"[ \n\t]+{identifier}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*([ \n\t]*\.\.\.)?[ \n\t]*")" {
	yylval->method = hyb_lex_method(yytext);
	return T_METHOD_PROTOTYPE;
}

"operator"[ \n\t]+{operators}[ \n\t]*"("([ \n\t]*{identifier}[ \n\t]*,?)*")" {
	yylval->method = hyb_lex_operator(yytext);
	return T_METHOD_PROTOTYPE;
}

"__FILE__" {
	if( __hyb_file_stack.size() ){
		strncpy( yylval->string, __hyb_file_stack.back().c_str(), MAX_STRING_SIZE );
	}
	else{
		strncpy( yylval->string,  "<unknown>", MAX_STRING_SIZE );
	}
	return T_STRING;
}

"__LINE__" {
	yylval->integer = vm_get_lineno(__hyb_vm);

	return T_INTEGER;
}

"true"|"false"						 { yylval->boolean = !strcmp( yytext, "true"); return T_BOOLEAN; }
[0-9]+                               { yylval->integer = atol(yytext);             return T_INTEGER; }
0x[A-Fa-f0-9]+                       { yylval->integer = strtol(yytext,0,16);      return T_INTEGER; }
([0-9]+|([0-9]*\.[0-9]+){exponent}?) { yylval->real    = atof(yytext);             return T_REAL; }
"'"                                  { yylval->byte    = hyb_lex_char('\'');    	return T_CHAR; }
"\""                                 { hyb_lex_string( '"', yylval->string ); 		return T_STRING; }

{identifier} {
	if( strlen(yytext) > MAX_IDENT_SIZE ){
		hyb_error( H_ET_GENERIC, "Identifier name above max size" );
	}

	strncpy( yylval->identifier, yytext, MAX_STRING_SIZE );

	return T_IDENT;
}

. { hyb_error( H_ET_SYNTAX, "Unexpected token '%s'", yytext ); }

%%

bool hyb_is_dir( const char *filename ){
	int status;
	struct stat sbuff;

	status = stat( filename, &sbuff );

	if( status < 0 || !S_ISDIR(sbuff.st_mode) ){
	    return false;
	}
	else{
		return true;
	}
}


void hyb_lex_skip_comment(){
    char c, c1;

loop:
    while( LEX_FETCH(c) != '*' && c != 0 ){
        if( c == '\n' ){
        	vm_inc_lineno(__hyb_vm);
        }
    }

    if( LEX_FETCH(c1) != '/' && c != 0){
        if( c1 == '\n' ){
        	vm_inc_lineno(__hyb_vm);
        }

        LEX_UNFETCH(c1);

        goto loop;
    }
}

void hyb_lex_skip_line(){
    char c;

    while( LEX_FETCH(c) != '\n' && c != EOF );

    vm_inc_lineno(__hyb_vm);
}

char hyb_lex_char( char delimiter ){
    char ch;

    LEX_FETCH(ch);
	if( ch != '\\' ){
		LEX_NEXT();
		return ch;
	}
	else{
		LEX_FETCH(ch);
		LEX_NEXT();
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

char *hyb_lex_parse_string( char *str ){
	char *s, *t, *end, next;

	s 	 = t = str;
	end  = s + strlen(str);

	while( s < end ){
		/*
		 * Non escaped character.
		 */
		if( *s != '\\' ) {
			*t++ = *s;
		}
		/*
		 * Escaped character.
		 */
		else{
			/*
			 * Go to the next character after the \
			 */
			if( ++s >= end ){
				*t++ = '\\';
				break;
			}

			switch( *s ){
				case 'n' : *t++ = '\n'; break;
				case 'r' : *t++ = '\r'; break;
				case 't' : *t++ = '\t'; break;
				case 'f' : *t++ = '\f'; break;
				case 'v' : *t++ = '\v'; break;
				case '"' :
				case '`' :
				case '\\': *t++ = *s; 	break;

				case 'x':
				case 'X':
					/*
					 * Handle hex characters.
					 */
					next = *( s + 1 );
					if( (next >= 'A' && next <= 'F') || (next >= 'a' && next <= 'f') || (next >= '0' && next <= '9') ){
						char s_hex[3] = {0x00};

						s_hex[0] = *(++s);
						next 	 = *( s + 1 );

						if( (next >= 'A' && next <= 'F') || (next >= 'a' && next <= 'f') || (next >= '0' && next <= '9') ){
							s_hex[1] = *(++s);
						}

						*t++ = (char)strtol( s_hex, NULL, 16 );
					}
					/*
					 * Not a hex character.
					 */
					else {
						*t++ = '\\';
						*t++ = *s;
					}
				break;

				default:
					/*
					 * Eventually handle octal characters.
					 */
					if( *s >= '0' && *s <= '7' ){
						char s_oct[4] = {0x00};

						s_oct[0] = *s;
						if( *(s + 1) >= '0' && *(s + 1) <= '7' ){
							s_oct[1] = *(++s);
							if( *(s + 1) >= '0' && *(s + 1) <= '7' ){
								s_oct[2] = *(++s);
							}
						}
						*t++ = (char)strtol( s_oct, NULL, 8 );
					}
					/*
					 * Simply an escaped character.
					 */
					else {
						*t++ = '\\';
						*t++ = *s;
					}
			}
		}
		/*
		 * Loop to the next character.
		 */
		s++;
	}
	/*
	 * Null terminate the string.
	 */
	*t = 0x00;

	return str;
}

char *hyb_lex_string( char delimiter, char *buffer ){
    char *ptr  = NULL;
    int offset = 0, c, prev = 0x00;

    memset( buffer, 0x00, MAX_STRING_SIZE );
    ptr = buffer;

	for(;;){
		/*
		 * Break on non-escaped delimiter.
		 */
		if( LEX_FETCH(c) == delimiter && prev != '\\' ){
			break;
		}
		else{
			if( ++offset >= MAX_STRING_SIZE ){
				hyb_error( H_ET_GENERIC, "String constant above max size %d", MAX_STRING_SIZE );
			}

			*ptr++ = prev = c;
		}
	}
    *ptr = 0x00;
    /*
     * Do special characters escaping and return the result.
     */
    return (buffer = hyb_lex_parse_string(buffer));
}

matches_t hyb_pcre_matches( string pattern, char *subject ){
	int    		 i, ccount, rc,
				*offsets, offset = 0,
				 eoffset;
	const char  *error;
	pcre 		*compiled;
	matches_t    matches;

	compiled = vm_pcre_compile( __hyb_vm, pattern, PCRE_CASELESS|PCRE_MULTILINE, &error, &eoffset );
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
	int 			 i, argc;

	tokens = hyb_pcre_matches( pattern, text );

	strcpy( declaration->function, tokens[0].c_str() );

	pattern = "(" + identifier + "|\\.\\.\\.)";

	tokens = hyb_pcre_matches( pattern, (char *)tokens[1].c_str() );

	declaration->argc  = tokens.size();
	declaration->vargs = false;
	argc			   = declaration->argc;
	for( i = 0; i < argc; ++i ){
		if( tokens[i] != "..." ){
			strcpy( declaration->argv[i], tokens[i].c_str() );
		}
		else{
			declaration->vargs = true;
			declaration->argc--;
		}
	}

	return declaration;
}

method_decl_t *hyb_lex_method( char * text ){
	method_decl_t   *declaration = new method_decl_t;
	string			 identifier  = "[a-zA-Z_][a-zA-Z0-9_]*",
					 pattern     = "method[\\s]+("+identifier+")[\\s]*\\(([^\\)]*)\\)";
	matches_t 		 tokens;
	int 			 i, argc;

	tokens = hyb_pcre_matches( pattern, text );

	strcpy( declaration->method, tokens[0].c_str() );

	pattern = "(" + identifier + "|\\.\\.\\.)";

	tokens = hyb_pcre_matches( pattern, (char *)tokens[1].c_str() );

	declaration->argc  = tokens.size();
	declaration->vargs = false;
	argc			   = declaration->argc;
	for( i = 0; i < declaration->argc; ++i ){
		if( tokens[i] != "..." ){
			strcpy( declaration->argv[i], tokens[i].c_str() );
		}
		else{
			declaration->vargs = true;
			declaration->argc--;
		}
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
	strcpy( declaration->method, tokens[0].c_str() );

	pattern = "("+identifier+")";

	tokens = hyb_pcre_matches( pattern, (char *)tokens[1].c_str() );

	declaration->argc = tokens.size();
	for( i = 0; i < declaration->argc; ++i ){
		strcpy( declaration->argv[i], tokens[i].c_str() );
	}

    return declaration;
}

void hyb_parse_string( const char *str ){
	YY_BUFFER_STATE prev, current;
	int				state;
	int				lineno;
	FILE		   *prev_yyin;
	/*
	 * Save buffer value, lex state, input file pointer
	 * and line number.
	 */
	prev      = YY_CURRENT_BUFFER_LVALUE;
	state     = YYSTATE;
	prev_yyin = yyin;
	lineno    = vm_get_lineno(__hyb_vm);
	/*
	 * yy_scan_string will call yy_switch_to_buffer with
	 * the newly created buffer from str.
	 */
	yy_scan_string(str);
	/*
	 * Reset lex state to initial.
	 */
	BEGIN(INITIAL);

	vm_set_lineno( __hyb_vm, 1 );
	/*
	 * Parse the str, yyparse will call yylex.
	 */
	yyparse();
	/*
	 * Get current buffer, and switch to the previous
	 * one.
	 */
	current = YY_CURRENT_BUFFER;
	if( prev ){
		yy_switch_to_buffer( prev );
	}
	/*
	 * We don't need this buffer anymore, delete it.
	 */
	yy_delete_buffer(current);
	/*
	 * Restore lex state, line number and input file pointer.
	 */
	BEGIN(state);

	vm_set_lineno( __hyb_vm, lineno );

	yyin = prev_yyin;
}
