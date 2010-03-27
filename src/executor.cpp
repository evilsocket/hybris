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
#include "executor.h"
#include "parser.hpp"

Executor::Executor( h_context_t *context ) :
    ctx(context),
    vm(&context->vmem),
    vc(&context->vcode) {
}

Executor::~Executor(){

}

Object *Executor::exec( vframe_t *frame, Node *node ){
    /* skip undefined/null nodes */
    if( node == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    switch(node->type()){
        /* identifier */
        case H_NT_IDENTIFIER :
            return onIdentifier( frame, node );
        /* constant value */
        case H_NT_CONSTANT   :
            return onConstant( frame, node );
        /* function definition */
        case H_NT_FUNCTION   :
            return onFunctionDeclaration( frame, node );
        /* function call */
        case H_NT_CALL       :
            return onFunctionCall( frame, node );
        /* unary, binary or ternary operator */
        case H_NT_OPERATOR   :
            switch( node->_operator ){
                /* identifier = expression */
                case ASSIGN    :
                    return onAssign( frame, node );
                /* expression ; */
                case EOSTMT  :
                    return onEostmt( frame, node );
                /* if( condition ) */
                case IF     :
                    return onIf( frame, node );
                /* return */
                case RETURN :
                    return onReturn( frame, node );
                /* $ */
                case DOLLAR :
                    return onDollar( frame, node );
                /* * */
                case PTR :
                    return onPointer( frame, node );
                case OBJ :
                    return onObject( frame, node );
                /* expression .. expression */
                case RANGE :
                    return onRange( frame, node );
                /* array[] = object; */
                case SUBSCRIPTADD :
                    return onSubscriptAdd( frame, node );
                /* (identifier)? = object[ expression ]; */
                case SUBSCRIPTGET :
                    return onSubscriptGet( frame, node );
                /* object[ expression ] = expression */
                case SUBSCRIPTSET :
                    return onSubscriptSet( frame, node );
                /* while( condition ){ body } */
                case WHILE  :
                    return onWhile( frame, node );
                /* do{ body }while( condition ); */
                case DO  :
                    return onDo( frame, node );
                /* for( initialization; condition; variance ){ body } */
                case FOR    :
                    return onFor( frame, node );
                /* foreach( item of array ) */
                case FOREACH :
                    return onForeach( frame, node );
                /* foreach( label -> item of map ) */
                case FOREACHM :
                    return onForeachm( frame, node );
                break;
                /* (condition ? expression : expression) */
                case QUESTION :
                    return onQuestion( frame, node );
                /* expression.expression */
                case DOT    :
                    return onDot( frame, node );
                /* expression .= expression */
                case DOTE   :
                    return onDote( frame, node );
                /* -expression */
                case UMINUS :
                    return onUminus( frame, node );
                /* expression ~= expression */
                case REGEX_OP :
                    return onRegex( frame, node );
                /* expression + expression */
                case PLUS    :
                    return onPlus( frame, node );
                /* expression += expression */
                case PLUSE   :
                    return onPluse( frame, node );
                /* expression - expression */
                case MINUS    :
                    return onMinus( frame, node );
                /* expression -= expression */
                case MINUSE   :
                    return onMinuse( frame, node );
                /* expression * expression */
                case MUL	:
                    return onMul( frame, node );
                /* expression *= expression */
                case MULE	:
                    return onMule( frame, node );
                /* expression / expression */
                case DIV    :
                    return onDiv( frame, node );
                /* expression /= expression */
                case DIVE   :
                    return onDive( frame, node );
                /* expression % expression */
                case MOD    :
                    return onMod( frame, node );
                /* expression %= expression */
                case MODE   :
                    return onMode( frame, node );
                /* expression++ */
                case INC    :
                    return onInc( frame, node );
                /* expression-- */
                case DEC    :
                    return onDec( frame, node );
                /* expression ^ expression */
                case XOR    :
                    return onXor( frame, node );
                /* expression ^= expression */
                case XORE   :
                    return onXore( frame, node );
                /* expression & expression */
                case AND    :
                    return onAnd( frame, node );
                /* expression &= expression */
                case ANDE   :
                    return onAnde( frame, node );
                /* expression | expression */
                case OR     :
                    return onOr( frame, node );
                /* expression |= expression */
                case ORE    :
                    return onOre( frame, node );
                /* expression << expression */
                case SHIFTL  :
                    return onShiftl( frame, node );
                /* expression <<= expression */
                case SHIFTLE :
                    return onShiftle( frame, node );
                /* expression >> expression */
                case SHIFTR  :
                    return onShiftr( frame, node );
                /* expression >>= expression */
                case SHIFTRE :
                    return onShiftre( frame, node );
                /* expression! */
                case FACT :
                    return onFact( frame, node );
                /* ~expression */
                case NOT    :
                    return onNot( frame, node );
                /* !expression */
                case LNOT   :
                    return onLnot( frame, node );
                /* expression < expression */
                case LESS    :
                    return onLess( frame, node );
                /* expression > expression */
                case GREATER    :
                    return onGreater( frame, node );
                /* expression >= expression */
                case GE     :
                    return onGe( frame, node );
                /* expression <= expression */
                case LE     :
                    return onLe( frame, node );
                /* expression != expression */
                case NE     :
                    return onNe( frame, node );
                /* expression == expression */
                case EQ     :
                    return onEq( frame, node );
                /* expression && expression */
                case LAND   :
                    return onLand( frame, node );
                /* expression || expression */
                case LOR    :
                    return onLor( frame, node );
            }
    }

    return H_UNDEFINED;
}

Node * Executor::findEntryPoint( vframe_t *frame, Node *call, char *name ){
    char *callname = (char *)call->_call.c_str();

    /* search first in the code segment */
	Node *function = H_UNDEFINED;
	if( (function = hybris_vc_get( vc, callname )) != H_UNDEFINED ){
	    strcpy( name, callname );
		return function;
	}
	/* then search for a function alias */
	Object *alias = H_UNDEFINED;
	if( (alias = hybris_vm_get( frame, callname )) != H_UNDEFINED && alias->xtype == H_OT_ALIAS ){
	    strcpy( name, vc->label( alias->xalias ) );
		return vc->at( alias->xalias );
	}
	/* try to evaluate the call as an alias itself */
	if( call->_aliascall != NULL ){
		alias = exec( frame, call->_aliascall );
		if( alias->xtype == H_OT_ALIAS ){
		    strcpy( name, vc->label( alias->xalias ) );
			return vc->find( name );
		}
	}
	/* function is not defined */
	return H_UNDEFINED;
}

Object *Executor::onIdentifier( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    int     idx;
    char   *identifier = (char *)node->_identifier.c_str();

    // search for the identifier on the function frame
    o = hybris_vm_get( frame, identifier );
    if( o == H_UNDEFINED && H_ADDRESS_OF(frame) != H_ADDRESS_OF(vm) ){
        // search it on the global frame if it's different from local frame
        o = hybris_vm_get( vm, identifier );
    }
    // search for it as a function name
    if( o == H_UNDEFINED ){
        idx = vc->index( identifier );
        if( idx != -1 ){
            o = new Object((unsigned int)idx);
        }
        // identifier not found
        else{
            hybris_syntax_error( "'%s' undeclared identifier", identifier );
        }
    }

    return o;
}

Object *Executor::onConstant( vframe_t *frame, Node *node ){
    return node->_constant;
}

Object *Executor::onFunctionDeclaration( vframe_t *frame, Node *node ){
    /* check for double definition */
    if( hybris_vc_get( vc, (char *)node->_function.c_str() ) != H_UNDEFINED ){
        hybris_syntax_error( "function '%s' already defined", node->_function.c_str() );
    }
    else if( hfunction_search( ctx, (char *)node->_function.c_str() ) != H_UNDEFINED ){
        hybris_syntax_error( "function '%s' already defined as a language builtin", node->_function.c_str() );
    }
    /* add the function to the code segment */
    hybris_vc_add( vc, node );

    return H_UNDEFINED;
}

Object *Executor::onFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    vframe_t stack;
    char function_name[0xFF] = {0};
    function_t builtin;
    Node *node, *function, *id, *clone;
    unsigned int i = 0, children;
    Object *value = H_UNDEFINED;
    H_NODE_TYPE type;
    char *callname = (char *)call->_call.c_str();

    /* check if function is a builtin */
    builtin = hfunction_search( ctx, callname );
    if( builtin != H_UNDEFINED ){
        children = call->children();
        /* do object assignment */
        for( i = 0; i < children; ++i ){
            /* create function stack value */
            node  = call->child(i);
            value = exec( frame, node );
            /* prevent value from being deleted, we'll take care of it */
            if( value != H_UNDEFINED ){
                value->setGarbageAttribute( ~H_OA_GARBAGE );
            }
            stack.insert( HANONYMOUSIDENTIFIER, value );
        }

        #ifdef MT_SUPPORT
        pthread_mutex_lock( &ctx->th_mutex );
        #endif

        /* fill the stack traceing system */
        ctx->stack_trace.push_back( h_trace( callname, &stack, 0 ) );

        #ifdef MT_SUPPORT
        pthread_mutex_unlock( &ctx->th_mutex );
        #endif

        /* call the function */
        Object *_return = builtin( ctx, &stack );

        for( i = 0; i < children; i++ ){
            value = stack.at(i);
            if( value != H_UNDEFINED && H_IS_NOT_CONSTANT(value) ){
                delete value;
            }
        }

        #ifdef MT_SUPPORT
        pthread_mutex_lock( &ctx->th_mutex );
        #endif

        /* remove the call from the stack trace */
        ctx->stack_trace.pop_back();

        #ifdef MT_SUPPORT
        pthread_mutex_unlock( &ctx->th_mutex );
        #endif

        /* return function evaluation value */
        return _return;
    }
    /* check for an user defined function */
    else if( (function = findEntryPoint( frame, call, function_name )) != H_UNDEFINED ){
        vector<string> identifiers;
        unsigned int body = 0;
        /* a function could be without arguments */
        if( function->child(0)->type() == H_NT_IDENTIFIER ){
            id       = function->child(0);
            children = function->children();
            type     = id->type();

            for( i = 0; type == H_NT_IDENTIFIER && i < children; ++i ){
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

        children = call->children();
        for( i = 0; i < children; ++i ){
            clone = call->child(i);
            value = exec( frame, clone );
            /* prevent value from being deleted, we'll take care of it */
            if( value != H_UNDEFINED ){
                value->setGarbageAttribute( ~H_OA_GARBAGE );
            }
            stack.insert( (char *)identifiers[i].c_str(), value );
        }

        #ifdef MT_SUPPORT
        pthread_mutex_lock( &ctx->th_mutex );
        #endif

        /* fill the stack traceing system */
        ctx->stack_trace.push_back( h_trace( callname, &stack, 0 ) );

        #ifdef MT_SUPPORT
        pthread_mutex_unlock( &ctx->th_mutex );
        #endif

        /* call the function */
        Object *_return = exec( &stack, function->child(body) );

        for( i = 0; i < children; i++ ){
            value = stack.at(i);
            if( value != H_UNDEFINED && H_IS_NOT_CONSTANT(value) ){
                delete value;
            }
        }

        #ifdef MT_SUPPORT
        pthread_mutex_lock( &ctx->th_mutex );
        #endif

        /* remove the call from the stack trace */
        ctx->stack_trace.pop_back();
        #ifdef MT_SUPPORT
        pthread_mutex_unlock( &ctx->th_mutex );
        #endif

        /* return function evaluation value */
        return _return;
    }
    #ifndef _LP64
    /* finally check if the function is an extern identifier loaded by dll importing routines */
    else{
        Object *external;
        if( (external = hybris_vm_get( frame, callname )) == H_UNDEFINED ){
            #ifdef MT_SUPPORT
            if( threaded ){
                POOL_DEL( pthread_self() );
            }
            #endif
            hybris_syntax_error( "'%s' undeclared function identifier", callname );
        }
        else if( (external->attributes & H_OA_EXTERN) != H_OA_EXTERN ){
            #ifdef MT_SUPPORT
            if( threaded ){
                POOL_DEL( pthread_self() );
            }
            #endif
           hybris_syntax_error( "'%s' does not name a function", callname );
        }
        /* at this point we're sure that it's an external, so build the frame for hdllcall */
        stack.insert( HANONYMOUSIDENTIFIER, external );
        children = call->children();
        for( i = 0; i < children; ++i ){
            clone = call->child(i);
            value = exec( frame, clone );
            /* prevent value from being deleted, we'll take care of it */
            if( value != H_UNDEFINED ){
                value->setGarbageAttribute( ~H_OA_GARBAGE );
            }
            stack.insert( HANONYMOUSIDENTIFIER, value );
        }

        #ifdef MT_SUPPORT
        pthread_mutex_lock( &ctx->th_mutex );
        #endif

        /* fill the stack traceing system */
        ctx->stack_trace.push_back( h_trace( callname, &stack, 0 ) );
        #ifdef MT_SUPPORT
        pthread_mutex_unlock( &ctx->th_mutex );
        #endif

        /* call the function */
        Object *_return = hdllcall( ctx, &stack );

        for( i = 0; i < children; i++ ){
            value = stack.at(i);
            if( value != H_UNDEFINED && H_IS_NOT_CONSTANT(value) ){
                delete value;
            }
        }

        #ifdef MT_SUPPORT
        pthread_mutex_lock( &ctx->th_mutex );
        #endif

        /* remove the call from the stack trace */
        ctx->stack_trace.pop_back();
        #ifdef MT_SUPPORT
        pthread_mutex_unlock( &ctx->th_mutex );
        #endif

        /* return function evaluation value */
        return _return;
    }
    #endif
}

Object *Executor::onDollar( vframe_t *frame, Node *node ){
    Object *o    = H_UNDEFINED,
           *name = H_UNDEFINED;

    o    = exec( frame, node->child(0) );
    name = o->toString();

    H_FREE_GARBAGE(o);

    if( (o = hybris_vm_get( frame, (char *)name->xstring.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", (char *)name->xstring.c_str() );
    }

    H_FREE_GARBAGE(name);

    return o;
}

Object *Executor::onPointer( vframe_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = exec( frame, node->child(0) );
    res = new Object( (unsigned int)( H_ADDRESS_OF(o) ) );

    H_FREE_GARBAGE(o);

    return res;
}

Object *Executor::onObject( vframe_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = exec( frame, node->child(0) );
    res = o->getObject();

    H_FREE_GARBAGE(o);

    return res;
}

Object *Executor::onReturn( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    return o;
}

Object *Executor::onRange( vframe_t *frame, Node *node ){
    Object *range = H_UNDEFINED,
           *from  = H_UNDEFINED,
           *to    = H_UNDEFINED;

    from  = exec( frame, node->child(0) );
    to    = exec( frame, node->child(1) );
    range = from->range( to );

    H_FREE_GARBAGE(from);
    H_FREE_GARBAGE(to);

    return range;
}

Object *Executor::onSubscriptAdd( vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *object = H_UNDEFINED,
           *res    = H_UNDEFINED;

    array  = exec( frame, node->child(0) );
    object = exec( frame, node->child(1) );
    res    = array->push(object);

    H_FREE_GARBAGE(array);
    H_FREE_GARBAGE(object);

    return res;
}

Object *Executor::onSubscriptGet( vframe_t *frame, Node *node ){
    Object *identifier = H_UNDEFINED,
           *array      = H_UNDEFINED,
           *index      = H_UNDEFINED,
           *result     = H_UNDEFINED;

    if( node->children() == 3 ){
        identifier    = exec( frame, node->child(0) );
        array         = exec( frame, node->child(1) );
        index         = exec( frame, node->child(2) );
        (*identifier) = array->at( index );
        result        = identifier;

        H_FREE_GARBAGE(array);
    }
    else{
        array  = exec( frame, node->child(0) );
        index  = exec( frame, node->child(1) );
        result = array->at( index );
    }

    H_FREE_GARBAGE(index);

    return result;
}

Object *Executor::onSubscriptSet( vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *index  = H_UNDEFINED,
           *object = H_UNDEFINED;

    array  = exec( frame, node->child(0) );
    index  = exec( frame, node->child(1) );
    object = exec( frame, node->child(2) );

    array->at( index, object );

    H_FREE_GARBAGE(object);
    H_FREE_GARBAGE(index);

    return array;
}

Object *Executor::onWhile( vframe_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    condition = node->child(0);
    body      = node->child(1);

    while( (boolean = exec(  frame, condition ))->lvalue() ){
        result = exec( frame, body );
        H_FREE_GARBAGE(result);
        H_FREE_GARBAGE(boolean);
    }
    H_FREE_GARBAGE(boolean);

    return H_UNDEFINED;
}

Object *Executor::onDo( vframe_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    body      = node->child(0);
    condition = node->child(1);
    do{
        result = exec( frame, body );
        H_FREE_GARBAGE(result);
        H_FREE_GARBAGE(boolean);
    }
    while( (boolean = exec(  frame, condition ))->lvalue() );

    H_FREE_GARBAGE(result);
    H_FREE_GARBAGE(boolean);

    return H_UNDEFINED;
}

Object *Executor::onFor( vframe_t *frame, Node *node ){
    Node *condition,
         *increment,
         *body;

    Object *init    = H_UNDEFINED,
           *boolean = H_UNDEFINED,
           *inc     = H_UNDEFINED,
           *result  = H_UNDEFINED;

    init      = exec(  frame, node->child(0) );
    condition = node->child(1);
    increment = node->child(2);
    body      = node->child(3);
    for( init;
         (boolean = exec(  frame, condition ))->lvalue();
         (inc     = exec(  frame, increment )) ){

        result = exec( frame, body );
        H_FREE_GARBAGE(result);
        H_FREE_GARBAGE(boolean);
        H_FREE_GARBAGE(inc);
    }

    H_FREE_GARBAGE(boolean);
    H_FREE_GARBAGE(inc);
    H_FREE_GARBAGE(init);

    return H_UNDEFINED;
}

Object *Executor::onForeach( vframe_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *identifier;

    identifier = (char *)node->child(0)->_identifier.c_str();
    map        = exec( frame, node->child(1) );
    body       = node->child(2);
    size       = map->xarray.size();

    for( i = 0; i < size; ++i ){
        hybris_vm_add( frame, identifier, map->xarray[i] );
        result = exec( frame, body );
        H_FREE_GARBAGE(result);
    }

    H_FREE_GARBAGE(map);

    return H_UNDEFINED;
}

Object *Executor::onForeachm( vframe_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *key_identifier,
           *value_identifier;

    key_identifier   = (char *)node->child(0)->_identifier.c_str();
    value_identifier = (char *)node->child(1)->_identifier.c_str();
    map              = exec( frame, node->child(2) );
    body             = node->child(3);
    size             = map->xmap.size();

    for( i = 0; i < size; ++i ){
        hybris_vm_add( frame, key_identifier,   map->xmap[i] );
        hybris_vm_add( frame, value_identifier, map->xarray[i] );
        result = exec(  frame, body );
        H_FREE_GARBAGE(result);
    }

    H_FREE_GARBAGE(map);

    return H_UNDEFINED;
}

Object *Executor::onIf( vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = exec(  frame, node->child(0) );

    if( boolean->lvalue() ){
        result = exec( frame, node->child(1) );
    }
    /* handle else case */
    else if( node->children() > 2 ){
        result = exec( frame, node->child(2) );
    }

    H_FREE_GARBAGE(boolean);
    H_FREE_GARBAGE(result);

    return H_UNDEFINED;
}

Object *Executor::onQuestion( vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = exec(  frame, node->child(0) );

    if( boolean->lvalue() ){
        result = exec( frame, node->child(1) );
    }
    else{
        result = exec( frame, node->child(2) );
    }

    H_FREE_GARBAGE(boolean);

    return result;
}

Object *Executor::onEostmt( vframe_t *frame, Node *node ){
    Object *res_1 = H_UNDEFINED,
           *res_2 = H_UNDEFINED;

    res_1 = exec( frame, node->child(0) );
    res_2 = exec( frame, node->child(1) );

    H_FREE_GARBAGE(res_1);

    return res_2;
}

Object *Executor::onDot( vframe_t *frame, Node *node ){
    Object *a      = H_UNDEFINED,
           *b      = H_UNDEFINED,
           *result = H_UNDEFINED;

    a      = exec( frame, node->child(0) );
    b      = exec( frame, node->child(1) );
    result = a->dot( b );

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return result;
}

Object *Executor::onDote( vframe_t *frame, Node *node ){
    Object *a      = H_UNDEFINED,
           *b      = H_UNDEFINED,
           *result = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b      = exec( frame, node->child(1) );
    result = a->dotequal( b );

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return result;
}

Object *Executor::onAssign( vframe_t *frame, Node *node ){
    Object *object = H_UNDEFINED,
           *value  = H_UNDEFINED;

    value  = exec(  frame, node->child(1) );
    object = hybris_vm_add( frame, (char *)node->child(0)->_identifier.c_str(), value );

    H_FREE_GARBAGE(value);

    return object;
}

Object *Executor::onUminus( vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = exec( frame, node->child(0) );
    result = -(*o);

    H_FREE_GARBAGE(o);

    return result;
}

Object *Executor::onRegex( vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *regexp = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = exec(  frame, node->child(0) );
    regexp = exec(  frame, node->child(1) );
    result = hrex_operator( o, regexp );

    H_FREE_GARBAGE(o);
    H_FREE_GARBAGE(regexp);

    return result;
}

Object *Executor::onPlus( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) + b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onPluse( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) += b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onMinus( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) - b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onMinuse( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) -= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onMul( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) * b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onMule( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) *= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onDiv( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) / b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onDive( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) /= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onMod( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) % b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onMode( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) %= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onInc( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    if( (o = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
    }
    ++(*o);
    return o;
}

Object *Executor::onDec( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    if( (o = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
    }
    --(*o);
    return o;
}

Object *Executor::onXor( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) ^ b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onXore( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) ^= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onAnd( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) & b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onAnde( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) &= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onOr( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) | b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onOre( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) |= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onShiftl( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) << b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onShiftle( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) <<= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onShiftr( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) >> b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onShiftre( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = exec( frame, node->child(1) );

    (*a) >>= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Executor::onFact( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = o->factorial();

    H_FREE_GARBAGE(o);

    return r;
}

Object *Executor::onNot( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = ~(*o);

    H_FREE_GARBAGE(o);

    return r;
}

Object *Executor::onLnot( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = o->lnot();

    H_FREE_GARBAGE(o);

    return r;
}

Object *Executor::onLess( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) < b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onGreater( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) > b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onGe( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) >= b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onLe( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) <= b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onNe( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) != b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onEq( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) == b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onLand( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) && b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

Object *Executor::onLor( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = (*a) || b;

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

    return c;
}

