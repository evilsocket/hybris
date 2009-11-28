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

#include "object.h"
#include "vmem.h"
#include "tree.h"
#include "common.h"
#include "builtin.h"
#include "cmdline.h"

#define YY_(s) (char *)s

#define HMAGIC "XCS\x10\x12\x19\x85"

extern int yyparse(void);
extern int yylex(void);

extern unsigned long h_running_threads;

Object *htree_function_call( vmem_t *stackframe, Node *call, int threaded = 0 );
Object *htree_execute( vmem_t *stackframe, Node *node );
/*
void    htree_compile( Node *node, FILE *output );
Node   *htree_load( FILE *input );
*/

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


%token EOSTMT DOT DOTE PLUS PLUSE MINUS MINUSE DIV DIVE MUL MULE MOD MODE XOR XORE NOT AND ANDE OR ORE SHIFTL SHIFTLE SHIFTR SHIFTRE ASSIGN REGEX_OP
%token SUBSCRIPTADD SUBSCRIPTSET SUBSCRIPTGET WHILE FOR FOREACH FOREACHM OF TO IF QUESTION DOLLAR MAPS PTR OBJ
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

program    : body { /*exit(0);*/ };

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

arglist    : expression MAPS arglist { $$ = $3;                  $$->push_front($1); }
           | expression ',' arglist  { $$ = $3;                  $$->push_front($1); }
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

string hbuild_function_trace( char *function, vmem_t *stack, int identifiers ){
    string trace = function + string("(");
    unsigned int i;
    for( i = 0; i < stack->size(); i++ ){
        trace += " " + (identifiers ? string(stack->label(i)) + "=" : string("")) +  stack->at(i)->svalue() + (i < (stack->size() - 1) ? "," : "");
    }
    trace += " )";
    return trace;
}

Node * hresolve_call( vmem_t *stackframe, Node *call, char *name ){
    /* search first in the code segment */
	Node *function = H_UNDEFINED;
	if( (function = hybris_vc_get( &HVC, (char *)call->_call.c_str() )) != H_UNDEFINED ){
	    strcpy( name, (char *)call->_call.c_str() );
		return function;
	}
	/* then search for a function alias */
	Object *alias = H_UNDEFINED;
	if( (alias = hybris_vm_get( stackframe, (char *)call->_call.c_str() )) != H_UNDEFINED && alias->xtype == H_OT_ALIAS ){
	    strcpy( name, HVC.label( alias->xalias ) );
		return HVC.at( alias->xalias );
	}
	/* try to evaluate the call as an alias itself */
	if( call->_aliascall != NULL ){
		alias = htree_execute( stackframe, call->_aliascall );
		if( alias->xtype == H_OT_ALIAS ){
		    strcpy( name, HVC.label( alias->xalias ) );
			return HVC.at( alias->xalias );
		}
	}
	/* function is not defined */
	return H_UNDEFINED;
}

Object *htree_function_call( vmem_t *stackframe, Node *call, int threaded /*= 0*/ ){
    vmem_t stack;
    char function_name[0xFF] = {0};
    function_t builtin;
    Node *function, *id;
    unsigned int i = 0;
    vector<Node *> garbage;

    /* check if function is a builtin */
    if( (builtin = hfunction_search( (char *)call->_call.c_str() )) != H_UNDEFINED ){
        /* do object assignment */
        for( i = 0; i < call->children(); i++ ){
            /* create a clone of the statements node */
            Node *node = Tree::clone( call->child(i), node );
            stack.insert( HANONYMOUSIDENTIFIER, htree_execute( stackframe, node ) );
            /* add the node to the garbage vector to be released after the function call */
            garbage.push_back(node);
        }
        if( !threaded ){
            /* fill the stack traceing system */
            HSTACKTRACE.push_back( hbuild_function_trace( (char *)call->_call.c_str(), &stack, 0 ) );
        }
        /* call the function */
        Object *_return = builtin( &stack );

        if( !threaded ){
            /* remove the call from the stack trace */
            HSTACKTRACE.pop_back();
        }

        /* free cloned nodes */
        for( i = 0; i < garbage.size(); i++ ){
            Tree::release( garbage[i] );
        }

        /* return function evaluation value */
        return _return;
    }
    /* check for an user defined function */
    else if( (function = hresolve_call( stackframe, call, function_name )) != H_UNDEFINED ){
        vector<string> identifiers;
        unsigned int body = 0;
        /* a function could be without arguments */
        if( function->child(0)->type() == H_NT_IDENTIFIER ){
            for( i = 0, id = function->child(0); id->type() == H_NT_IDENTIFIER; i++ ){
                id = function->child(i);
                identifiers.push_back( id->_identifier );
            }
            body = i - 1;
            identifiers.pop_back();
		}

        if( identifiers.size() != call->children() ){
            if( threaded ){
                __sync_fetch_and_sub( &h_running_threads, 1 );
            }
            hybris_syntax_error( "function '%s' requires %d parameters (called with %d)",
                           function->_function.c_str(),
                           identifiers.size(),
                           call->children() );
        }

        for( i = 0; i < call->children(); i++ ){
            Node   *clone = Tree::clone( call->child(i), clone );
            Object *value = htree_execute( stackframe, clone );
            stack.insert( (char *)identifiers[i].c_str(), value );
            garbage.push_back(clone);
        }
        if( !threaded ){
            /* fill the stack traceing system */
            HSTACKTRACE.push_back( hbuild_function_trace( function_name, &stack, 1 ) );
        }

        /* call the function */
        Object *_return = htree_execute( &stack, function->child(body) );

        if( !threaded ){
            /* remove the call from the stack trace */
            HSTACKTRACE.pop_back();
        }

        /* free cloned nodes */
        for( i = 0; i < garbage.size(); i++ ){
            Tree::release( garbage[i] );
        }
        /* return function evaluation value */
        return _return;
    }
    #ifndef _LP64
    /* finally check if the function is an extern identifier loaded by dll importing routines */
    else{
        Object *external;
        if( (external = hybris_vm_get( stackframe, (char *)call->_call.c_str() )) == H_UNDEFINED ){
            if( threaded ){
                __sync_fetch_and_sub( &h_running_threads, 1 );
            }
            hybris_syntax_error( "'%s' undeclared function identifier", (char *)call->_call.c_str() );
        }
        else if( external->is_extern == 0 ){
            if( threaded ){
                __sync_fetch_and_sub( &h_running_threads, 1 );
            }
           hybris_syntax_error( "'%s' does not name a function", (char *)call->_call.c_str() );
        }
        /* at this point we're sure that it's an external, so build the frame for hdllcall */
        stack.insert( HANONYMOUSIDENTIFIER, external );
        for( i = 0; i < call->children(); i++ ){
            Node   *clone = Tree::clone( call->child(i), clone );
            Object *value = htree_execute( stackframe, clone );
            stack.insert( HANONYMOUSIDENTIFIER, value );
            garbage.push_back(clone);
        }

        if( !threaded ){
            /* fill the stack traceing system */
            HSTACKTRACE.push_back( hbuild_function_trace( (char *)call->_call.c_str(), &stack, 0 ) );
        }

        /* call the function */
        Object *_return = hdllcall(&stack);

        if( !threaded ){
            /* remove the call from the stack trace */
            HSTACKTRACE.pop_back();
        }

        /* free cloned nodes */
        for( i = 0; i < garbage.size(); i++ ){
            Tree::release( garbage[i] );
        }
        /* return function evaluation value */
        return _return;
    }
    #endif
}

Object *htree_execute( vmem_t *stackframe, Node *node ){
    if(node){
        /* some helper variables */
        Object *object      = NULL,
               *destination = NULL,
			   *to          = NULL;
        Node   *condition   = NULL,
               *variance    = NULL,
               *body        = NULL;
        int tmp, idx;

        switch(node->type()){
			/* constant value */
            case H_NT_CONSTANT   :
                return node->_constant;
            /* identifier */
            case H_NT_IDENTIFIER :
                if( (object = hybris_vm_get( stackframe, (char *)node->_identifier.c_str() )) == H_UNDEFINED ){
					/* check if the identifier is a function name */
					if( (idx = HVC.index( (char *)node->_identifier.c_str() )) != -1 ){
						/* create alias */
						object = new Object((unsigned int)idx);
					}
					else{
                    	hybris_syntax_error( "'%s' undeclared identifier", node->_identifier );
					}
                }
                return object;
            /* function definition */
            case H_NT_FUNCTION   :
				/* check for double definition */
				if( hybris_vc_get( &HVC, (char *)node->_function.c_str() ) != H_UNDEFINED ){
					hybris_syntax_error( "function '%s' already defined", node->_function.c_str() );
				}
				else if( hfunction_search( (char *)node->_function.c_str() ) != H_UNDEFINED ){
					hybris_syntax_error( "function '%s' already defined as a language builtin", node->_function.c_str() );
				}
				/* add the function to the code segment */
                hybris_vc_add( &HVC, node );
                return H_UNDEFINED;
            /* function call */
            case H_NT_CALL       :
                return htree_function_call( stackframe, node );
            /* unary, binary or ternary operator */
            case H_NT_OPERATOR   :
                switch( node->_operator ){
					/* $ */
                    case DOLLAR :
                        object = htree_execute( stackframe, node->child(0) )->toString();
                        if( (destination = hybris_vm_get( stackframe, (char *)object->xstring.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)object->xstring.c_str() );
                        }
                        return destination;

                    case PTR :
						object = htree_execute( stackframe, node->child(0) );
						return new Object( (unsigned int)( reinterpret_cast<unsigned long>(new Object(object)) ) );

					case OBJ :
						object = htree_execute( stackframe, node->child(0) );
						return object->getObject();


                    /* return */
                    case RETURN :
                        return new Object( htree_execute( stackframe, node->child(0) ) );
                    /* array[] = object; */
					case SUBSCRIPTADD :
						destination = htree_execute( stackframe, node->child(0) );
						object      = htree_execute( stackframe, node->child(1) );
						return destination->push(object);
					break;

					/* (identifier)? = object[ expression ]; */
                    case SUBSCRIPTGET :
                        if( node->children() == 3 ){
                            destination    = htree_execute( stackframe, node->child(0) );
                            object         = htree_execute( stackframe, node->child(1) );
                            (*destination) = object->at( htree_execute(stackframe, node->child(2)) );
                            return destination;
                        }
                        else{
                            object = htree_execute( stackframe, node->child(0) );
                            return object->at( htree_execute(stackframe, node->child(1)) );
                        }
                    /* object[ expression ] = expression */
                    case SUBSCRIPTSET :
                        destination = htree_execute( stackframe, node->child(0) );
                        object      = htree_execute( stackframe, node->child(2) );
                        destination->at( htree_execute(stackframe, node->child(1)), object );
                        return destination;
                    /* while( condition ){ body } */
                    case WHILE  :
                        condition = node->child(0);
                        body      = node->child(1);
                        while( htree_execute( stackframe, condition )->lvalue() ){
                            htree_execute( stackframe, body );
                        }
                        return H_UNDEFINED;
                    /* for( initialization; condition; variance ){ body } */
                    case FOR    :
                        tmp       = htree_execute( stackframe, node->child(0) )->lvalue();
                        condition = node->child(1);
                        variance  = node->child(2);
                        body      = node->child(3);
                        for( tmp;
                             htree_execute( stackframe, condition )->lvalue();
                             htree_execute( stackframe, variance )->lvalue() ){
                            htree_execute( stackframe, body );
                        }
                        return H_UNDEFINED;
					/* foreach( item of array ) */
					case FOREACH :
						body   = node->child(2);
						object = htree_execute( stackframe, node->child(1) );
						for( tmp = 0; tmp < object->xarray.size(); tmp++ ){
							hybris_vm_add( stackframe, (char *)node->child(0)->_identifier.c_str(), object->xarray[tmp] );
							htree_execute( stackframe, body );
						}
                        return H_UNDEFINED;
					/* foreach( label -> item of map ) */
					case FOREACHM :
						body   = node->child(3);
						object = htree_execute( stackframe, node->child(2) );
						for( tmp = 0; tmp < object->xmap.size(); tmp++ ){
							hybris_vm_add( stackframe, (char *)node->child(0)->_identifier.c_str(), object->xmap[tmp] );
							hybris_vm_add( stackframe, (char *)node->child(1)->_identifier.c_str(), object->xarray[tmp] );
							htree_execute( stackframe, body );
						}
                        return H_UNDEFINED;
					break;
					/* if( condition ) */
                    case IF     :
                        if( htree_execute( stackframe, node->child(0) )->lvalue() ){
                            htree_execute( stackframe, node->child(1) );
                        }
                        else if( node->children() > 2 ){
                            htree_execute( stackframe, node->child(2) );
                        }
                        return H_UNDEFINED;
                    /* (condition ? expression : expression) */
                    case QUESTION :
                        if( htree_execute( stackframe, node->child(0) )->lvalue() ){
                            return htree_execute( stackframe, node->child(1) );
                        }
                        else{
                            return htree_execute( stackframe, node->child(2) );
                        }
                    /* expression ; */
                    case EOSTMT  :
                        htree_execute( stackframe, node->child(0) );
                        return htree_execute( stackframe, node->child(1) );
                    /* expression.expression */
                    case DOT    :
                        object = htree_execute( stackframe, node->child(0) );
                        return object->dot( htree_execute( stackframe, node->child(1) ) );
				    /* expression .= expression */
                    case DOTE   :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
                        }
                        return object->dotequal( htree_execute(stackframe, node->child(1)) );
					/* identifier = expression */
                    case ASSIGN    :
                        object      = htree_execute( stackframe, node->child(1) );
                        destination = hybris_vm_add( stackframe, (char *)node->child(0)->_identifier.c_str(), object );
                        return destination;
				    /* -expression */
                    case UMINUS :
                        object = htree_execute(stackframe, node->child(0));
                        return -(*object);

					/* expression ~= expression */
					case REGEX_OP :
						return hrex_operator(  htree_execute( stackframe, node->child(0) ),  htree_execute( stackframe, node->child(1) ) );
					break;

					/* expression + expression */
                    case PLUS    :
                        return (*htree_execute(stackframe, node->child(0))) + htree_execute(stackframe, node->child(1));
					/* expression += expression */
					case PLUSE   :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
                        }
                        (*object) += htree_execute(stackframe, node->child(1));
                        return object;
                    /* expression - expression */
                    case MINUS    :
                        return (*htree_execute(stackframe, node->child(0))) - htree_execute(stackframe, node->child(1));
                    /* expression -= expression */
					case MINUSE   :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
                        }
                        (*object) -= htree_execute(stackframe, node->child(1));
                        return object;
					/* expression * expression */
                    case MUL	:
                        return (*htree_execute(stackframe, node->child(0))) * htree_execute(stackframe, node->child(1));
					/* expression *= expression */
                    case MULE	:
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) *= htree_execute(stackframe, node->child(1));
                        return object;
					/* expression / expression */
                    case DIV    :
                        return (*htree_execute(stackframe, node->child(0))) / htree_execute(stackframe, node->child(1));
					/* expression /= expression */
                    case DIVE   :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) /= htree_execute(stackframe, node->child(1));
                        return object;
					/* expression % expression */
                    case MOD    :
                        return (*htree_execute(stackframe, node->child(0))) % htree_execute(stackframe, node->child(1));
					/* expression %= expression */
                    case MODE   :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) %= htree_execute(stackframe, node->child(1));
                        return object;
					/* expression++ */
                    case INC    :
                        if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        ++(*object);
                        return object;
                    /* expression-- */
                    case DEC    :
                        if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        --(*object);
                        return object;
                    /* expression ^ expression */
                    case XOR    :
                        return (*htree_execute(stackframe, node->child(0))) ^ htree_execute(stackframe, node->child(1));
					/* expression ^= expression */
					case XORE   :
                        if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) ^= htree_execute(stackframe, node->child(1));
                        return object;
                    /* expression & expression */
                    case AND    :
                        return (*htree_execute(stackframe, node->child(0))) & htree_execute(stackframe, node->child(1));
					/* expression &= expression */
                    case ANDE   :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) &= htree_execute(stackframe, node->child(1));
                        return object;
					/* expression | expression */
                    case OR     :
                        return (*htree_execute(stackframe, node->child(0))) | htree_execute(stackframe, node->child(1));
					/* expression |= expression */
                    case ORE    :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) |= htree_execute(stackframe, node->child(1));
                        return object;
					/* expression << expression */
					case SHIFTL  :
						return (*htree_execute(stackframe, node->child(0))) << htree_execute(stackframe, node->child(1));
					/* expression <<= expression */
					case SHIFTLE :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) <<= htree_execute(stackframe, node->child(1));
                        return object;
					/* expression >> expression */
					case SHIFTR  :
						return (*htree_execute(stackframe, node->child(0))) >> htree_execute(stackframe, node->child(1));
					/* expression >>= expression */
					case SHIFTRE :
						if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                        }
                        (*object) >>= htree_execute(stackframe, node->child(1));
                        return object;
					/* ~expression */
                    case NOT    :
                        return ~(*htree_execute( stackframe, node->child(0) ));
					/* !expression */
					case LNOT   :
                        return htree_execute( stackframe, node->child(0) )->lnot();
                    /* expression < expression */
                    case LESS    :
                        return (*htree_execute(stackframe, node->child(0))) < htree_execute(stackframe, node->child(1));
                    /* expression > expression */
                    case GREATER    :
                        return (*htree_execute(stackframe, node->child(0))) > htree_execute(stackframe, node->child(1));
                    /* expression >= expression */
                    case GE     :
                        return (*htree_execute(stackframe, node->child(0))) >= htree_execute(stackframe, node->child(1));
                    /* expression <= expression */
                    case LE     :
                        return (*htree_execute(stackframe, node->child(0))) <= htree_execute(stackframe, node->child(1));
                    /* expression != expression */
                    case NE     :
                        return (*htree_execute(stackframe, node->child(0))) != htree_execute(stackframe, node->child(1));
                    /* expression == expression */
                    case EQ     :
                        return (*htree_execute(stackframe, node->child(0))) == htree_execute(stackframe, node->child(1));
                    /* expression && expression */
                    case LAND   :
                        return (*htree_execute(stackframe, node->child(0))) && htree_execute(stackframe, node->child(1));
                    /* expression || expression */
                    case LOR    :
                        return (*htree_execute(stackframe, node->child(0))) || htree_execute(stackframe, node->child(1));
                }
        }
    }

    return 0;
}
/*
void htree_compile( Node *node, FILE *output ){
    Tree::compile( node, output );
}

Node *htree_load( FILE *input ){
    return Tree::load(input);
}
*/
void h_env_release( int onerror = 0 ){
    if( HGLOBALS.action != H_COMPILE ){
        if( h_running_threads > 0 ){
            printf( "*** [WARNING] Program exiting with running threads still being executed, this may cause virtual memory corruption (use pthread_join) ! ***\n" );
        }

        #ifdef GC_SUPPORT
            hybris_vm_release( &HVM, &HVG );
            hybris_vc_release(&HVC);
        #else
            hybris_vm_release( &HVM );
        #endif
    }
    else{
        fclose(HGLOBALS.compiled);
        if( onerror ){
            unlink(HGLOBALS.destination);
        }
    }
}

void hsignal_handler( int signo ) {
    if( signo == SIGSEGV ){
        printf( "!!! SIGSEGV SIGNAL CATCHED !!!\n" );

        printf( "\n-- DATA AREA -------------------------------------\n" );
        unsigned int i, size = HVM.size();
        for( i = 0; i < size; i++ ){
            Object *o = HVM.at(i);
            if( o ){
                printf( "\t<%s> : [%s] [%d bytes]\n", HVM.label(i), Object::type( o ), o->xsize );
            }
        }
        printf( "-------------------------------------------------\n" );

        HGLOBALS.stacktrace = 1;
        hprint_stacktrace();

        h_env_release();

        exit(signo);
    }
}

void h_env_init( int argc, char *argv[] ){
    int i;
    char name[0xFF] = {0};

    if( HGLOBALS.action != H_COMPILE ){
        Object *o;
		/* initialize command line arguments */
		o = new Object( static_cast<long>(argc - 1) );
		hybris_vm_add( &HVM, (char *)"argc", o );
		delete o;
        for( i = 1; i < argc; i++ ){
            sprintf( name, "%d", i - 1 );
            o = new Object(argv[i]);
            hybris_vm_add( &HVM, name, o );
            delete o;
        }
    }
    else{
        HGLOBALS.compiled = fopen( HGLOBALS.destination, "w+b" );
        hybris_header_t header;

        memcpy( header.magic, HMAGIC, sizeof(HMAGIC) );
        header.compressed = 0;
        fwrite( &header, 1, sizeof(hybris_header_t), HGLOBALS.compiled );
    }
    /* save interpreter directory */
    getcwd( HGLOBALS.rootpath, 0xFF );

    signal( SIGSEGV, hsignal_handler );
}

int h_file_exists( char *filename ){
    FILE *fp = fopen( filename, "r" );
    if( fp ){
        fclose(fp);
        return 1;
    }
    else{
        return 0;
    }
}

int h_check_header( FILE *fp ){
    hybris_header_t header;
    fread( &header, 1, sizeof(hybris_header_t), fp );

    return (memcmp( header.magic, HMAGIC, sizeof(HMAGIC) ) == 0);
}

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

int h_changepath(){
	/* compute source path and chdir to it */
	char *ptr = strrchr( HGLOBALS.source, '/' );
	if( ptr != NULL ){
		unsigned int pos = ptr - HGLOBALS.source + 1;
		char path[0xFF]  = {0};
		strncpy( path, HGLOBALS.source, pos );
		return chdir(path);
	}
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
