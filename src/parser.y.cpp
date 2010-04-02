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
    /* base types */
    long    integer;
    double  real;
    char    byte;
    char    string[0xFF];
    /* variable identifier */
    char    identifier[0xFF];
    /* function prototype declaration */
    function_decl_t *function;
    /* function call temp arg list */
    NodeList *argv;
    /* not reduced node */
    Node *HNODEPTR;
};

%locations

%token <integer>    T_INTEGER
%token <real>       T_REAL;
%token <byte>       T_CHAR;
%token <string>     T_STRING;
%token <identifier> T_IDENT;
%token <function>   T_FUNCTION_PROTOTYPE;

%token T_EOSTMT
%token T_DDOT
%token T_DOT
%token T_DOTE
%token T_PLUS
%token T_PLUSE
%token T_MINUS
%token T_MINUSE
%token T_DIV
%token T_DIVE
%token T_MUL
%token T_MULE
%token T_MOD
%token T_MODE
%token T_FACT
%token T_XOR
%token T_XORE
%token T_NOT
%token T_AND
%token T_ANDE
%token T_OR
%token T_ORE
%token T_SHIFTL
%token T_SHIFTLE
%token T_SHIFTR
%token T_SHIFTRE
%token T_ASSIGN
%token T_REGEX_OP
%token T_RANGE
%token T_SUBSCRIPTADD
%token T_SUBSCRIPTSET
%token T_SUBSCRIPTGET
%token T_WHILE
%token T_DO
%token T_FOR
%token T_FOREACH
%token T_FOREACHM
%token T_OF
%token T_TO
%token T_IF
%token T_SWITCH
%token T_CASE
%token T_BREAK
%token T_DEFAULT
%token T_QUESTION
%token T_DOLLAR
%token T_MAPS
%token T_PTR
%token T_OBJ
%token T_RETURN
%token T_CALL
%token T_STRUCT

%nonassoc T_IF_END
%nonassoc T_SUBSCRIPT_END
%nonassoc T_SWITCH_END
%nonassoc T_CALL_END
%nonassoc T_ELSE
%nonassoc T_UMINUS
%nonassoc T_DOT_END

%left T_LNOT T_GE T_LE T_EQ T_NE T_GREATER T_LESS T_LAND T_LOR
%left T_DOLLAR T_INC T_DEC T_MULE T_MUL T_DIVE T_DIV T_PLUSE T_PLUS T_MINUSE T_MINUS T_DOTE T_DOT

%type <HNODEPTR> statement expression statements
%type <argv>     T_ARGV_LIST
%type <argv>     T_CASE_LIST
%type <argv>     T_ATTR_LIST
%type <argv>     T_IDENT_LIST
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
                              delete $2;
                            }
           | /* empty */ ;


T_ARGV_LIST  : expression T_MAPS T_ARGV_LIST { $$ = $3;             $$->head($1); }
             | expression ',' T_ARGV_LIST    { $$ = $3;             $$->head($1); }
             | expression                    { $$ = new NodeList(); $$->tail($1); }
             | /* empty */                   { $$ = new NodeList(); };

T_CASE_LIST  : T_CASE expression ':' statements T_BREAK T_EOSTMT T_CASE_LIST { $$ = $7;             $$->head( $2, $4 ); }
             | T_CASE expression ':' statements T_BREAK T_EOSTMT             { $$ = new NodeList(); $$->tail( $2, $4 ); }
             | /* empty */                                                   { $$ = new NodeList(); };

T_ATTR_LIST  : T_IDENT ',' T_ATTR_LIST      { $$ = $3;             $$->head( new IdentifierNode($1) ); }
             | T_IDENT T_EOSTMT T_ATTR_LIST { $$ = $3;             $$->head( new IdentifierNode($1) ); }
             | /* empty */                  { $$ = new NodeList(); };

T_IDENT_LIST : T_IDENT T_MAPS T_IDENT_LIST { $$ = $3;             $$->head( new IdentifierNode($1) ); }
             | T_IDENT T_MAPS T_IDENT      { $$ = new NodeList(); $$->tail( new IdentifierNode($1), new IdentifierNode($3) ); }

statement  : T_EOSTMT                                                   { $$ = new ExpressionNode( T_EOSTMT, 2, NULL, NULL ); }
	   	   | expression                                                 { $$ = $1; }
           | expression T_EOSTMT                                        { $$ = $1; }
           | T_RETURN expression T_EOSTMT                               { $$ = new ExpressionNode( T_RETURN, 1, $2 ); }
           /* subscript operator special cases */
		   | expression '[' ']' T_ASSIGN expression T_EOSTMT            { $$ = new ExpressionNode( T_SUBSCRIPTADD, 2, $1, $5 ); }
           | expression '[' expression ']' T_ASSIGN expression T_EOSTMT { $$ = new ExpressionNode( T_SUBSCRIPTSET, 3, $1, $3, $6 ); }
           /* conditional and loops */
           | T_WHILE '(' expression ')' statement                       { $$ = new StatementNode( T_WHILE, 2, $3, $5 ); }
           | T_DO statement T_WHILE '(' expression ')' T_EOSTMT         { $$ = new StatementNode( T_DO, 2, $2, $5 ); }
		   | T_FOR '(' statement statement expression ')' statement     { $$ = new StatementNode( T_FOR, 4, $3, $4, $5, $7 ); }
		   | T_FOREACH '(' T_IDENT T_OF expression ')' statement        { $$ = new StatementNode( T_FOREACH, 3, new IdentifierNode($3), $5, $7 ); }
		   | T_FOREACH '(' T_IDENT T_MAPS T_IDENT T_OF expression ')' statement {
		   		$$ = new StatementNode( T_FOREACHM, 4, new IdentifierNode($3), new IdentifierNode($5), $7, $9 );
		   }
           | T_IF '(' expression ')' statement %prec T_IF_END           { $$ = new StatementNode( T_IF, 2, $3, $5 ); }
           | T_IF '(' expression ')' statement T_ELSE statement         { $$ = new StatementNode( T_IF, 3, $3, $5, $7 ); }
           | T_SWITCH '(' expression ')' '{'
                T_CASE_LIST
            '}' %prec T_SWITCH_END                                      { $$ = new StatementNode( T_SWITCH, $3, $6 ); }
           | T_SWITCH '(' expression ')' '{'
                T_CASE_LIST
                T_DEFAULT ':' statements
            '}'                                                         { $$ = new StatementNode( T_SWITCH, $3, $6, $9 ); }
           /* statement body */
           | '{' statements '}' { $$ = $2; }
           /* function declaration */
           | T_FUNCTION_PROTOTYPE '{' statements '}'                    { $$ = new FunctionNode( $1, 1, $3 ); }
           /* structure declaration */
           | T_STRUCT T_IDENT '{' T_ATTR_LIST '}'                       { $$ = new StructureNode( $2, $4 ); };

statements : /* empty */          { $$ = 0;  }
           | statement            { $$ = $1; }
           | statements statement { $$ = new ExpressionNode( T_EOSTMT, 2, $1, $2 ); };

expression : T_INTEGER                                 { $$ = new ConstantNode($1); }
           | T_REAL                                    { $$ = new ConstantNode($1); }
           | T_CHAR                                    { $$ = new ConstantNode($1); }
           | T_STRING                                  { $$ = new ConstantNode($1); }

           | T_IDENT                                   { $$ = new IdentifierNode($1); }
           | T_IDENT_LIST                              { $$ = new AttributeNode($1);  }
           /* expression evaluation returns an identifier */
           | T_DOLLAR expression                       { $$ = new ExpressionNode( T_DOLLAR, 1, $2 ); }
           /* ptr/alias evaluation */
           | T_AND expression                          { $$ = new ExpressionNode( T_PTR, 1, $2 ); }
           | T_MUL expression                          { $$ = new ExpressionNode( T_OBJ, 1, $2 ); }

           | T_IDENT_LIST T_ASSIGN expression          { $$ = new ExpressionNode( T_ASSIGN, 2, new AttributeNode($1), $3 ); }

		   /* identifier declaration/assignation */
		   | T_IDENT T_ASSIGN expression               { $$ = new ExpressionNode( T_ASSIGN, 2, new IdentifierNode($1), $3 ); }
           /* a single subscript could be an expression itself */
           | expression '[' expression ']' %prec T_SUBSCRIPT_END { $$ = new ExpressionNode( T_SUBSCRIPTGET, 2, $1, $3 ); }
           /* range evaluation */
           | expression T_DDOT expression              { $$ = new ExpressionNode( T_RANGE, 2, $1, $3 ); }
           /* arithmetic & misc operators */
           | T_MINUS expression %prec T_UMINUS         { $$ = new ExpressionNode( T_UMINUS, 1, $2 ); }
           | expression T_DOT expression               { $$ = new ExpressionNode( T_DOT, 2, $1, $3 ); }
		   | expression T_DOTE expression              { $$ = new ExpressionNode( T_DOTE, 2, $1, $3 ); }
           | expression T_PLUS expression              { $$ = new ExpressionNode( T_PLUS, 2, $1, $3 ); }
		   | expression T_PLUSE expression             { $$ = new ExpressionNode( T_PLUSE, 2, $1, $3 ); }
           | expression T_MINUS expression             { $$ = new ExpressionNode( T_MINUS, 2, $1, $3 ); }
		   | expression T_MINUSE expression            { $$ = new ExpressionNode( T_MINUSE, 2, $1, $3 ); }
           | expression T_MUL expression               { $$ = new ExpressionNode( T_MUL, 2, $1, $3 ); }
		   | expression T_MULE expression              { $$ = new ExpressionNode( T_MULE, 2, $1, $3 ); }
           | expression T_DIV expression               { $$ = new ExpressionNode( T_DIV, 2, $1, $3 ); }
		   | expression T_DIVE expression              { $$ = new ExpressionNode( T_DIVE, 2, $1, $3 ); }
           | expression T_MOD expression               { $$ = new ExpressionNode( T_MOD, 2, $1, $3 ); }
           | expression T_INC                          { $$ = new ExpressionNode( T_INC, 1, $1 ); }
           | expression T_DEC                          { $$ = new ExpressionNode( T_DEC, 1, $1 ); }
           /* bitwise */
           | expression T_XOR expression               { $$ = new ExpressionNode( T_XOR, 2, $1, $3 ); }
		   | expression T_XORE expression              { $$ = new ExpressionNode( T_XORE, 2, $1, $3 ); }
           | T_NOT expression                          { $$ = new ExpressionNode( T_NOT, 1, $2 ); }
           | expression T_AND expression               { $$ = new ExpressionNode( T_AND, 2, $1, $3 ); }
		   | expression T_ANDE expression              { $$ = new ExpressionNode( T_ANDE, 2, $1, $3 ); }
           | expression T_OR expression                { $$ = new ExpressionNode( T_OR, 2, $1, $3 ); }
		   | expression T_ORE expression               { $$ = new ExpressionNode( T_ORE, 2, $1, $3 ); }
		   | expression T_SHIFTL expression            { $$ = new ExpressionNode( T_SHIFTL, 2, $1, $3 ); }
		   | expression T_SHIFTLE expression           { $$ = new ExpressionNode( T_SHIFTLE, 2, $1, $3 ); }
		   | expression T_SHIFTR expression            { $$ = new ExpressionNode( T_SHIFTR, 2, $1, $3 ); }
		   | expression T_SHIFTRE expression           { $$ = new ExpressionNode( T_SHIFTRE, 2, $1, $3 ); }
		   | expression T_LNOT                         { $$ = new ExpressionNode( T_FACT, 1, $1 ); }
           /* logic */
		   | T_LNOT expression                         { $$ = new ExpressionNode( T_LNOT, 1, $2 ); }
           | expression T_LESS expression              { $$ = new ExpressionNode( T_LESS, 2, $1, $3 ); }
           | expression T_GREATER expression           { $$ = new ExpressionNode( T_GREATER, 2, $1, $3 ); }
           | expression T_GE expression                { $$ = new ExpressionNode( T_GE,  2, $1, $3 ); }
           | expression T_LE expression                { $$ = new ExpressionNode( T_LE,  2, $1, $3 ); }
           | expression T_NE expression                { $$ = new ExpressionNode( T_NE,  2, $1, $3 ); }
           | expression T_EQ expression                { $$ = new ExpressionNode( T_EQ,  2, $1, $3 ); }
           | expression T_LAND expression              { $$ = new ExpressionNode( T_LAND,  2, $1, $3 ); }
           | expression T_LOR expression               { $$ = new ExpressionNode( T_LOR,  2, $1, $3 ); }
           /* regex specific */
           | expression T_REGEX_OP expression          { $$ = new ExpressionNode( T_REGEX_OP, 2, $1, $3 ); }
           /* function call (consider two different cases due to builtin calls */
		   | T_IDENT    '(' T_ARGV_LIST ')'  %prec T_CALL_END { $$ = new CallNode( $1, $3 ); }
           | expression '(' T_ARGV_LIST ')'                   { $$ = new CallNode( $1, $3 ); }
           /* ternary operator */
           | '(' expression '?' expression ':' expression ')' { $$ = new StatementNode( T_QUESTION, 3, $2, $4, $6 ); }
           /* group expression */
           | '(' expression ')'                               { $$ = $2; };

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

    if( f_offset > 0 ){
        strncpy( __context.args.source, argv[f_offset], sizeof(__context.args.source) );
        if( h_file_exists(__context.args.source) == 0 ){
            printf( "Error :'%s' no such file or directory .\n\n", __context.args.source );
            return h_usage( argv[0] );
        }
    }

    __context.init( argc, argv );

    extern FILE *yyin;
    yyin = __context.openFile();

    while( !feof(yyin) ){
        yyparse();
    }

    __context.closeFile();
    __context.release();


    return 0;
}
