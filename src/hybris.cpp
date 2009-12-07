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
#include "parser.hpp"

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
               POOL_DEL( pthread_self() );
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
                POOL_DEL( pthread_self() );
            }
            hybris_syntax_error( "'%s' undeclared function identifier", (char *)call->_call.c_str() );
        }
        else if( external->is_extern == 0 ){
            if( threaded ){
                POOL_DEL( pthread_self() );
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
					    /* check for a global defined object if the frame is not the main one */
					    if( reinterpret_cast<long>(stackframe) != reinterpret_cast<long>(&HVM) ){
                            if( (object = hybris_vm_get( &HVM, (char *)node->_identifier.c_str() )) == H_UNDEFINED ){
                                hybris_syntax_error( "'%s' undeclared identifier", node->_identifier );
                            }
					    }
                        else{
                            hybris_syntax_error( "'%s' undeclared identifier", node->_identifier );
                        }
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
                    /* expression .. expression */
                    case RANGE :
                        return htree_execute( stackframe, node->child(0) )->range( htree_execute( stackframe, node->child(1) ) );
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
                    /* do{ body }while( condition ); */
                    case DO  :
                        body      = node->child(0);
                        condition = node->child(1);
                        do{
                            htree_execute( stackframe, body );
                        }
                        while( htree_execute( stackframe, condition )->lvalue() );

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
                    /* expression! */
                    case FACT :
                        object = htree_execute( stackframe, node->child(0) );
                        return object->factorial();
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
void h_env_release( int onerror /*= 0*/ ){
    if( HGLOBALS.action != H_COMPILE ){
        pthread_mutex_lock( &h_thread_pool_mutex );
            if( h_thread_pool.size() > 0 ){
                printf( "[WARNING] Hard killing remaining running threads ... " );
                for( int pool_i = 0; pool_i < h_thread_pool.size(); pool_i++ ){
                    pthread_kill( h_thread_pool[pool_i], SIGTERM );
                }
                h_thread_pool.clear();
                printf( "done .\n" );
            }
        pthread_mutex_unlock( &h_thread_pool_mutex );


        #ifdef GC_SUPPORT
            hybris_vm_release( &HVM, &HVG );
            hybris_vc_release( &HVC );
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
        /* initialize builtins' constants */
        for( i = 0; i < NCONSTANTS; i++ ){
            hybris_vm_add( &HVM, (char *)HSTATICCONSTANTS[i].identifier.c_str(), HSTATICCONSTANTS[i].value );
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


