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

#define YY_(s) (char *)s
/*
 * Make bison parsing errors more verbose rather than a
 * idiot "syntax error" -.-
 *
 * A note to Bison developers :
 * For God sake, why don't you guys document this macro in a
 * more detailed way!
 * I spent 2 days just to end up reversing the bison generated
 * code to "find out" YYERROR_VERBOSE!!!
 */
#define YYERROR_VERBOSE 1

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

extern int yyparse(void);
extern int yylex( hyb_token_value* yylval, YYLTYPE *yyloc );

/** macros to define parse tree **/
/* get the node evaluation */
#define REDUCE_NODE(a)                			 a
/* delete the node */
#define RM_NODE(a)                      		 delete (a)
/* identifiers, attributes and constants */
#define MK_IDENT_NODE( lineno, a)                new IdentifierNode( lineno, a)
#define MK_ATTR_NODE( lineno, a,b)		         new IdentifierNode( lineno, a,b)
#define MK_STATIC_ATTR_NODE( lineno, a,b)        new IdentifierNode( lineno, asPublic, true, a, b )
#define MK_CONST_NODE( lineno, a)                new ConstantNode( lineno, a)
#define MK_ARRAY_NODE( lineno, a)		         new ExpressionNode( lineno, T_ARRAY, a )
#define MK_MAP_NODE( lineno, a)			         new ExpressionNode( lineno, T_MAP, a )
#define MK_ATTRIBUTE_REQUEST_NODE( lineno, a,b)  new AttributeRequestNode( lineno, a,b)
/* statements */
#define MK_EXPLODE_NODE( lineno, a,b)	         new StatementNode( lineno, T_EXPLODE, a, b )
#define MK_WHILE_NODE( lineno, a, b)             new StatementNode( lineno, T_WHILE, 2, a, b )
#define MK_DO_NODE( lineno, a, b)                new StatementNode( lineno, T_DO, 2, a, b )
#define MK_FOR_NODE( lineno, a,b,c,d)            new StatementNode( lineno, T_FOR, 4, a, b, c, d )
#define MK_FOREACH_NODE( lineno, a,b,c)          new StatementNode( lineno, T_FOREACH, 3, MK_IDENT_NODE( lineno, a), b, c )
#define MK_FOREACHM_NODE( lineno, a,b,c,d)       new StatementNode( lineno, T_FOREACHM, 4, MK_IDENT_NODE( lineno, a), MK_IDENT_NODE(lineno, b), c, d )
#define MK_BREAK_NODE( lineno )		  	         new StatementNode( lineno, T_BREAK, 0 )
#define MK_NEXT_NODE( lineno )		  	         new StatementNode( lineno, T_NEXT, 0 )
#define MK_RETURN_NODE( lineno, a)               new StatementNode( lineno, T_RETURN, 1, a )
#define MK_UNLESS_NODE( lineno, a,b)		     new StatementNode( lineno, T_UNLESS, 2, a, b )
#define MK_IF_NODE( lineno, a, b)                new StatementNode( lineno, T_IF, 2, a, b )
#define MK_IF_ELSE_NODE( lineno, a, b, c)        new StatementNode( lineno, T_IF, 3, a, b, c )
#define MK_SWITCH_NODE( lineno, a, b)            new StatementNode( lineno, T_SWITCH, a, b )
#define MK_SWITCH_DEF_NODE( lineno, a,b,c)       new StatementNode( lineno, T_SWITCH, a, b, c )
#define MK_THROW_NODE( lineno, a)		         new StatementNode( lineno, T_THROW, 1, a )
#define MK_TRYCATCH_NODE( lineno, a,b,c,d)       new TryCatchNode( lineno, T_TRY, a, b, c, d )
#define MK_FUNCTION_NODE( lineno, a, b)          new FunctionNode( lineno, a, 1, b )
#define MK_STRUCT_NODE( lineno, a, b)            new StructureNode( lineno, a, b )
#define MK_CLASS_NODE( lineno, a, b, c)          new ClassNode( lineno, a, b, c )
#define MK_METHOD_DECL_NODE( lineno, a, b, c)    new MethodDeclarationNode( lineno, a, b, 1, c )
#define MK_STATIC_METHOD_DECL_NODE( lineno, a,b) new MethodDeclarationNode( lineno, asPublic, a, true, 1, b )
/* expressions */
#define MK_BACKTICK_NODE( lineno, a)             new ExpressionNode( lineno, T_BACKTICK, 1, a )
#define MK_EOSTMT_NODE( lineno, a, b)            new ExpressionNode( lineno, T_EOSTMT, 2, a, b )
#define MK_REF_NODE( lineno, a)		  	         new ExpressionNode( lineno, T_REF, 1, a )
#define MK_DOLLAR_NODE( lineno, a)		         new ExpressionNode( lineno, T_DOLLAR, 1, a )
#define MK_ASSIGN_NODE( lineno, a, b)            new ExpressionNode( lineno, T_ASSIGN, 2, a, b )
#define MK_SB_NODE( lineno, a, b)	             new ExpressionNode( lineno, T_SUBSCRIPTGET, 2, a, b )
#define MK_SB_PUSH_NODE( lineno, a, b)           new ExpressionNode( lineno, T_SUBSCRIPTADD, 2, a, b )
#define MK_SB_SET_NODE( lineno, a, b, c)         new ExpressionNode( lineno, T_SUBSCRIPTSET, 3, a, b, c )
#define MK_VARGS_NODE( lineno )			         new ExpressionNode( lineno, T_VARGS, 0 )
#define MK_RANGE_NODE( lineno, a, b)		     new ExpressionNode( lineno, T_RANGE, 2, a, b )
#define MK_UMINUS_NODE( lineno, a)		         new ExpressionNode( lineno, T_UMINUS, 1, a )
#define MK_PLUS_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_PLUS, 2, a, b )
#define MK_PLUSE_NODE( lineno, a, b)		     new ExpressionNode( lineno, T_PLUSE, 2, a, b )
#define MK_MINUS_NODE( lineno, a, b)		     new ExpressionNode( lineno, T_MINUS, 2, a, b )
#define MK_MINUSE_NODE( lineno, a, b)	         new ExpressionNode( lineno, T_MINUSE, 2, a, b )
#define MK_MUL_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_MUL, 2, a, b )
#define MK_MULE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_MULE, 2, a, b )
#define MK_DIV_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_DIV, 2, a, b )
#define MK_DIVE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_DIVE, 2, a, b )
#define MK_MOD_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_MOD, 2, a, b )
#define MK_INC_NODE( lineno, a)			         new ExpressionNode( lineno, T_INC, 1, a )
#define MK_DEC_NODE( lineno, a)			         new ExpressionNode( lineno, T_DEC, 1, a )
#define MK_XOR_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_XOR, 2, a, b )
#define MK_XORE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_XORE, 2, a, b )
#define MK_NOT_NODE( lineno, a)			         new ExpressionNode( lineno, T_NOT, 1, a )
#define MK_AND_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_AND, 2, a, b )
#define MK_ANDE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_ANDE, 2, a, b )
#define MK_OR_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_OR, 2, a, b )
#define MK_ORE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_ORE, 2, a, b )
#define MK_SHIFTL_NODE( lineno, a, b)	         new ExpressionNode( lineno, T_SHIFTL, 2, a, b )
#define MK_SHIFTLE_NODE( lineno, a, b)	         new ExpressionNode( lineno, T_SHIFTLE, 2, a, b )
#define MK_SHIFTR_NODE( lineno, a, b)	         new ExpressionNode( lineno, T_SHIFTR, 2, a, b )
#define MK_SHIFTRE_NODE( lineno, a, b)	         new ExpressionNode( lineno, T_SHIFTRE, 2, a, b )
#define MK_FACT_NODE( lineno, a)			     new ExpressionNode( lineno, T_FACT, 1, a )
#define MK_LNOT_NODE( lineno, a)			     new ExpressionNode( lineno, T_L_NOT, 1, a )
#define MK_LESS_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_LESS, 2, a, b )
#define MK_GREATER_NODE( lineno, a, b)	         new ExpressionNode( lineno, T_GREATER, 2, a, b )
#define MK_GE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_GREATER_EQ, 2, a, b )
#define MK_LE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_LESS_EQ, 2, a, b )
#define MK_NE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_NOT_SAME, 2, a, b )
#define MK_EQ_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_SAME, 2, a, b )
#define MK_LAND_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_L_AND, 2, a, b )
#define MK_LOR_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_L_OR, 2, a, b )
#define MK_PCRE_NODE( lineno, a, b)		         new ExpressionNode( lineno, T_REGEX_OP, 2, a, b )
#define MK_NEW_NODE( lineno, a, b )		         new NewNode( lineno, a, b )
#define MK_CALL_NODE( lineno, a, b)              new CallNode( lineno, a, b )
#define MK_METHOD_CALL_NODE( lineno, a,b)        new MethodCallNode( lineno, a, b )
#define MK_QUESTION_NODE( lineno, a, b, c)       new StatementNode( lineno, T_QUESTION, 3, a, b, c )

/*
 * The global virtual machine holder.
 */
vm_t *__hyb_vm;

%}

%locations
%pure_parser

%token <boolean>    T_BOOLEAN;
%token <integer>    T_INTEGER;
%token <real>       T_REAL;
%token <byte>       T_CHAR;
%token <string>     T_STRING;
%token <identifier> T_IDENT;
%token <function>   T_FUNCTION_PROTOTYPE;
%token <method>   	T_METHOD_PROTOTYPE;

%token T_EOSTMT
%token T_ARRAY
%token T_MAP
%token T_BACKTICK
%token T_ASSIGN
%token T_EXPLODE
%token T_VARGS
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
%token T_UNLESS
%token T_IF
%token T_SWITCH
%token T_CASE
%token T_BREAK
%token T_NEXT
%token T_DEFAULT
%token T_QUESTION
%token T_DOLLAR
%token T_MAPS
%token T_GET_MEMBER
%token T_REF
%token T_RETURN
%token T_CALL
%token T_NEW
%token T_STRUCT
%token T_CLASS
%token T_EXTENDS
%token T_STATIC
%token T_PUBLIC
%token T_PRIVATE
%token T_PROTECTED
%token T_THROW
%token T_TRY
%token T_CATCH
%token T_FINALLY

%nonassoc T_IF_END
%nonassoc T_SB_END
%nonassoc T_SWITCH_END
%nonassoc T_CALL_END
%nonassoc T_ELSE

%left T_L_NOT T_L_AND T_L_OR
%left T_LESS T_GREATER T_SAME T_NOT_SAME T_LESS_EQ T_GREATER_EQ
%left T_RANGE T_REGEX_OP
%left T_PLUSE T_MINUSE T_XORE
%left T_DIVE T_MULE T_ANDE T_ORE
%left T_PLUS T_XOR
%left T_DIV T_MUL T_AND T_OR
%left T_MOD T_MODE
%left T_SHIFTL T_SHIFTLE T_SHIFTR T_SHIFTRE T_DOLLAR
%left T_UMINUS
%left T_MINUS

%nonassoc T_FACT
%nonassoc T_NOT
%nonassoc T_INC T_DEC

%type <node>   statement arithmeticExpression bitwiseExpression logicExpression callExpression expression statements
%type <list>   mapList
%type <list>   itemList
%type <list>   argumentList
%type <list>   caseCascade
%type <list>   attrList
%type <list>   identList
%type <list>   methodList
%type <list>   classMembers
%type <list>   classExtensions
%type <node>   finallyBlock
%type <access> accessSpecifier
%%

main : statements {

	vm_timer( __hyb_vm, VM_TIMER_START );

	__hyb_vm->state = vmExecuting;

	vm_exec( __hyb_vm, &__hyb_vm->vmem, $1 );

	vm_timer( __hyb_vm, VM_TIMER_STOP );

	RM_NODE($1);
}

mapList : expression ':' expression ',' mapList { $$ = REDUCE_NODE($5); ll_prepend_pair( $$, $1, $3 ); }
	    | expression ':' expression 			{ $$ = ll_create();  	ll_append_pair( $$, $1, $3 ); }
	    | ':'			   						{ $$ = NULL; }

itemList : expression ',' itemList { $$ = REDUCE_NODE($3); ll_prepend( $$, $1 ); }
 		 | expression              { $$ = ll_create();     ll_append( $$, $1 ); }
 		 | /* empty */			   { $$ = NULL; }

argumentList : expression ':' argumentList { $$ = REDUCE_NODE($3); ll_prepend( $$, $1 ); }
			 | expression ',' argumentList { $$ = REDUCE_NODE($3); ll_prepend( $$, $1 ); }
			 | expression                  { $$ = ll_create();     ll_append( $$, $1 ); }
			 | /* empty */                 { $$ = ll_create(); };

caseCascade  : T_CASE expression ':' statements T_BREAK T_EOSTMT caseCascade { $$ = REDUCE_NODE($7); ll_prepend_pair( $$, $2, $4 ); }
             | T_CASE expression ':' statements T_BREAK T_EOSTMT             { $$ = ll_create();     ll_append_pair( $$, $2, $4 ); }
             | /* empty */                                                   { $$ = ll_create(); };

classExtensions : T_EXTENDS identList { $$ = REDUCE_NODE($2); }
			    | /* empty */	      { $$ = NULL; };

finallyBlock	: T_FINALLY '{' statements '}' { $$ = REDUCE_NODE($3); }
				| /* empty */			       { $$ = NULL; }

accessSpecifier : T_PUBLIC    { $$ = asPublic;    }
		        | T_PRIVATE   { $$ = asPrivate;   }
		        | T_PROTECTED { $$ = asProtected; }
		        | /* empty */ { $$ = asPublic;    };

identList : T_IDENT ',' identList {
			  $$ = REDUCE_NODE($3);
			  ll_prepend( $$, MK_IDENT_NODE(@1.first_line,$1) );
		  }
		  | T_IDENT ',' T_IDENT {
			  $$ = ll_create();
			  ll_append_pair( $$, MK_IDENT_NODE(@3.first_line,$3), MK_IDENT_NODE(@1.first_line,$1) );
		  }
		  | T_IDENT {
			  $$ = ll_create();
			  ll_prepend( $$, MK_IDENT_NODE(@1.first_line,$1) );
		  }

attrList  : accessSpecifier identList T_EOSTMT attrList {
			  $$ = REDUCE_NODE($4);
			  ll_foreach( $2, node ){
				  ll_prepend( $$, MK_ATTR_NODE( @1.first_line, $1, ll_node(node) ) );
			  }
			  ll_destroy($2);
		  }
		  | accessSpecifier identList T_EOSTMT {
			  $$ = ll_create();
			  ll_foreach( $2, node ){
				  ll_prepend( $$, MK_ATTR_NODE( @1.first_line, $1, ll_node(node) ) );
			  }
			  ll_destroy($2);
		  }
		  | T_STATIC T_IDENT T_ASSIGN expression T_EOSTMT attrList {
			  $$ = REDUCE_NODE($6);
			  ll_prepend( $$, MK_STATIC_ATTR_NODE( @2.first_line, $2, REDUCE_NODE($4) ) );
		  }
		  | T_STATIC T_IDENT T_ASSIGN expression T_EOSTMT {
			  $$ = ll_create();
			  ll_prepend( $$, MK_STATIC_ATTR_NODE( @2.first_line, $2, REDUCE_NODE($4) ) );
		  }
		  | methodList {
			  $$ = REDUCE_NODE($1);
		  }

methodList : accessSpecifier T_METHOD_PROTOTYPE '{' statements '}' methodList {
				 $$ = REDUCE_NODE($6);
				 ll_prepend( $$, MK_METHOD_DECL_NODE( @1.first_line, $1, $2, $4 ) );
		   }
		   | accessSpecifier T_METHOD_PROTOTYPE '{' statements '}' {
				 $$ = ll_create();
				 ll_prepend( $$, MK_METHOD_DECL_NODE( @1.first_line, $1, $2, $4 ) );
		   }
		   | T_STATIC T_METHOD_PROTOTYPE '{' statements '}' methodList {
				 $$ = REDUCE_NODE($6);
				 ll_prepend( $$, MK_STATIC_METHOD_DECL_NODE( @2.first_line, $2, $4 ) );
		   }
		   | T_STATIC T_METHOD_PROTOTYPE '{' statements '}' {
				 $$ = ll_create();
				 ll_prepend( $$, MK_STATIC_METHOD_DECL_NODE( @2.first_line, $2, $4 ) );
		   };

classMembers : attrList methodList classMembers {
				$$ = REDUCE_NODE($3);
				ll_merge_destroy( $$, $1 );
				ll_merge_destroy( $$, $2 );
			 }
			 | methodList attrList {
				$$ = ll_create();
				ll_merge_destroy( $$, $1 );
				ll_merge_destroy( $$, $2 );
			 }
			 | attrList {
				 $$ = REDUCE_NODE($1);
			 }
			 | /* empty */
			 {  $$ = ll_create(); };

statement  : T_EOSTMT                                                   { $$ = MK_EOSTMT_NODE( @1.first_line, NULL, NULL ); }
	   	   | expression                                                 { $$ = REDUCE_NODE($1); }
           | expression T_EOSTMT                                        { $$ = REDUCE_NODE($1); }
           | T_BREAK T_EOSTMT											{ $$ = MK_BREAK_NODE(@1.first_line); }
           | T_NEXT T_EOSTMT											{ $$ = MK_NEXT_NODE(@1.first_line); }
           | T_RETURN expression T_EOSTMT                               { $$ = MK_RETURN_NODE( @2.first_line, $2 ); }
           | T_THROW expression T_EOSTMT								{ $$ = MK_THROW_NODE( @2.first_line, $2 ); }
           /* subscript operator special cases */
		   | expression '[' ']' T_ASSIGN expression T_EOSTMT            { $$ = MK_SB_PUSH_NODE( @1.first_line, $1, $5 ); }
           | expression '[' expression ']' T_ASSIGN expression T_EOSTMT { $$ = MK_SB_SET_NODE( @1.first_line, $1, $3, $6 ); }
           /* ( a, b, c ) = array; */
           | '(' identList ')' T_ASSIGN expression T_EOSTMT             { $$ = MK_EXPLODE_NODE( @2.first_line, $2, $5 ); }
           /* conditional and loops */
           | T_WHILE '(' expression ')' statement                       { $$ = MK_WHILE_NODE( @3.first_line, $3, $5 ); }
           | T_DO statement T_WHILE '(' expression ')' T_EOSTMT         { $$ = MK_DO_NODE( @2.first_line, $2, $5 ); }
		   | T_FOR '(' statement statement expression ')' statement     { $$ = MK_FOR_NODE( @3.first_line, $3, $4, $5, $7 ); }
		   | T_FOREACH '(' T_IDENT T_OF expression ')' statement        { $$ = MK_FOREACH_NODE( @3.first_line, $3, $5, $7 ); }
		   | T_FOREACH '(' T_IDENT T_MAPS T_IDENT T_OF expression ')' statement {
		   		$$ = MK_FOREACHM_NODE( @3.first_line, $3, $5, $7, $9 );
		   }
		   | statement T_UNLESS expression								{ $$ = MK_UNLESS_NODE( @1.first_line, $1, $3 ); }
           | T_IF '(' expression ')' statement %prec T_IF_END           { $$ = MK_IF_NODE( @3.first_line, $3, $5 ); }
           | T_IF '(' expression ')' statement T_ELSE statement         { $$ = MK_IF_ELSE_NODE( @3.first_line, $3, $5, $7 ); }
           | T_SWITCH '(' expression ')' '{'
                caseCascade
            '}' %prec T_SWITCH_END {
        	   $$ = MK_SWITCH_NODE( @3.first_line, $3, $6 );
           }
           | T_SWITCH '(' expression ')' '{'
                caseCascade
                T_DEFAULT ':' statements
            '}' {
        	   $$ = MK_SWITCH_DEF_NODE( @3.first_line, $3, $6, $9 );
           }
           /* statement body */
           | '{' statements '}' {
        	   $$ = $2;
           }
           /* function declaration */
           | T_FUNCTION_PROTOTYPE '{' statements '}' {
        	   $$ = MK_FUNCTION_NODE( @1.first_line, $1, $3 );
           }
           /* structure declaration */
           | T_STRUCT T_IDENT '{' attrList '}' {
        	   $$ = MK_STRUCT_NODE( @2.first_line, $2, $4 );
           }
		   /* class declaration */
           | T_CLASS T_IDENT classExtensions '{' classMembers '}' {
        	   $$ = MK_CLASS_NODE( @2.first_line, $2, $3, $5 );
           }
           /* exception handling */
           | T_TRY
			   statement
			 T_CATCH '(' T_IDENT ')'
			   statement
			 finallyBlock {
        	   $$ = MK_TRYCATCH_NODE( @2.first_line, $2, MK_IDENT_NODE( @5.first_line, $5 ), $7, $8 );
           };

statements : /* empty */ 		  { $$ = REDUCE_NODE(NULL); }
		   | statement 			  { $$ = REDUCE_NODE($1); }
		   | statements statement { $$ = MK_EOSTMT_NODE( @1.first_line, $1, $2 ); };

arithmeticExpression : expression T_PLUS expression      { $$ = MK_PLUS_NODE( @1.first_line, $1, $3 ); }
					 | expression T_PLUSE expression     { $$ = MK_PLUSE_NODE( @1.first_line, $1, $3 ); }
					 | expression T_MINUS expression     { $$ = MK_MINUS_NODE( @1.first_line, $1, $3 ); }
					 | expression T_MINUSE expression    { $$ = MK_MINUSE_NODE( @1.first_line, $1, $3 ); }
					 | expression T_MUL expression       { $$ = MK_MUL_NODE( @1.first_line, $1, $3 ); }
					 | expression T_MULE expression      { $$ = MK_MULE_NODE( @1.first_line, $1, $3 ); }
					 | expression T_DIV expression       { $$ = MK_DIV_NODE( @1.first_line, $1, $3 ); }
					 | expression T_DIVE expression      { $$ = MK_DIVE_NODE( @1.first_line, $1, $3 ); }
					 | expression T_MOD expression       { $$ = MK_MOD_NODE( @1.first_line, $1, $3 ); }
					 | T_MINUS expression %prec T_UMINUS { $$ = MK_UMINUS_NODE( @2.first_line, $2 ); }
					 | expression T_INC                  { $$ = MK_INC_NODE( @1.first_line, $1 ); }
					 | expression T_DEC                  { $$ = MK_DEC_NODE( @1.first_line, $1 ); };

bitwiseExpression : expression T_XOR expression     { $$ = MK_XOR_NODE( @1.first_line, $1, $3 ); }
				  | expression T_XORE expression    { $$ = MK_XORE_NODE( @1.first_line, $1, $3 ); }
				  | T_NOT expression                { $$ = MK_NOT_NODE( @1.first_line, $2 ); }
				  | expression T_AND expression     { $$ = MK_AND_NODE( @1.first_line, $1, $3 ); }
				  | expression T_ANDE expression    { $$ = MK_ANDE_NODE( @1.first_line, $1, $3 ); }
				  | expression T_OR expression      { $$ = MK_OR_NODE( @1.first_line, $1, $3 ); }
				  | expression T_ORE expression     { $$ = MK_ORE_NODE( @1.first_line, $1, $3 ); }
				  | expression T_SHIFTL expression  { $$ = MK_SHIFTL_NODE( @1.first_line, $1, $3 ); }
				  | expression T_SHIFTLE expression { $$ = MK_SHIFTLE_NODE( @1.first_line, $1, $3 ); }
				  | expression T_SHIFTR expression  { $$ = MK_SHIFTR_NODE( @1.first_line, $1, $3 ); }
				  | expression T_SHIFTRE expression { $$ = MK_SHIFTRE_NODE( @1.first_line, $1, $3 ); }
				  | expression T_L_NOT              { $$ = MK_FACT_NODE( @1.first_line, $1 ); };

logicExpression : T_L_NOT expression                 			   { $$ = MK_LNOT_NODE( @2.first_line, $2 ); }
				| expression T_LESS expression       			   { $$ = MK_LESS_NODE( @1.first_line, $1, $3 ); }
				| expression T_GREATER expression    			   { $$ = MK_GREATER_NODE( @1.first_line, $1, $3 ); }
				| expression T_GREATER_EQ expression 			   { $$ = MK_GE_NODE( @1.first_line, $1, $3 ); }
				| expression T_LESS_EQ expression    			   { $$ = MK_LE_NODE( @1.first_line, $1, $3 ); }
				| expression T_NOT_SAME expression   			   { $$ = MK_NE_NODE( @1.first_line, $1, $3 ); }
				| expression T_SAME expression       			   { $$ = MK_EQ_NODE( @1.first_line, $1, $3 ); }
				| expression T_L_AND expression      			   { $$ = MK_LAND_NODE( @1.first_line, $1, $3 ); }
				| expression T_L_OR expression       			   { $$ = MK_LOR_NODE( @1.first_line, $1, $3 ); }
				| '(' expression '?' expression ':' expression ')' { $$ = MK_QUESTION_NODE( @2.first_line, $2, $4, $6 ); };

callExpression : /* expression . <identifier>( ... ) */
				 expression T_GET_MEMBER T_IDENT '(' argumentList ')' { $$ = MK_METHOD_CALL_NODE( @1.first_line, $1, MK_CALL_NODE( @3.first_line, $3, $5 ) ); }
				 /* new <identifier>( ... ) */
			   | T_NEW T_IDENT '(' argumentList ')' 				  { $$ = MK_NEW_NODE( @2.first_line, $2, $4 ); }
			     /* <identifier>( ... ) */
			   | T_IDENT    '(' argumentList ')' %prec T_CALL_END     { $$ = MK_CALL_NODE( @1.first_line, $1, $3 ); }
			     /* expression ( ... ) */
			   | expression '(' argumentList ')'                      { $$ = MK_CALL_NODE( @1.first_line, $1, $3 ); };

expression : T_BOOLEAN										  { $$ = MK_CONST_NODE(@1.first_line, $1); }
		   | T_INTEGER                                        { $$ = MK_CONST_NODE(@1.first_line, $1); }
           | T_REAL                                           { $$ = MK_CONST_NODE(@1.first_line, $1); }
           | T_CHAR                                           { $$ = MK_CONST_NODE(@1.first_line, $1); }
           | T_STRING                                         { $$ = MK_CONST_NODE(@1.first_line, $1); }
           | '`' expression '`'								  { $$ = MK_BACKTICK_NODE(@2.first_line, $2); }
           | '[' itemList ']'                                 { $$ = MK_ARRAY_NODE(@2.first_line, $2); }
           | '[' mapList ']'								  { $$ = MK_MAP_NODE(@2.first_line, $2); }
           /* expression . <identifier> */
           | expression T_GET_MEMBER T_IDENT 				  { $$ = MK_ATTRIBUTE_REQUEST_NODE( @1.first_line, $1, MK_IDENT_NODE(@3.first_line, $3) ); }
           /* identifier */
           | T_IDENT                                          { $$ = MK_IDENT_NODE(@1.first_line, $1); }
           /* @ arguments vector */
           | T_VARGS										  { $$ = MK_VARGS_NODE(@1.first_line); }
           /* expression evaluation returns an identifier */
           | T_DOLLAR expression                              { $$ = MK_DOLLAR_NODE(@2.first_line, $2); }
           /* object reference */
           | T_AND expression								  { $$ = MK_REF_NODE(@2.first_line, $2); }
           /* attribute declaration/assignation */
           | expression T_ASSIGN expression                   { $$ = MK_ASSIGN_NODE( @1.first_line, $1, $3 ); }
		   /* identifier declaration/assignation */
		   | T_IDENT T_ASSIGN expression       				  { $$ = MK_ASSIGN_NODE( @1.first_line, MK_IDENT_NODE(@1.first_line, $1), $3 ); }
           /* a single subscript could be an expression itself */
           | expression '[' expression ']' %prec T_SB_END     { $$ = MK_SB_NODE( @1.first_line, $1, $3 ); }
           /* range evaluation */
           | expression T_RANGE expression                    { $$ = MK_RANGE_NODE( @1.first_line, $1, $3 ); }
           /* arithmetic */
           | arithmeticExpression 							  { $$ = REDUCE_NODE($1); }
           /* bitwise */
           | bitwiseExpression 								  { $$ = REDUCE_NODE($1); }
           /* logic */
           | logicExpression 								  { $$ = REDUCE_NODE($1); }
           /* regex specific */
           | expression T_REGEX_OP expression                 { $$ = MK_PCRE_NODE( @1.first_line, $1, $3 ); }
           /* call */
           | callExpression									  { $$ = REDUCE_NODE($1); }
           /* group expression */
           | '(' expression ')'                               { $$ = REDUCE_NODE($2); };

%%
