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

#include "hybris.h"
#include "cmdline.h"

#define YY_(s) (char *)s

extern int yyparse(void);
extern int yylex(void);

vector<string>   HSTACKTRACE;
vmem_t           HVM;
#ifdef GC_SUPPORT
vgarbage_t       HVG;
#endif
vcode_t          HVC;
hybris_globals_t HGLOBALS;

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

program    : body { };

body       : body statement { switch( HGLOBALS.action ){
                                case H_EXECUTE : htree_execute( &HVM, $2 );              break;
                                /* case H_COMPILE : htree_compile( $2, HGLOBALS.compiled ); break; */
                                default :
                                    hybris_generic_error( "action not yet implemented" );
                              }

                              Tree::release($2);
                            }
           | /* empty */ ;

statement  : EOSTMT                                                   { $$ = Tree::addOperator( EOSTMT, 2, NULL, NULL ); }
	   	   | expression                                               { $$ = $1; }
           | expression EOSTMT                                        { $$ = $1; }
           | RETURN expression EOSTMT                                 { $$ = Tree::addOperator( RETURN, 1, $2 ); }
           /* subscript operator special cases */
		   | expression '[' ']' ASSIGN expression EOSTMT              { $$ = Tree::addOperator( SUBSCRIPTADD, 2, $1, $5 ); }
           | expression '[' expression ']' ASSIGN expression EOSTMT   { $$ = Tree::addOperator( SUBSCRIPTSET, 3, $1, $3, $6 ); }
           /* conditional and loops */
           | WHILE '(' expression ')' statement                       { $$ = Tree::addOperator( WHILE, 2, $3, $5 ); }
           | DO statement WHILE '(' expression ')' EOSTMT             { $$ = Tree::addOperator( DO, 2, $2, $5 ); }
		   | FOR '(' statement statement expression ')' statement     { $$ = Tree::addOperator( FOR, 4, $3, $4, $5, $7 ); }
		   | FOREACH '(' IDENT OF expression ')' statement            { $$ = Tree::addOperator( FOREACH, 3, Tree::addIdentifier($3), $5, $7 ); free($3); }
		   | FOREACH '(' IDENT MAPS IDENT OF expression ')' statement {
		   		$$ = Tree::addOperator( FOREACHM, 4, Tree::addIdentifier($3), Tree::addIdentifier($5), $7, $9 );
				free($3);
				free($5);
		   }
           | IF '(' expression ')' statement %prec IFX                { $$ = Tree::addOperator( IF, 2, $3, $5 ); }
           | IF '(' expression ')' statement ELSE statement           { $$ = Tree::addOperator( IF, 3, $3, $5, $7 ); }
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
           | statements statement { $$ = Tree::addOperator( EOSTMT, 2, $1, $2 ); };

expression : INTEGER                                 { $$ = Tree::addInt($1); }
           | REAL                                    { $$ = Tree::addFloat($1); }
           | CHAR                                    { $$ = Tree::addChar($1); }
           | STRING                                  { $$ = Tree::addString($1); free($1); }
           | IDENT                                   { $$ = Tree::addIdentifier($1); free($1); }
           /* expression evaluation returns an identifier */
           | DOLLAR expression                       { $$ = Tree::addOperator( DOLLAR, 1, $2 ); }
           /* ptr/alias evaluation */
           | AND expression                          { $$ = Tree::addOperator( PTR, 1, $2 ); }
           | MUL expression                          { $$ = Tree::addOperator( OBJ, 1, $2 ); }
		   /* identifier declaration/assignation */
		   | IDENT ASSIGN expression                 { $$ = Tree::addOperator( ASSIGN, 2, Tree::addIdentifier($1), $3 ); }
           /* a single subscript could be an expression itself */
           | expression '[' expression ']' %prec SBX { $$ = Tree::addOperator( SUBSCRIPTGET, 2, $1, $3 ); }
           /* range evaluation */
           | expression DDOT expression              { $$ = Tree::addOperator( RANGE, 2, $1, $3 ); }
           /* arithmetic */
           | MINUS expression %prec UMINUS           { $$ = Tree::addOperator( UMINUS, 1, $2 ); }
           | expression DOT expression               { $$ = Tree::addOperator( DOT, 2, $1, $3 ); }
		   | expression DOTE expression              { $$ = Tree::addOperator( DOTE, 2, $1, $3 ); }
           | expression PLUS expression              { $$ = Tree::addOperator( PLUS, 2, $1, $3 ); }
		   | expression PLUSE expression             { $$ = Tree::addOperator( PLUSE, 2, $1, $3 ); }
           | expression MINUS expression             { $$ = Tree::addOperator( MINUS, 2, $1, $3 ); }
		   | expression MINUSE expression            { $$ = Tree::addOperator( MINUSE, 2, $1, $3 ); }
           | expression MUL expression               { $$ = Tree::addOperator( MUL, 2, $1, $3 ); }
		   | expression MULE expression              { $$ = Tree::addOperator( MULE, 2, $1, $3 ); }
           | expression DIV expression               { $$ = Tree::addOperator( DIV, 2, $1, $3 ); }
		   | expression DIVE expression              { $$ = Tree::addOperator( DIVE, 2, $1, $3 ); }
           | expression MOD expression               { $$ = Tree::addOperator( MOD, 2, $1, $3 ); }
           | expression INC                          { $$ = Tree::addOperator( INC, 1, $1 ); }
           | expression DEC                          { $$ = Tree::addOperator( DEC, 1, $1 ); }
           /* bitwise */
           | expression XOR expression               { $$ = Tree::addOperator( XOR, 2, $1, $3 ); }
		   | expression XORE expression              { $$ = Tree::addOperator( XORE, 2, $1, $3 ); }
           | NOT expression                          { $$ = Tree::addOperator( NOT, 1, $2 ); }
           | expression AND expression               { $$ = Tree::addOperator( AND, 2, $1, $3 ); }
		   | expression ANDE expression              { $$ = Tree::addOperator( ANDE, 2, $1, $3 ); }
           | expression OR expression                { $$ = Tree::addOperator( OR, 2, $1, $3 ); }
		   | expression ORE expression               { $$ = Tree::addOperator( ORE, 2, $1, $3 ); }
		   | expression SHIFTL expression            { $$ = Tree::addOperator( SHIFTL, 2, $1, $3 ); }
		   | expression SHIFTLE expression           { $$ = Tree::addOperator( SHIFTLE, 2, $1, $3 ); }
		   | expression SHIFTR expression            { $$ = Tree::addOperator( SHIFTR, 2, $1, $3 ); }
		   | expression SHIFTRE expression           { $$ = Tree::addOperator( SHIFTRE, 2, $1, $3 ); }
		   | expression LNOT                         { $$ = Tree::addOperator( FACT, 1, $1 ); }
           /* logic */
		   | LNOT expression                         { $$ = Tree::addOperator( LNOT, 1, $2 ); }
           | expression LESS expression              { $$ = Tree::addOperator( LESS, 2, $1, $3 ); }
           | expression GREATER expression           { $$ = Tree::addOperator( GREATER, 2, $1, $3 ); }
           | expression GE expression                { $$ = Tree::addOperator( GE,  2, $1, $3 ); }
           | expression LE expression                { $$ = Tree::addOperator( LE,  2, $1, $3 ); }
           | expression NE expression                { $$ = Tree::addOperator( NE,  2, $1, $3 ); }
           | expression EQ expression                { $$ = Tree::addOperator( EQ,  2, $1, $3 ); }
           | expression LAND expression              { $$ = Tree::addOperator( LAND,  2, $1, $3 ); }
           | expression LOR expression               { $$ = Tree::addOperator( LOR,  2, $1, $3 ); }
           /* regex specific */
           | expression REGEX_OP expression          { $$ = Tree::addOperator( REGEX_OP, 2, $1, $3 ); }
           /* function call (consider two different cases due to builtin calls */
		   | IDENT      '(' arglist ')'  %prec FBX   { $$ = Tree::addCall( $1, $3 ); }
           | expression '(' arglist ')'              { $$ = Tree::addCall( $1, $3 ); }
           /* ternary operator */
           | '(' expression '?' expression ':' expression ')' { $$ = Tree::addOperator( QUESTION, 3, $2, $4, $6 ); }
           /* group expression */
           | '(' expression ')'                      { $$ = $2; };

%%

int h_banner(){
    printf( "Hybris %s (built: %s %s)\n"
            "Copyright (c) by %s\n",
            VERSION,
            __DATE__,
            __TIME__,
            AUTHOR );
}

int h_usage( char *argvz ){
    h_banner();
    printf("\nUsage: %s (action) (-o output) file (--trace)\n"
           "Where action could be :\n"
           "\t-e : Execute the script as an interpreter .\n"
           "\t-c : Compile the script in x-byte-code and save it to 'output' .\n"
           "\t-r : Load the x-byte-code from 'file' and run it .\n\n"
           "\t--trace : Will enable stack trace report on errors .\n\n"
           "Notes :\n"
           "If no action is specified, it's assumed as '-e file' .\n"
           "If -c is set and no output is specified, the file will be compiled into 'output.ch' .\n\n", argvz );
    return 0;
}

int main( int argc, char *argv[] ){
    if( argc < 2 ){
        return h_banner();
    }
    else{
        CmdLine cmdline( argc, argv );

        if( cmdline.isset("-h") || cmdline.isset("--help") ){
            return h_usage( argv[0] );
        }

        memset( &HGLOBALS, 0x00, sizeof(hybris_globals_t) );
        if( cmdline.isset( "-e" ) ){
            cmdline.get( "-e", HGLOBALS.source );
            HGLOBALS.action = H_EXECUTE;
        }
        else if( cmdline.isset( "-c" ) ){
            cmdline.get( "-c", HGLOBALS.source );
            HGLOBALS.action = H_COMPILE;
            if( cmdline.isset( "-o" ) ){
                cmdline.get( "-o", HGLOBALS.destination );
            }
            else{
                strcpy( HGLOBALS.destination, "output.ch" );
            }
        }
        else if( cmdline.isset( "-r" ) ){
            cmdline.get( "-r", HGLOBALS.source );
            HGLOBALS.action = H_RUN;
        }
        else{
            cmdline.nonFlaggedArg( HGLOBALS.source );
            HGLOBALS.action = H_EXECUTE;
        }

        HGLOBALS.stacktrace = cmdline.isset("--trace");

        if( h_file_exists(HGLOBALS.source) == 0 ){
            printf( "Error :'%s' no such file or directory .\n\n", HGLOBALS.source );
            return h_usage( argv[0] );
        }

        h_env_init( argc, argv );

		/* compile or execute */
        if( HGLOBALS.action != H_RUN ){
            extern FILE *yyin;
            yyin = fopen( HGLOBALS.source, "r");

			h_changepath();
            while( !feof(yyin) ){
                yyparse();
            }
            fclose(yyin);
        }
		/* run pseudo-compiled byte code
        else{
            FILE *input = fopen( HGLOBALS.source, "rb" );

			h_changepath();
            if( h_check_header(input) == 0 ){
                fclose(input);
                hybris_generic_error( "'%s' damaged or invalid file", HGLOBALS.source );
            }

            while( !feof(input) ){
                Node *tree = htree_load(input);
                htree_execute(&HVM,tree);
                Tree::release(tree);
            }
            fclose(input);
        }
        */
        h_env_release();
    }

    return 0;
}
