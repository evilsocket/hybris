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

#include "hybris.h"
#include "engine.h"

#define YY_(s) (char *)s

extern int yyparse(void);
extern int yylex(void);

Context   __context;
unsigned long t_start = 0,
              t_end   = 0;
%}

%union {
    /* basic types */
    long    integer;
    double  real;
    char    byte;
    char   *string;
    /* variable identifier */
    char   *identifier;
    /* function prototype declaration */
    function_decl_t *function;
    /* function call temp arg list */
    HNodeList *argv;
    /* not reduced node */
    Node *HNODEPTR;
};

%locations

%token <integer>    INTEGER
%token <real>       REAL;
%token <byte>       CHAR;
%token <string>     STRING;
%token <identifier> IDENT;
%token <function>   FUNCTION_PROTOTYPE;


%token EOSTMT DDOT DOT DOTE PLUS PLUSE MINUS MINUSE DIV DIVE MUL MULE MOD MODE FACT XOR XORE NOT AND ANDE OR ORE SHIFTL SHIFTLE SHIFTR SHIFTRE ASSIGN REGEX_OP
%token RANGE SUBSCRIPTADD SUBSCRIPTSET SUBSCRIPTGET WHILE DO FOR FOREACH FOREACHM OF TO IF QUESTION DOLLAR MAPS PTR OBJ
%token RETURN CALL
%nonassoc IFX
%nonassoc SBX
%nonassoc FBX
%nonassoc ELSE

%left LNOT GE LE EQ NE GREATER LESS LAND LOR
%left DOLLAR INC DEC MULE MUL DIVE DIV PLUSE PLUS MINUSE MINUS DOTE DOT
%nonassoc UMINUS

%type <HNODEPTR> statement expression statements
%type <argv>     arglist

%%

program    : body { if( __context.args.do_timing == 1 ){
                        t_end = h_uticks();
                        printf( "\033[01;33m[TIME] Elapsed %s .\n\033[00m", h_dtime( t_end - t_start ) );
                    }
                  }

body       : body statement { if( __context.args.do_timing == 1 && t_start == 0 ){
                                  t_start = h_uticks();
                              }
                              __context.engine->exec( &__context.vmem, $2 );
                              Tree::release($2);
                            }
           | /* empty */ ;

statement  : EOSTMT                                                   { $$ = Tree::addExpression( EOSTMT, 2, NULL, NULL ); }
	   	   | expression                                               { $$ = $1; }
           | expression EOSTMT                                        { $$ = $1; }
           | RETURN expression EOSTMT                                 { $$ = Tree::addExpression( RETURN, 1, $2 ); }
           /* subscript operator special cases */
		   | expression '[' ']' ASSIGN expression EOSTMT              { $$ = Tree::addExpression( SUBSCRIPTADD, 2, $1, $5 ); }
           | expression '[' expression ']' ASSIGN expression EOSTMT   { $$ = Tree::addExpression( SUBSCRIPTSET, 3, $1, $3, $6 ); }
           /* conditional and loops */
           | WHILE '(' expression ')' statement                       { $$ = Tree::addStatement( WHILE, 2, $3, $5 ); }
           | DO statement WHILE '(' expression ')' EOSTMT             { $$ = Tree::addStatement( DO, 2, $2, $5 ); }
		   | FOR '(' statement statement expression ')' statement     { $$ = Tree::addStatement( FOR, 4, $3, $4, $5, $7 ); }
		   | FOREACH '(' IDENT OF expression ')' statement            { $$ = Tree::addStatement( FOREACH, 3, Tree::addIdentifier($3), $5, $7 ); free($3); }
		   | FOREACH '(' IDENT MAPS IDENT OF expression ')' statement {
		   		$$ = Tree::addStatement( FOREACHM, 4, Tree::addIdentifier($3), Tree::addIdentifier($5), $7, $9 );
				free($3);
				free($5);
		   }
           | IF '(' expression ')' statement %prec IFX                { $$ = Tree::addStatement( IF, 2, $3, $5 ); }
           | IF '(' expression ')' statement ELSE statement           { $$ = Tree::addStatement( IF, 3, $3, $5, $7 ); }
           /* statement body */
           | '{' statements '}' { $$ = $2; }
           /* function declaration */
           | FUNCTION_PROTOTYPE '{' statements '}'                    { $$ = Tree::addFunction( $1, 1, $3 ); };

arglist    : expression MAPS arglist { $$ = $3;                 $$->push_front($1); }
           | expression ',' arglist  { $$ = $3;                 $$->push_front($1); }
           | expression              { $$ = Tree::createList(); $$->push_back($1);  }
           | /* empty */             { $$ = Tree::createList(); };

statements : /* empty */          { $$ = 0;  }
           | statement            { $$ = $1; }
           | statements statement { $$ = Tree::addExpression( EOSTMT, 2, $1, $2 ); };

expression : INTEGER                                 { $$ = Tree::addInt($1); }
           | REAL                                    { $$ = Tree::addFloat($1); }
           | CHAR                                    { $$ = Tree::addChar($1); }
           | STRING                                  { $$ = Tree::addString($1);     free($1); }
           | IDENT                                   { $$ = Tree::addIdentifier($1); free($1); }
           /* expression evaluation returns an identifier */
           | DOLLAR expression                       { $$ = Tree::addExpression( DOLLAR, 1, $2 ); }
           /* ptr/alias evaluation */
           | AND expression                          { $$ = Tree::addExpression( PTR, 1, $2 ); }
           | MUL expression                          { $$ = Tree::addExpression( OBJ, 1, $2 ); }
		   /* identifier declaration/assignation */
		   | IDENT ASSIGN expression                 { $$ = Tree::addExpression( ASSIGN, 2, Tree::addIdentifier($1), $3 ); }
           /* a single subscript could be an expression itself */
           | expression '[' expression ']' %prec SBX { $$ = Tree::addExpression( SUBSCRIPTGET, 2, $1, $3 ); }
           /* range evaluation */
           | expression DDOT expression              { $$ = Tree::addExpression( RANGE, 2, $1, $3 ); }
           /* arithmetic */
           | MINUS expression %prec UMINUS           { $$ = Tree::addExpression( UMINUS, 1, $2 ); }
           | expression DOT expression               { $$ = Tree::addExpression( DOT, 2, $1, $3 ); }
		   | expression DOTE expression              { $$ = Tree::addExpression( DOTE, 2, $1, $3 ); }
           | expression PLUS expression              { $$ = Tree::addExpression( PLUS, 2, $1, $3 ); }
		   | expression PLUSE expression             { $$ = Tree::addExpression( PLUSE, 2, $1, $3 ); }
           | expression MINUS expression             { $$ = Tree::addExpression( MINUS, 2, $1, $3 ); }
		   | expression MINUSE expression            { $$ = Tree::addExpression( MINUSE, 2, $1, $3 ); }
           | expression MUL expression               { $$ = Tree::addExpression( MUL, 2, $1, $3 ); }
		   | expression MULE expression              { $$ = Tree::addExpression( MULE, 2, $1, $3 ); }
           | expression DIV expression               { $$ = Tree::addExpression( DIV, 2, $1, $3 ); }
		   | expression DIVE expression              { $$ = Tree::addExpression( DIVE, 2, $1, $3 ); }
           | expression MOD expression               { $$ = Tree::addExpression( MOD, 2, $1, $3 ); }
           | expression INC                          { $$ = Tree::addExpression( INC, 1, $1 ); }
           | expression DEC                          { $$ = Tree::addExpression( DEC, 1, $1 ); }
           /* bitwise */
           | expression XOR expression               { $$ = Tree::addExpression( XOR, 2, $1, $3 ); }
		   | expression XORE expression              { $$ = Tree::addExpression( XORE, 2, $1, $3 ); }
           | NOT expression                          { $$ = Tree::addExpression( NOT, 1, $2 ); }
           | expression AND expression               { $$ = Tree::addExpression( AND, 2, $1, $3 ); }
		   | expression ANDE expression              { $$ = Tree::addExpression( ANDE, 2, $1, $3 ); }
           | expression OR expression                { $$ = Tree::addExpression( OR, 2, $1, $3 ); }
		   | expression ORE expression               { $$ = Tree::addExpression( ORE, 2, $1, $3 ); }
		   | expression SHIFTL expression            { $$ = Tree::addExpression( SHIFTL, 2, $1, $3 ); }
		   | expression SHIFTLE expression           { $$ = Tree::addExpression( SHIFTLE, 2, $1, $3 ); }
		   | expression SHIFTR expression            { $$ = Tree::addExpression( SHIFTR, 2, $1, $3 ); }
		   | expression SHIFTRE expression           { $$ = Tree::addExpression( SHIFTRE, 2, $1, $3 ); }
		   | expression LNOT                         { $$ = Tree::addExpression( FACT, 1, $1 ); }
           /* logic */
		   | LNOT expression                         { $$ = Tree::addExpression( LNOT, 1, $2 ); }
           | expression LESS expression              { $$ = Tree::addExpression( LESS, 2, $1, $3 ); }
           | expression GREATER expression           { $$ = Tree::addExpression( GREATER, 2, $1, $3 ); }
           | expression GE expression                { $$ = Tree::addExpression( GE,  2, $1, $3 ); }
           | expression LE expression                { $$ = Tree::addExpression( LE,  2, $1, $3 ); }
           | expression NE expression                { $$ = Tree::addExpression( NE,  2, $1, $3 ); }
           | expression EQ expression                { $$ = Tree::addExpression( EQ,  2, $1, $3 ); }
           | expression LAND expression              { $$ = Tree::addExpression( LAND,  2, $1, $3 ); }
           | expression LOR expression               { $$ = Tree::addExpression( LOR,  2, $1, $3 ); }
           /* regex specific */
           | expression REGEX_OP expression          { $$ = Tree::addExpression( REGEX_OP, 2, $1, $3 ); }
           /* function call (consider two different cases due to builtin calls */
		   | IDENT      '(' arglist ')'  %prec FBX   { $$ = Tree::addCall( $1, $3 ); }
           | expression '(' arglist ')'              { $$ = Tree::addCall( $1, $3 ); }
           /* ternary operator */
           | '(' expression '?' expression ':' expression ')' { $$ = Tree::addStatement( QUESTION, 3, $2, $4, $6 ); }
           /* group expression */
           | '(' expression ')'                      { $$ = $2; };

%%

int h_banner(){
    printf( "Hybris %s (built: %s %s)\n"
            "Released under GPL v3.0 by %s\n"
            "Compiled with :\n"
            "\tModules path      : %s\n"
            "\tInclude path      : %s\n"
            #ifdef MEM_DEBUG
            "\tMemory Debug      : ON\n"
            #endif
            #ifdef GC_SUPPORT
            "\tGarbage collector : ON\n"
            #endif
            #ifdef BOUNDS_CHECK
            "\tBoundaries check  : ON\n"
            #endif
            #ifdef PCRE_SUPPORT
            "\tPCRE              : ON\n"
            #endif
            #ifdef HTTP_SUPPORT
            "\tHTTP              : ON\n"
            #endif
            #ifdef XML_SUPPORT
            "\tXML               : ON\n"
            #endif
            #ifdef MT_SUPPORT
            "\tMulti threading   : ON\n"
            #endif
            ,
            VERSION,
            __DATE__,
            __TIME__,
            AUTHOR,
            LIBS_PATH,
            MODS_PATH );
}

int h_usage( char *argvz ){
    h_banner();
    printf( "\nUsage: %s file (--trace) (--time)\n"
            "\t-h  (--help)  : Will print this menu .\n"
            "\t-tm (--time)  : Will print execution time in micro seconds .\n"
            "\t-t  (--trace) : Will enable stack trace report on errors .\n\n", argvz );
    return 0;
}

int main( int argc, char *argv[] ){
    int i, f_offset = 0;

    for( i = 0; i < argc; ++i ){
        if( strcmp( argv[i], "--trace" ) == 0 || strcmp( argv[i], "-t" ) == 0 ){
            __context.args.stacktrace = 1;
        }
        else if( strcmp( argv[i], "--time" ) == 0 || strcmp( argv[i], "-tm" ) == 0 ){
            __context.args.do_timing  = 1;
        }
        else if( strcmp( argv[i], "--help" ) == 0 || strcmp( argv[i], "-h" ) == 0 ){
            return h_usage(argv[0]);
        }
        else if( f_offset == 0 ){
            f_offset = i;
        }
    }

    strncpy( __context.args.source, argv[f_offset], sizeof(__context.args.source) );

    if( f_offset > 0 ){
        if( h_file_exists(__context.args.source) == 0 ){
            printf( "Error :'%s' no such file or directory .\n\n", __context.args.source );
            return h_usage( argv[0] );
        }
    }

    __context.init( argc, argv );

    extern FILE *yyin;
    yyin = f_offset > 0 ? fopen( __context.args.source, "r") : stdin;

    __context.chdir();

    while( !feof(yyin) ){
        yyparse();
    }
    if( f_offset > 0 ){
        fclose(yyin);
    }

    __context.release();


    return 0;
}
