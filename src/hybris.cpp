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

Node * hresolve_call( h_context_t *ctx, vmem_t *stackframe, Node *call, char *name ){
    /* search first in the code segment */
	Node *function = H_UNDEFINED;
	if( (function = hybris_vc_get( &ctx->HVC, (char *)call->_call.c_str() )) != H_UNDEFINED ){
	    strcpy( name, (char *)call->_call.c_str() );
		return function;
	}
	/* then search for a function alias */
	Object *alias = H_UNDEFINED;
	if( (alias = hybris_vm_get( stackframe, (char *)call->_call.c_str() )) != H_UNDEFINED && alias->xtype == H_OT_ALIAS ){
	    strcpy( name, ctx->HVC.label( alias->xalias ) );
		return ctx->HVC.at( alias->xalias );
	}
	/* try to evaluate the call as an alias itself */
	if( call->_aliascall != NULL ){
		alias = htree_execute( ctx, stackframe, call->_aliascall );
		if( alias->xtype == H_OT_ALIAS ){
		    strcpy( name, ctx->HVC.label( alias->xalias ) );
			return ctx->HVC.at( alias->xalias );
		}
	}
	/* function is not defined */
	return H_UNDEFINED;
}

Object *htree_function_call( h_context_t *ctx, vmem_t *stackframe, Node *call, int threaded /*= 0*/ ){
    vmem_t stack;
    char function_name[0xFF] = {0};
    function_t builtin;
    Node *function, *id;
    unsigned int i = 0;
    vector<Node *> garbage;

    /* check if function is a builtin */
    if( (builtin = hfunction_search( ctx, (char *)call->_call.c_str() )) != H_UNDEFINED ){
        /* do object assignment */
        for( i = 0; i < call->children(); i++ ){
            /* create a clone of the statements node */
            Node *node = Tree::clone( call->child(i), node );
            stack.insert( HANONYMOUSIDENTIFIER, htree_execute( ctx, stackframe, node ) );
            /* add the node to the garbage vector to be released after the function call */
            garbage.push_back(node);
        }
        if( !threaded ){
            /* fill the stack traceing system */
            ctx->HSTACKTRACE.push_back( hbuild_function_trace( (char *)call->_call.c_str(), &stack, 0 ) );
        }
        /* call the function */
        Object *_return = builtin( ctx, &stack );

        if( !threaded ){
            /* remove the call from the stack trace */
            ctx->HSTACKTRACE.pop_back();
        }

        /* free cloned nodes */
        for( i = 0; i < garbage.size(); i++ ){
            Tree::release( garbage[i] );
        }

        /* return function evaluation value */
        return _return;
    }
    /* check for an user defined function */
    else if( (function = hresolve_call( ctx, stackframe, call, function_name )) != H_UNDEFINED ){
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
            #ifdef MT_SUPPORT
            if( threaded ){
               POOL_DEL( pthread_self() );
            }
            #endif
            hybris_syntax_error( "function '%s' requires %d parameters (called with %d)",
                                 function->_function.c_str(),
                                 identifiers.size(),
                                 call->children() );
        }

        for( i = 0; i < call->children(); i++ ){
            Node   *clone = Tree::clone( call->child(i), clone );
            Object *value = htree_execute( ctx, stackframe, clone );
            stack.insert( (char *)identifiers[i].c_str(), value );
            garbage.push_back(clone);
        }
        if( !threaded ){
            /* fill the stack traceing system */
            ctx->HSTACKTRACE.push_back( hbuild_function_trace( function_name, &stack, 1 ) );
        }

        /* call the function */
        Object *_return = htree_execute( ctx, &stack, function->child(body) );

        if( !threaded ){
            /* remove the call from the stack trace */
            ctx->HSTACKTRACE.pop_back();
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
            #ifdef MT_SUPPORT
            if( threaded ){
                POOL_DEL( pthread_self() );
            }
            #endif
            hybris_syntax_error( "'%s' undeclared function identifier", (char *)call->_call.c_str() );
        }
        else if( external->is_extern == 0 ){
            #ifdef MT_SUPPORT
            if( threaded ){
                POOL_DEL( pthread_self() );
            }
            #endif
           hybris_syntax_error( "'%s' does not name a function", (char *)call->_call.c_str() );
        }
        /* at this point we're sure that it's an external, so build the frame for hdllcall */
        stack.insert( HANONYMOUSIDENTIFIER, external );
        for( i = 0; i < call->children(); i++ ){
            Node   *clone = Tree::clone( call->child(i), clone );
            Object *value = htree_execute( ctx, stackframe, clone );
            stack.insert( HANONYMOUSIDENTIFIER, value );
            garbage.push_back(clone);
        }

        if( !threaded ){
            /* fill the stack traceing system */
            ctx->HSTACKTRACE.push_back( hbuild_function_trace( (char *)call->_call.c_str(), &stack, 0 ) );
        }

        /* call the function */
        Object *_return = hdllcall( ctx, &stack );

        #ifdef MT_SUPPORT
        if( !threaded ){
            /* remove the call from the stack trace */
            ctx->HSTACKTRACE.pop_back();
        }
        #endif

        /* free cloned nodes */
        for( i = 0; i < garbage.size(); i++ ){
            Tree::release( garbage[i] );
        }
        /* return function evaluation value */
        return _return;
    }
    #endif
}
/* !!! This function is full of memory leaks and it should be improved !!! */
Object *htree_execute( h_context_t *ctx, vmem_t *stackframe, Node *node ){
    /* skip undefined/null nodes */
    if( node == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    /* some helper variables */
    Object *object      = NULL,
           *destination = NULL,
           *to          = NULL;
    Node   *condition   = NULL,
           *variance    = NULL,
           *body        = NULL;
    int tmp, idx;

    #ifdef GC_SUPPORT
    hybris_vc_release( &ctx->HVC );
    #endif

    switch(node->type()){
        /* constant value */
        case H_NT_CONSTANT   :
            return node->_constant;
        /* identifier */
        case H_NT_IDENTIFIER :
            if( (object = hybris_vm_get( stackframe, (char *)node->_identifier.c_str() )) == H_UNDEFINED ){
                /* check if the identifier is a function name */
                if( (idx = ctx->HVC.index( (char *)node->_identifier.c_str() )) != -1 ){
                    /* create alias */
                    object = new Object((unsigned int)idx);
                }
                else{
                    /* check for a global defined object if the frame is not the main one */
                    if( reinterpret_cast<long>(stackframe) != reinterpret_cast<long>(&ctx->HVM) ){
                        object = hybris_vm_get( &ctx->HVM, (char *)node->_identifier.c_str() );
                        if( object == H_UNDEFINED ){
                            hybris_syntax_error( "'%s' undeclared identifier", node->_identifier.c_str() );
                        }
                    }
                    else{
                        hybris_syntax_error( "'%s' undeclared identifier", node->_identifier.c_str() );
                    }
                }
            }
            return object;
        /* function definition */
        case H_NT_FUNCTION   :
            /* check for double definition */
            if( hybris_vc_get( &ctx->HVC, (char *)node->_function.c_str() ) != H_UNDEFINED ){
                hybris_syntax_error( "function '%s' already defined", node->_function.c_str() );
            }
            else if( hfunction_search( ctx, (char *)node->_function.c_str() ) != H_UNDEFINED ){
                hybris_syntax_error( "function '%s' already defined as a language builtin", node->_function.c_str() );
            }
            /* add the function to the code segment */
            hybris_vc_add( &ctx->HVC, node );
            return H_UNDEFINED;
        /* function call */
        case H_NT_CALL       :
            return htree_function_call( ctx, stackframe, node );
        /* unary, binary or ternary operator */
        case H_NT_OPERATOR   :
            switch( node->_operator ){
                /* $ */
                case DOLLAR :
                    object = htree_execute( ctx,  stackframe, node->child(0) )->toString();
                    if( (destination = hybris_vm_get( stackframe, (char *)object->xstring.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)object->xstring.c_str() );
                    }
                    return destination;

                case PTR :
                    object = htree_execute( ctx,  stackframe, node->child(0) );
                    return new Object( (unsigned int)( reinterpret_cast<unsigned long>(new Object(object)) ) );

                case OBJ :
                    object = htree_execute( ctx,  stackframe, node->child(0) );
                    return object->getObject();


                /* return */
                case RETURN :
                    return new Object( htree_execute( ctx,  stackframe, node->child(0) ) );
                /* expression .. expression */
                case RANGE :
                    return htree_execute( ctx,  stackframe, node->child(0) )->range( htree_execute( ctx,  stackframe, node->child(1) ) );
                /* array[] = object; */
                case SUBSCRIPTADD :
                    destination = htree_execute( ctx,  stackframe, node->child(0) );
                    object      = htree_execute( ctx,  stackframe, node->child(1) );
                    return destination->push(object);
                break;

                /* (identifier)? = object[ expression ]; */
                case SUBSCRIPTGET :
                    if( node->children() == 3 ){
                        destination    = htree_execute( ctx,  stackframe, node->child(0) );
                        object         = htree_execute( ctx,  stackframe, node->child(1) );
                        (*destination) = object->at( htree_execute( ctx, stackframe, node->child(2)) );
                        return destination;
                    }
                    else{
                        object = htree_execute( ctx,  stackframe, node->child(0) );
                        return object->at( htree_execute( ctx, stackframe, node->child(1)) );
                    }
                /* object[ expression ] = expression */
                case SUBSCRIPTSET :
                    destination = htree_execute( ctx,  stackframe, node->child(0) );
                    object      = htree_execute( ctx,  stackframe, node->child(2) );
                    destination->at( htree_execute( ctx, stackframe, node->child(1)), object );
                    return destination;
                /* while( condition ){ body } */
                case WHILE  :
                    condition = node->child(0);
                    body      = node->child(1);
                    while( htree_execute( ctx,  stackframe, condition )->lvalue() ){
                        htree_execute( ctx,  stackframe, body );
                    }
                    return H_UNDEFINED;
                /* do{ body }while( condition ); */
                case DO  :
                    body      = node->child(0);
                    condition = node->child(1);
                    do{
                        htree_execute( ctx,  stackframe, body );
                    }
                    while( htree_execute( ctx,  stackframe, condition )->lvalue() );

                    return H_UNDEFINED;
                /* for( initialization; condition; variance ){ body } */
                case FOR    :
                    tmp       = htree_execute( ctx,  stackframe, node->child(0) )->lvalue();
                    condition = node->child(1);
                    variance  = node->child(2);
                    body      = node->child(3);
                    for( tmp;
                         htree_execute( ctx,  stackframe, condition )->lvalue();
                         htree_execute( ctx,  stackframe, variance )->lvalue() ){
                        htree_execute( ctx,  stackframe, body );
                    }
                    return H_UNDEFINED;
                /* foreach( item of array ) */
                case FOREACH :
                    body   = node->child(2);
                    object = htree_execute( ctx,  stackframe, node->child(1) );
                    for( tmp = 0; tmp < object->xarray.size(); tmp++ ){
                        hybris_vm_add( stackframe, (char *)node->child(0)->_identifier.c_str(), object->xarray[tmp] );
                        htree_execute( ctx,  stackframe, body );
                    }
                    return H_UNDEFINED;
                /* foreach( label -> item of map ) */
                case FOREACHM :
                    body   = node->child(3);
                    object = htree_execute( ctx,  stackframe, node->child(2) );
                    for( tmp = 0; tmp < object->xmap.size(); tmp++ ){
                        hybris_vm_add( stackframe, (char *)node->child(0)->_identifier.c_str(), object->xmap[tmp] );
                        hybris_vm_add( stackframe, (char *)node->child(1)->_identifier.c_str(), object->xarray[tmp] );
                        htree_execute( ctx,  stackframe, body );
                    }
                    return H_UNDEFINED;
                break;
                /* if( condition ) */
                case IF     :
                    if( htree_execute( ctx,  stackframe, node->child(0) )->lvalue() ){
                        htree_execute( ctx,  stackframe, node->child(1) );
                    }
                    else if( node->children() > 2 ){
                        htree_execute( ctx,  stackframe, node->child(2) );
                    }
                    return H_UNDEFINED;
                /* (condition ? expression : expression) */
                case QUESTION :
                    if( htree_execute( ctx,  stackframe, node->child(0) )->lvalue() ){
                        return htree_execute( ctx,  stackframe, node->child(1) );
                    }
                    else{
                        return htree_execute( ctx,  stackframe, node->child(2) );
                    }
                /* expression ; */
                case EOSTMT  :
                    htree_execute( ctx,  stackframe, node->child(0) );
                    return htree_execute( ctx,  stackframe, node->child(1) );
                /* expression.expression */
                case DOT    :
                    object = htree_execute( ctx,  stackframe, node->child(0) );
                    return object->dot( htree_execute( ctx,  stackframe, node->child(1) ) );
                /* expression .= expression */
                case DOTE   :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
                    }
                    return object->dotequal( htree_execute( ctx, stackframe, node->child(1)) );
                /* identifier = expression */
                case ASSIGN    :
                    object      = htree_execute( ctx,  stackframe, node->child(1) );
                    destination = hybris_vm_add( stackframe, (char *)node->child(0)->_identifier.c_str(), object );
                    return destination;
                /* -expression */
                case UMINUS :
                    object = htree_execute( ctx, stackframe, node->child(0));
                    return -(*object);

                /* expression ~= expression */
                case REGEX_OP :
                    return hrex_operator(  htree_execute( ctx,  stackframe, node->child(0) ),  htree_execute( ctx,  stackframe, node->child(1) ) );
                break;

                /* expression + expression */
                case PLUS    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) + htree_execute( ctx, stackframe, node->child(1));
                /* expression += expression */
                case PLUSE   :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
                    }
                    (*object) += htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression - expression */
                case MINUS    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) - htree_execute( ctx, stackframe, node->child(1));
                /* expression -= expression */
                case MINUSE   :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
                    }
                    (*object) -= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression * expression */
                case MUL	:
                    return (*htree_execute( ctx, stackframe, node->child(0))) * htree_execute( ctx, stackframe, node->child(1));
                /* expression *= expression */
                case MULE	:
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) *= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression / expression */
                case DIV    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) / htree_execute( ctx, stackframe, node->child(1));
                /* expression /= expression */
                case DIVE   :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) /= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression % expression */
                case MOD    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) % htree_execute( ctx, stackframe, node->child(1));
                /* expression %= expression */
                case MODE   :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) %= htree_execute( ctx, stackframe, node->child(1));
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
                    return (*htree_execute( ctx, stackframe, node->child(0))) ^ htree_execute( ctx, stackframe, node->child(1));
                /* expression ^= expression */
                case XORE   :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) ^= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression & expression */
                case AND    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) & htree_execute( ctx, stackframe, node->child(1));
                /* expression &= expression */
                case ANDE   :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) &= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression | expression */
                case OR     :
                    return (*htree_execute( ctx, stackframe, node->child(0))) | htree_execute( ctx, stackframe, node->child(1));
                /* expression |= expression */
                case ORE    :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) |= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression << expression */
                case SHIFTL  :
                    return (*htree_execute( ctx, stackframe, node->child(0))) << htree_execute( ctx, stackframe, node->child(1));
                /* expression <<= expression */
                case SHIFTLE :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) <<= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression >> expression */
                case SHIFTR  :
                    return (*htree_execute( ctx, stackframe, node->child(0))) >> htree_execute( ctx, stackframe, node->child(1));
                /* expression >>= expression */
                case SHIFTRE :
                    if( (object = hybris_vm_get( stackframe, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
                        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
                    }
                    (*object) >>= htree_execute( ctx, stackframe, node->child(1));
                    return object;
                /* expression! */
                case FACT :
                    object = htree_execute( ctx,  stackframe, node->child(0) );
                    return object->factorial();
                /* ~expression */
                case NOT    :
                    return ~(*htree_execute( ctx,  stackframe, node->child(0) ));
                /* !expression */
                case LNOT   :
                    return htree_execute( ctx,  stackframe, node->child(0) )->lnot();
                /* expression < expression */
                case LESS    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) < htree_execute( ctx, stackframe, node->child(1));
                /* expression > expression */
                case GREATER    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) > htree_execute( ctx, stackframe, node->child(1));
                /* expression >= expression */
                case GE     :
                    return (*htree_execute( ctx, stackframe, node->child(0))) >= htree_execute( ctx, stackframe, node->child(1));
                /* expression <= expression */
                case LE     :
                    return (*htree_execute( ctx, stackframe, node->child(0))) <= htree_execute( ctx, stackframe, node->child(1));
                /* expression != expression */
                case NE     :
                    return (*htree_execute( ctx, stackframe, node->child(0))) != htree_execute( ctx, stackframe, node->child(1));
                /* expression == expression */
                case EQ     :
                    return (*htree_execute( ctx, stackframe, node->child(0))) == htree_execute( ctx, stackframe, node->child(1));
                /* expression && expression */
                case LAND   :
                    return (*htree_execute( ctx, stackframe, node->child(0))) && htree_execute( ctx, stackframe, node->child(1));
                /* expression || expression */
                case LOR    :
                    return (*htree_execute( ctx, stackframe, node->child(0))) || htree_execute( ctx, stackframe, node->child(1));
            }
    }

    return H_UNDEFINED;
}

void hsignal_handler( int signo ) {
    if( signo == SIGSEGV ){
        extern h_context_t HCTX;
        HCTX.HARGS.stacktrace = 1;
        hybris_generic_error( "SIGSEGV Signal Catched" );
    }
}


void h_env_init( h_context_t *ctx, int argc, char *argv[] ){
    int i;
    char name[0xFF] = {0};
    Object *o;

    /** initialize command line arguments **/
    o = new Object( static_cast<long>(argc - 1) );
    hybris_vm_add( &ctx->HVM, (char *)"argc", o );
    delete o;
    for( i = 1; i < argc; i++ ){
        sprintf( name, "%d", i - 1 );
        o = new Object(argv[i]);
        hybris_vm_add( &ctx->HVM, name, o );
        delete o;
    }

    /** initialize builtins' constants **/
    /* misc */
    HYBRIS_DEFINE_CONSTANT( ctx, "true",  static_cast<long>(1) );
    HYBRIS_DEFINE_CONSTANT( ctx, "false", static_cast<long>(0) );
    /* fileio.cc::fseek */
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_SET", static_cast<long>(SEEK_SET) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_CUR", static_cast<long>(SEEK_CUR) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_END", static_cast<long>(SEEK_END) );
    /* fileio.cc::readdir */
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_BLK",   static_cast<long>(DT_BLK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_CHR",   static_cast<long>(DT_CHR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_DIR",   static_cast<long>(DT_DIR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_FIFO",  static_cast<long>(DT_FIFO) );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_LNK",   static_cast<long>(DT_LNK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_REG",   static_cast<long>(DT_REG)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_SOCK",  static_cast<long>(DT_SOCK) );

    for( i = 0; i < ctx->HSTATICCONSTANTS.size(); i++ ){
        hybris_vm_add( &ctx->HVM, (char *)ctx->HSTATICCONSTANTS[i]->identifier.c_str(), ctx->HSTATICCONSTANTS[i]->value );
    }

    /** initialize builtin functions */
    HYBRIS_DEFINE_BUILTIN( ctx, "isint", hisint );
	HYBRIS_DEFINE_BUILTIN( ctx, "isfloat", hisfloat );
	HYBRIS_DEFINE_BUILTIN( ctx, "ischar", hischar );
	HYBRIS_DEFINE_BUILTIN( ctx, "isstring", hisstring );
	HYBRIS_DEFINE_BUILTIN( ctx, "isarray", hisarray );
	HYBRIS_DEFINE_BUILTIN( ctx, "ismap", hismap );
	HYBRIS_DEFINE_BUILTIN( ctx, "isalias", hisalias );
	HYBRIS_DEFINE_BUILTIN( ctx, "typeof", htypeof );
	HYBRIS_DEFINE_BUILTIN( ctx, "sizeof", hsizeof );
	HYBRIS_DEFINE_BUILTIN( ctx, "toint", htoint );
	HYBRIS_DEFINE_BUILTIN( ctx, "tostring", htostring );
	HYBRIS_DEFINE_BUILTIN( ctx, "toxml", htoxml );
	HYBRIS_DEFINE_BUILTIN( ctx, "fromxml", hfromxml );
	HYBRIS_DEFINE_BUILTIN( ctx, "acos", hacos );
    HYBRIS_DEFINE_BUILTIN( ctx, "asin", hasin );
    HYBRIS_DEFINE_BUILTIN( ctx, "atan", hatan );
    HYBRIS_DEFINE_BUILTIN( ctx, "atan2", hatan2 );
    HYBRIS_DEFINE_BUILTIN( ctx, "ceil", hceil );
    HYBRIS_DEFINE_BUILTIN( ctx, "cos", hcos );
    HYBRIS_DEFINE_BUILTIN( ctx, "cosh", hcosh );
    HYBRIS_DEFINE_BUILTIN( ctx, "exp", hexp );
    HYBRIS_DEFINE_BUILTIN( ctx, "fabs", hfabs );
    HYBRIS_DEFINE_BUILTIN( ctx, "floor", hfloor );
    HYBRIS_DEFINE_BUILTIN( ctx, "fmod", hfmod );
    HYBRIS_DEFINE_BUILTIN( ctx, "log", hlog );
    HYBRIS_DEFINE_BUILTIN( ctx, "log10", hlog10 );
    HYBRIS_DEFINE_BUILTIN( ctx, "pow", hpow );
    HYBRIS_DEFINE_BUILTIN( ctx, "sin", hsin );
    HYBRIS_DEFINE_BUILTIN( ctx, "sinh", hsinh );
    HYBRIS_DEFINE_BUILTIN( ctx, "sqrt", hsqrt );
    HYBRIS_DEFINE_BUILTIN( ctx, "tan", htan );
    HYBRIS_DEFINE_BUILTIN( ctx, "tanh", htanh );
	HYBRIS_DEFINE_BUILTIN( ctx, "array", harray );
	HYBRIS_DEFINE_BUILTIN( ctx, "elements", helements );
	HYBRIS_DEFINE_BUILTIN( ctx, "pop", hpop );
	HYBRIS_DEFINE_BUILTIN( ctx, "remove", hremove );
	HYBRIS_DEFINE_BUILTIN( ctx, "contains", hcontains );
	HYBRIS_DEFINE_BUILTIN( ctx, "struct", hmap );
	HYBRIS_DEFINE_BUILTIN( ctx, "map", hmap );
	HYBRIS_DEFINE_BUILTIN( ctx, "mapelements", hmapelements );
	HYBRIS_DEFINE_BUILTIN( ctx, "mappop", hmappop );
	HYBRIS_DEFINE_BUILTIN( ctx, "unmap", hunmap );
	HYBRIS_DEFINE_BUILTIN( ctx, "ismapped", hismapped );
	HYBRIS_DEFINE_BUILTIN( ctx, "strlen", hstrlen );
	HYBRIS_DEFINE_BUILTIN( ctx, "strfind", hstrfind );
	HYBRIS_DEFINE_BUILTIN( ctx, "substr", hsubstr );
	HYBRIS_DEFINE_BUILTIN( ctx, "strreplace", hstrreplace );
	HYBRIS_DEFINE_BUILTIN( ctx, "strsplit", hstrsplit );
    HYBRIS_DEFINE_BUILTIN( ctx, "matrix",  hmatrix );
    HYBRIS_DEFINE_BUILTIN( ctx, "columns", hcolumns );
    HYBRIS_DEFINE_BUILTIN( ctx, "rows",    hrows );
	#ifdef PCRE_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "rex_match", hrex_match );
	HYBRIS_DEFINE_BUILTIN( ctx, "rex_matches", hrex_matches );
	HYBRIS_DEFINE_BUILTIN( ctx, "rex_replace", hrex_replace );
	#endif
	HYBRIS_DEFINE_BUILTIN( ctx, "print", hprint );
	HYBRIS_DEFINE_BUILTIN( ctx, "println", hprintln );
	HYBRIS_DEFINE_BUILTIN( ctx, "input", hinput );
	HYBRIS_DEFINE_BUILTIN( ctx, "exec", hexec );
	HYBRIS_DEFINE_BUILTIN( ctx, "fork", hfork );
	HYBRIS_DEFINE_BUILTIN( ctx, "getpid", hgetpid );
	HYBRIS_DEFINE_BUILTIN( ctx, "wait", hwait );
	HYBRIS_DEFINE_BUILTIN( ctx, "popen", hpopen );
	HYBRIS_DEFINE_BUILTIN( ctx, "pclose", hpclose );
    HYBRIS_DEFINE_BUILTIN( ctx, "exit", hexit );
	HYBRIS_DEFINE_BUILTIN( ctx, "var_names", hvar_names );
	HYBRIS_DEFINE_BUILTIN( ctx, "var_values", hvar_values );
	HYBRIS_DEFINE_BUILTIN( ctx, "user_functions", huser_functions );
	HYBRIS_DEFINE_BUILTIN( ctx, "core_functions", hcore_functions );
	HYBRIS_DEFINE_BUILTIN( ctx, "dyn_functions",  hdyn_functions );
	#ifndef _LP64
	HYBRIS_DEFINE_BUILTIN( ctx, "call", hcall );
	HYBRIS_DEFINE_BUILTIN( ctx, "dllopen", hdllopen );
    HYBRIS_DEFINE_BUILTIN( ctx, "dlllink", hdlllink );
    HYBRIS_DEFINE_BUILTIN( ctx, "dllcall", hdllcall );
    HYBRIS_DEFINE_BUILTIN( ctx, "dllclose", hdllclose );
	#endif
	HYBRIS_DEFINE_BUILTIN( ctx, "ticks", hticks );
	HYBRIS_DEFINE_BUILTIN( ctx, "usleep", husleep );
	HYBRIS_DEFINE_BUILTIN( ctx, "sleep", hsleep );
	HYBRIS_DEFINE_BUILTIN( ctx, "time", htime );
	HYBRIS_DEFINE_BUILTIN( ctx, "strtime", hstrtime );
	HYBRIS_DEFINE_BUILTIN( ctx, "strdate", hstrdate );
	HYBRIS_DEFINE_BUILTIN( ctx, "fopen", hfopen );
	HYBRIS_DEFINE_BUILTIN( ctx, "fseek", hfseek );
	HYBRIS_DEFINE_BUILTIN( ctx, "ftell", hftell );
	HYBRIS_DEFINE_BUILTIN( ctx, "fsize", hfsize );
	HYBRIS_DEFINE_BUILTIN( ctx, "fread", hfread );
	HYBRIS_DEFINE_BUILTIN( ctx, "fgets", hfgets );
	HYBRIS_DEFINE_BUILTIN( ctx, "fwrite", hfwrite );
	HYBRIS_DEFINE_BUILTIN( ctx, "fclose", hfclose );
	HYBRIS_DEFINE_BUILTIN( ctx, "file", hfile );
    HYBRIS_DEFINE_BUILTIN( ctx, "readdir", hreaddir );
	HYBRIS_DEFINE_BUILTIN( ctx, "settimeout", hsettimeout );
	HYBRIS_DEFINE_BUILTIN( ctx, "connect", hconnect );
	HYBRIS_DEFINE_BUILTIN( ctx, "server", hserver );
	HYBRIS_DEFINE_BUILTIN( ctx, "accept", haccept );
	HYBRIS_DEFINE_BUILTIN( ctx, "recv", hrecv );
	HYBRIS_DEFINE_BUILTIN( ctx, "send", hsend );
	HYBRIS_DEFINE_BUILTIN( ctx, "close", hclose );
	#ifdef HTTP_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "http_get", hhttp_get );
	HYBRIS_DEFINE_BUILTIN( ctx, "http_post", hhttp_post );
    #endif
	#ifdef XML_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "xml_load", hxml_load );
	HYBRIS_DEFINE_BUILTIN( ctx, "xml_parse", hxml_parse );
    #endif
    HYBRIS_DEFINE_BUILTIN( ctx, "urlencode", hurlencode );
	HYBRIS_DEFINE_BUILTIN( ctx, "urldecode", hurldecode );
    HYBRIS_DEFINE_BUILTIN( ctx, "base64encode", hbase64encode );
	HYBRIS_DEFINE_BUILTIN( ctx, "base64decode", hbase64decode );
    #ifdef MT_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "pthread_create", hpthread_create );
    HYBRIS_DEFINE_BUILTIN( ctx, "pthread_exit",   hpthread_exit );
    HYBRIS_DEFINE_BUILTIN( ctx, "pthread_join",   hpthread_join );
    #endif

    /* save interpreter directory */
    getcwd( ctx->HARGS.rootpath, 0xFF );

    #ifdef MT_SUPPORT
    ctx->h_thread_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
    #endif

    signal( SIGSEGV, hsignal_handler );
}

void h_env_release( h_context_t *ctx, int onerror /*= 0*/ ){
    if( ctx->HARGS.action != H_COMPILE ){
        #ifdef MT_SUPPORT
        pthread_mutex_lock( &ctx->h_thread_pool_mutex );
            if( ctx->h_thread_pool.size() > 0 ){
                printf( "[WARNING] Hard killing remaining running threads ... " );
                for( int pool_i = 0; pool_i < ctx->h_thread_pool.size(); pool_i++ ){
                    pthread_kill( ctx->h_thread_pool[pool_i], SIGTERM );
                }
                ctx->h_thread_pool.clear();
                printf( "done .\n" );
            }
        pthread_mutex_unlock( &ctx->h_thread_pool_mutex );
        #endif

        #ifdef GC_SUPPORT
            hybris_vm_release( &ctx->HVM, &ctx->HVG );
            hybris_vc_release( &ctx->HVC );
        #else
            hybris_vm_release( &ctx->HVM );
        #endif
    }
    else{
        fclose(ctx->HARGS.compiled);
        if( onerror ){
            unlink(ctx->HARGS.destination);
        }
    }
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

int h_changepath( h_context_t *ctx ){
	/* compute source path and chdir to it */
	char *ptr = strrchr( ctx->HARGS.source, '/' );
	if( ptr != NULL ){
		unsigned int pos = ptr - ctx->HARGS.source + 1;
		char path[0xFF]  = {0};
		strncpy( path, ctx->HARGS.source, pos );
		return chdir(path);
	}
	return 0;
}


