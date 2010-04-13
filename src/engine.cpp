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
#include "engine.h"
#include "parser.hpp"


Engine::Engine( Context *context ) :
    ctx(context),
    vm(&context->vmem),
    vc(&context->vcode),
    vt(&context->vtypes) {
}

Engine::~Engine(){

}

Object *Engine::exec( vframe_t *frame, Node *node ){
    /* skip undefined/null nodes */
    if( node == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    Object *sreturn = H_UNDEFINED;

    switch(node->type()){

        /* identifier */
        case H_NT_IDENTIFIER :
            return onIdentifier( frame, node );
        /* attribute */
        case H_NT_ATTRIBUTE  :
            return onAttribute( frame, node );
        /* constant value */
        case H_NT_CONSTANT   :
            return onConstant( frame, node );
        /* function definition */
        case H_NT_FUNCTION   :
            return onFunctionDeclaration( frame, node );
        /* function call */
        case H_NT_CALL       :
            return onFunctionCall( frame, node );
        /* struct type declaration */
        case H_NT_STRUCT :
            return onStructureDeclaration( frame, node );

        /* statements */
        case H_NT_STATEMENT :
            switch( node->value.m_statement ){
                /* if( condition ) */
                case T_IF     :
                    return onIf( frame, node );
                /* while( condition ){ body } */
                case T_WHILE  :
                    return onWhile( frame, node );
                /* do{ body }while( condition ); */
                case T_DO  :
                    return onDo( frame, node );
                /* for( initialization; condition; variance ){ body } */
                case T_FOR    :
                    return onFor( frame, node );
                /* foreach( item of array ) */
                case T_FOREACH :
                    return onForeach( frame, node );
                /* foreach( label -> item of map ) */
                case T_FOREACHM :
                    return onForeachm( frame, node );
                break;
                /* (condition ? expression : expression) */
                case T_QUESTION :
                    return onQuestion( frame, node );

                case T_SWITCH :
                    return onSwitch( frame, node );
            }
        break;

        /* expressions */
        case H_NT_EXPRESSION   :
            switch( node->value.m_expression ){
                /* identifier = expression */
                case T_ASSIGN    :
                    return onAssign( frame, node );
                /* expression ; */
                case T_EOSTMT  :
                    sreturn = onEostmt( frame, node );
                    /********************************************
                     *          Do garbage collection.
                     *********************************************/
                    gc_collect();
                    return sreturn;
                /* return */
                case T_RETURN :
                    return onReturn( frame, node );
                /* $ */
                case T_DOLLAR :
                    return onDollar( frame, node );
                /* * */
                case T_PTR :
                    return onPointer( frame, node );
                /** TODO case T_OBJ :
                    return onObject( frame, node );
                **/
                /* expression .. expression */
                case T_RANGE :
                    return onRange( frame, node );
                /* array[] = object; */
                case T_SUBSCRIPTADD :
                    return onSubscriptAdd( frame, node );
                /* (identifier)? = object[ expression ]; */
                case T_SUBSCRIPTGET :
                    return onSubscriptGet( frame, node );
                /* object[ expression ] = expression */
                case T_SUBSCRIPTSET :
                    return onSubscriptSet( frame, node );
                /* expression.expression */
                case T_DOT    :
                    return onDot( frame, node );
                /* expression .= expression */
                case T_DOTE   :
                    return onDote( frame, node );
                /* -expression */
                case T_UMINUS :
                    return onUminus( frame, node );
                /* expression ~= expression */
                case T_REGEX_OP :
                    return onRegex( frame, node );
                /* expression + expression */
                case T_PLUS    :
                    return onPlus( frame, node );
                /* expression += expression */
                case T_PLUSE   :
                    return onPluse( frame, node );
                /* expression - expression */
                case T_MINUS    :
                    return onMinus( frame, node );
                /* expression -= expression */
                case T_MINUSE   :
                    return onMinuse( frame, node );
                /* expression * expression */
                case T_MUL	:
                    return onMul( frame, node );
                /* expression *= expression */
                case T_MULE	:
                    return onMule( frame, node );
                /* expression / expression */
                case T_DIV    :
                    return onDiv( frame, node );
                /* expression /= expression */
                case T_DIVE   :
                    return onDive( frame, node );
                /* expression % expression */
                case T_MOD    :
                    return onMod( frame, node );
                /* expression %= expression */
                case T_MODE   :
                    return onMode( frame, node );
                /* expression++ */
                case T_INC    :
                    return onInc( frame, node );
                /* expression-- */
                case T_DEC    :
                    return onDec( frame, node );
                /* expression ^ expression */
                case T_XOR    :
                    return onXor( frame, node );
                /* expression ^= expression */
                case T_XORE   :
                    return onXore( frame, node );
                /* expression & expression */
                case T_AND    :
                    return onAnd( frame, node );
                /* expression &= expression */
                case T_ANDE   :
                    return onAnde( frame, node );
                /* expression | expression */
                case T_OR     :
                    return onOr( frame, node );
                /* expression |= expression */
                case T_ORE    :
                    return onOre( frame, node );
                /* expression << expression */
                case T_SHIFTL  :
                    return onShiftl( frame, node );
                /* expression <<= expression */
                case T_SHIFTLE :
                    return onShiftle( frame, node );
                /* expression >> expression */
                case T_SHIFTR  :
                    return onShiftr( frame, node );
                /* expression >>= expression */
                case T_SHIFTRE :
                    return onShiftre( frame, node );
                /* expression! */
                case T_FACT :
                    return onFact( frame, node );
                /* ~expression */
                case T_NOT    :
                    return onNot( frame, node );
                /* !expression */
                case T_LNOT   :
                    return onLnot( frame, node );
                /* expression < expression */
                case T_LESS    :
                    return onLess( frame, node );
                /* expression > expression */
                case T_GREATER    :
                    return onGreater( frame, node );
                /* expression >= expression */
                case T_GE     :
                    return onGe( frame, node );
                /* expression <= expression */
                case T_LE     :
                    return onLe( frame, node );
                /* expression != expression */
                case T_NE     :
                    return onNe( frame, node );
                /* expression == expression */
                case T_EQ     :
                    return onEq( frame, node );
                /* expression && expression */
                case T_LAND   :
                    return onLand( frame, node );
                /* expression || expression */
                case T_LOR    :
                    return onLor( frame, node );
            }
    }

    return H_UNDEFINED;
}

Node * Engine::findEntryPoint( vframe_t *frame, Node *call, char *name ){
    char *callname = (char *)call->value.m_call.c_str();

    /* search first in the code segment */
	Node *function = H_UNDEFINED;
	if( (function = vc->get(callname)) != H_UNDEFINED ){
	    strcpy( name, callname );
		return function;
	}
	/* then search for a function alias */
	AliasObject *alias = ALIAS_UPCAST( frame->get( callname ) );
	if( alias != H_UNDEFINED && IS_ALIAS_TYPE(alias) ){
	    strcpy( name, vc->label( alias->value ) );
		return vc->at( alias->value );
	}
	/* try to evaluate the call as an alias itself */
	if( call->value.m_alias_call != NULL ){
		alias = (AliasObject *)exec( frame, call->value.m_alias_call );
		if( IS_ALIAS_TYPE(alias) ){
		    strcpy( name, vc->label( alias->value ) );
			return vc->find( name );
		}
	}
	/* function is not defined */
	return H_UNDEFINED;
}

Object *Engine::onIdentifier( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    int     idx;
    char   *identifier = (char *)node->value.m_identifier.c_str();

	/*
	 * First thing first, search for the identifier definition on
	 * the function local stack frame.
	 */
	if( (o = frame->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Let's check if the address of this frame si different from
	 * global frame one, in that case try to search the definition
	 * on the global frame too.
	 */
	else if( H_ADDRESS_OF(frame) != H_ADDRESS_OF(vm) && (o = vm->get( identifier )) != H_UNDEFINED ){
		return o;
	}
	/*
	 * So, it's neither defined on local frame nor in the global one,
	 * let's search for it in the code frame.
	 */
	else if( (idx = vc->index( identifier )) != -1 ){
		/*
		 * Create an alias to that code region (basically its index).
		 */
		return OB_DOWNCAST( MK_ALIAS_OBJ(idx) );
	}
	/*
	 * Ok ok, got it! It's undefined, raise an error.
	 */
	else{
		hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
	}
}

Object *Engine::onAttribute( vframe_t *frame, Node *node ){
    Object    *owner = H_UNDEFINED,
              *child = H_UNDEFINED;
    int        i, j, attributes(node->children());
    char      *identifier = (char *)node->value.m_identifier.c_str(),
              *owner_id   = identifier,
              *child_id   = NULL;

    /*
	 * Search for the identifier on the function frame.
	 */
    owner = frame->get( identifier );
    if( owner == H_UNDEFINED && H_ADDRESS_OF(frame) != H_ADDRESS_OF(vm) ){
        /*
		 * Search it on the global frame if it's different from local frame.
		 */
        owner = vm->get( identifier );
    }
	/*
	 * Nope :( not defined anywhere.
	 */
    else if( owner == H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
    }
	/*
	 * Ok, definetly it's defined "somewhere", but we don't know exactly where.
	 * That could be the user types definition segments, who knows (the gc should),
	 * anyway, check if it's REALLY a user defined type.
	 */
    else if( IS_STRUCT_TYPE(owner) == false ){
        hyb_throw( H_ET_SYNTAX, "'%s' is not a structure identifier", identifier );
    }
	/* 
	 * Loop each element of the chain :
	 * 
	 * foo->bar->moo->...->X
	 * 
	 * Until last element 'X' of this chain is found.
	 */
    for( i = 0; i < attributes; ++i ){
        child_id = (char *)node->child(i)->value.m_identifier.c_str();
        child    = ob_get_attribute( owner, child_id );
		/*
		 * Something went wrong dude!
		 */
        if( child == H_UNDEFINED ){
            hyb_throw( H_ET_SYNTAX, "'%s' is not an attribute of %s", child_id, owner_id );
        }
		/*
		 * Ok, let's consider the next element and shift
		 * relationships between owner and child pointers.
		 */
        else if( IS_STRUCT_TYPE(child) ){
            owner    = child;
            owner_id = child_id;
        }
		/*
		 * Hello Mr. 'X', you are the last element of the chain!
		 */
        else{
            return child;
        }
    }
}

Object *Engine::onConstant( vframe_t *frame, Node *node ){
	/*
	 * Constants are obviously not evaluated every time, just
	 * the first time when the parser is traveling around the
	 * syntaxy tree and founds out a constant.
	 * For this reason, constants do NOT go out of scope, out
	 * of references, out of fuel, out-what-u-like-most.
	 * If the garbage collector detects an object to be a constant,
	 * (the object->attributes bitmask is set to H_OT_CONSTANT), it
	 * simply skip it in the loop.
	 */
    return node->value.m_constant;
}

Object *Engine::onFunctionDeclaration( vframe_t *frame, Node *node ){
    char *function_name = (char *)node->value.m_function.c_str();

    /* check for double definition */
    if( vc->get(function_name) != H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "function '%s' already defined", function_name );
    }
    else if( ctx->getFunction( function_name ) != H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "function '%s' already defined as a language function", function_name );
    }
    /* add the function to the code segment */
    vc->add( function_name, node );

    return H_UNDEFINED;
}

Object *Engine::onStructureDeclaration( vframe_t *frame, Node * node ){
    int        i, attributes( node->children() );
    char      *s_name = (char *)node->value.m_identifier.c_str();
    Object    *s      = (Object *)MK_STRUCT_OBJ();

    for( i = 0; i < attributes; ++i ){
        ob_add_attribute( s, (char *)node->child(i)->value.m_identifier.c_str() );
    }
    /*
     * ::defineType will take care of the structure attributes 
	 * to prevent it to be garbage collected (see ::onConstant).
     */
    ctx->defineType( s_name, s );

    return H_UNDEFINED;
}

Object *Engine::onBuiltinFunctionCall( vframe_t *frame, Node * call ){
    char        *callname = (char *)call->value.m_call.c_str(),
    		     identifier[0xFF] = {0};
    function_t   function;
    Node        *node;
    vframe_t     stack;
    unsigned int i(0),
                 children( call->children() );
    Object      *value  = H_UNDEFINED,
                *result = H_UNDEFINED;

    if( (function = ctx->getFunction( callname )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }
    /* do object assignment */
    for( i = 0; i < children; ++i ){
        /* create function stack value */
        node  = call->child(i);
        value = exec( frame, node );
		/*
		 * Create a temporary identifier to not overwrite the function pointer itself.
		 */
		sprintf( identifier, "%s%d", HANONYMOUSIDENTIFIER, i );
		/*
		 * Value references count is set to zero now, builtins
		 * do not care about reference counting, so this object will
		 * be safely freed after the function call by the gc.
		 */
		stack.insert( identifier, value );
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = function( ctx, &stack );

    ctx->detrace();

    /* return function evaluation value */
    return (result == H_UNDEFINED ? (Object *)MK_INT_OBJ(0) : result);
}

Object *Engine::onUserFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    char    *callname             = (char *)call->value.m_call.c_str(),
             function_name[0xFF]  = {0};
    Node    *function             = H_UNDEFINED,
            *identifier           = H_UNDEFINED;
    vframe_t stack;
    Object  *value                = H_UNDEFINED,
            *result               = H_UNDEFINED;

    vector<string> identifiers;
    unsigned int i, body = 0, children;
    H_NODE_TYPE type;

    if( (function = findEntryPoint( frame, call, function_name )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    /* a function could be without arguments */
    if( function->child(0)->type() == H_NT_IDENTIFIER ){
        identifier = function->child(0);
        children   = function->children();
        type       = identifier->type();

        for( i = 0; type == H_NT_IDENTIFIER && i < children; ++i ){
            identifier = function->child(i);
            identifiers.push_back( identifier->value.m_identifier );
        }

        body = i - 1;
        identifiers.pop_back();
    }

    if( identifiers.size() != call->children() ){
        if( threaded ){
           ctx->depool();
        }
        hyb_throw( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
                             function->value.m_function.c_str(),
                             identifiers.size(),
                             call->children() );
    }

    children = call->children();
    for( i = 0; i < children; ++i ){
        value = exec( frame, call->child(i) );
        /*
         * Value references count is set to zero now, builtins
         * do not care about reference counting, so this object will
         * be safely freed after the function call by the gc.
         */
        stack.insert( (char *)identifiers[i].c_str(), value );
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = exec( &stack, function->child(body) );

    ctx->detrace();

    /* return function evaluation value */
    return (result == H_UNDEFINED ? (Object *)MK_INT_OBJ(0) : result);
}

Object *Engine::onTypeCall( vframe_t *frame, Node *type ){
    char      *type_name = (char *)type->value.m_call.c_str();
    Object    *user_type = H_UNDEFINED,
              *structure = H_UNDEFINED,
              *object    = H_UNDEFINED;
    int        i, children( type->children() );

	/*
	 * Search for the used defined type calls, most like C++
	 * class constructors but without strict prototypes.
	 */ 
    if( (user_type = ctx->getType(type_name)) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    structure = ob_clone(user_type);

    // init structure attributes
    if( children > 0 ){
		/*
		 * It's ok to initialize less attributes that the structure 
		 * has (non ini'ed attributes are set to 0 by default), but
		 * you can not set more attributes than the structure have.
		 */
        if( children > STRUCT_UPCAST(structure)->items ){
            hyb_throw( H_ET_SYNTAX, "structure '%s' has %d attributes, initialized with %d",
                                 type_name,
                                 STRUCT_UPCAST(structure)->items,
                                 children );
        }

        for( i = 0; i < children; ++i ){
            object = exec( frame, type->child(i) );

            ob_set_attribute( structure, (char *)STRUCT_UPCAST(structure)->names[i].c_str(), object );
        }
    }

    return structure;
}

Object *Engine::onDllFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    char    *callname         = (char *)call->value.m_call.c_str(),
             identifier[0xFF] = {0};
    vframe_t stack;
    Object  *value            = H_UNDEFINED,
            *result           = H_UNDEFINED,
            *fn_pointer       = H_UNDEFINED;
    unsigned int children,
                 i;
    /*
     * We assume that dll module is already loaded, otherwise there shouldn't be
     * any onDllFunctionCall call .
     */
    function_t dllcall = ctx->getFunction( "dllcall" );

    if( (fn_pointer = frame->get( callname )) == H_UNDEFINED ){
        if( threaded ){
            ctx->depool();
        }
        return H_UNDEFINED;
    }
    else if( IS_EXTERN_TYPE(fn_pointer) == false ){
        if( threaded ){
            ctx->depool();
        }
        return H_UNDEFINED;
    }

    /* at this point we're sure that it's an extern function pointer, so build the frame for hdllcall */
    stack.insert( HANONYMOUSIDENTIFIER, fn_pointer );
    children = call->children();
    for( i = 0; i < children; ++i ){
        value = exec( frame, call->child(i) );
        /*
         * Create a temporary identifier to not overwrite the function pointer itself.
         */
        sprintf( identifier, "%s%d", HANONYMOUSIDENTIFIER, i );
        /*
         * Value references count is set to zero now, builtins
         * do not care about reference counting, so this object will
         * be safely freed after the function call by the gc.
         */
        stack.insert( identifier, value );
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = dllcall( ctx, &stack );

    ctx->detrace();

    /* return function evaluation value */
    return (result == H_UNDEFINED ? (Object *)MK_INT_OBJ(0) : result);
}

Object *Engine::onFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    Object *result = H_UNDEFINED;

    /* check if function is a function */
    if( (result = onBuiltinFunctionCall( frame, call )) == H_UNDEFINED ){
        /* check for an user defined function */
        if( (result = onUserFunctionCall( frame, call, threaded )) == H_UNDEFINED ){
            /* check for a structure constructor */
            if( (result = onTypeCall( frame, call )) == H_UNDEFINED ){
                /* finally check if the function is an extern identifier loaded by dll importing routines */
                if( (result = onDllFunctionCall( frame, call, threaded )) == H_UNDEFINED ){
                    hyb_throw( H_ET_SYNTAX, "'%s' undeclared function identifier", call->value.m_call.c_str() );
                }
            }
        }
    }

    return result;
}

Object *Engine::onDollar( vframe_t *frame, Node *node ){
    Object *o    = H_UNDEFINED,
           *name = H_UNDEFINED;

    o    = exec( frame, node->child(0) );
    name = ob_to_string(o);

    if( (o = frame->get( (char *)STRING_UPCAST(name)->value.c_str() )) == H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", STRING_UPCAST(name)->value.c_str() );
    }

    return o;
}

Object *Engine::onPointer( vframe_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = exec( frame, node->child(0) );
    res = (Object *)PTR_TO_INT_OBJ(o);

    return res;
}
/**
TODO:
Object *Engine::onObject( vframe_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = exec( frame, node->child(0) );
    res = o->getObject();

    return res;
}
**/
Object *Engine::onReturn( vframe_t *frame, Node *node ){
    return exec(  frame, node->child(0) );
}

Object *Engine::onRange( vframe_t *frame, Node *node ){
    Object *range = H_UNDEFINED,
           *from  = H_UNDEFINED,
           *to    = H_UNDEFINED;

    from  = exec( frame, node->child(0) );
    to    = exec( frame, node->child(1) );
    range = ob_range( from, to );

    return range;
}

Object *Engine::onSubscriptAdd( vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *object = H_UNDEFINED,
           *res    = H_UNDEFINED;

    array  = exec( frame, node->child(0) );
    object = exec( frame, node->child(1) );
    res    = ob_cl_push( array, object );

    return res;
}

Object *Engine::onSubscriptGet( vframe_t *frame, Node *node ){
    Object *identifier = H_UNDEFINED,
           *array      = H_UNDEFINED,
           *index      = H_UNDEFINED,
           *result     = H_UNDEFINED;

    if( node->children() == 3 ){
        identifier    = exec( frame, node->child(0) );
        array         = exec( frame, node->child(1) );
        index         = exec( frame, node->child(2) );

        ob_assign( identifier,
                    ob_cl_at( array, index )
                  );
        result        = identifier;
    }
    else{
        array  = exec( frame, node->child(0) );
        index  = exec( frame, node->child(1) );
        result = ob_cl_at( array, index );
    }

    return result;
}

Object *Engine::onSubscriptSet( vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *index  = H_UNDEFINED,
           *object = H_UNDEFINED;

    array  = exec( frame, node->child(0) );
    index  = exec( frame, node->child(1) );
    object = exec( frame, node->child(2) );

    ob_cl_set( array, index, object );

    return array;
}

Object *Engine::onWhile( vframe_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    condition = node->child(0);
    body      = node->child(1);

    while( ob_lvalue( (boolean = exec(  frame, condition )) ) ){
        result = exec( frame, body );
    }

    return H_UNDEFINED;
}

Object *Engine::onDo( vframe_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    body      = node->child(0);
    condition = node->child(1);
    do{
        result = exec( frame, body );
    }
    while( ob_lvalue( (boolean = exec(  frame, condition )) ) );

    return H_UNDEFINED;
}

Object *Engine::onFor( vframe_t *frame, Node *node ){
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
         ob_lvalue( (boolean = exec(  frame, condition )) );
         (inc     = exec(  frame, increment )) ){
        result = exec( frame, body );
    }

    return H_UNDEFINED;
}

Object *Engine::onForeach( vframe_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *v      = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *identifier;

    identifier = (char *)node->child(0)->value.m_identifier.c_str();
    v          = exec( frame, node->child(1) );
    body       = node->child(2);
    size       = ob_get_size(v);

    for( i = 0; i < size; ++i ){
        frame->add( identifier, VECTOR_UPCAST(v)->value[i] );
        result = exec( frame, body );
    }

    return H_UNDEFINED;
}

Object *Engine::onForeachm( vframe_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *key_identifier,
           *value_identifier;

    key_identifier   = (char *)node->child(0)->value.m_identifier.c_str();
    value_identifier = (char *)node->child(1)->value.m_identifier.c_str();
    map              = exec( frame, node->child(2) );
    body             = node->child(3);
    size             = ob_get_size(map);

    for( i = 0; i < size; ++i ){
        frame->add( key_identifier,   MAP_UPCAST(map)->keys[i] );
        frame->add( value_identifier, MAP_UPCAST(map)->values[i] );
        result = exec(  frame, body );
    }

    return H_UNDEFINED;
}

Object *Engine::onIf( vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = exec(  frame, node->child(0) );

    if( ob_lvalue(boolean) ){
        result = exec( frame, node->child(1) );
    }
    /* handle else case */
    else if( node->children() > 2 ){
        result = exec( frame, node->child(2) );
    }

    return H_UNDEFINED;
}

Object *Engine::onQuestion( vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = exec(  frame, node->child(0) );

    if( ob_lvalue(boolean) ){
        result = exec( frame, node->child(1) );
    }
    else{
        result = exec( frame, node->child(2) );
    }

    return result;
}

Object *Engine::onSwitch( vframe_t *frame, Node *node){
    Node   *case_node = H_UNDEFINED,
           *stmt_node = H_UNDEFINED;
    Object *target    = H_UNDEFINED,
           *compare   = H_UNDEFINED,
           *result    = H_UNDEFINED;
    int     size( node->children() ),
            i;

    target = exec( frame, node->value.m_switch );

    // exec case labels
    for( i = 0; i < size; i += 2 ){
        stmt_node = node->child(i + 1);
        case_node = node->child(i);

        if( case_node != H_UNDEFINED && stmt_node != H_UNDEFINED ){
            compare = exec( frame, case_node );
            if( ob_cmp( target, compare ) == 0 ){
                return exec( frame, stmt_node );
            }
        }
    }

    // exec default case
    if( node->value.m_default != H_UNDEFINED ){
        result = exec( frame, node->value.m_default );
    }

    return result;
}

Object *Engine::onEostmt( vframe_t *frame, Node *node ){
    Object *res_1 = H_UNDEFINED,
           *res_2 = H_UNDEFINED;

    res_1 = exec( frame, node->child(0) );
    res_2 = exec( frame, node->child(1) );

    return res_2;
}

Object *Engine::onDot( vframe_t *frame, Node *node ){
    Object *a      = H_UNDEFINED,
           *b      = H_UNDEFINED,
           *result = H_UNDEFINED;

    a      = exec( frame, node->child(0) );
    b      = exec( frame, node->child(1) );
    result = ob_cl_concat( a, b );

    return result;
}

Object *Engine::onDote( vframe_t *frame, Node *node ){
    Object *a      = H_UNDEFINED,
           *b      = H_UNDEFINED,
           *result = H_UNDEFINED;

    a      = exec( frame, node->child(0) );
    b      = exec( frame, node->child(1) );
    result = ob_cl_inplace_concat( a, b );

    return result;
}

Object *Engine::onAssign( vframe_t *frame, Node *node ){
    Object *object     = H_UNDEFINED,
           *value      = H_UNDEFINED;

    if( node->child(0)->type() != H_NT_ATTRIBUTE ){
        char   *identifier = (char *)node->child(0)->value.m_identifier.c_str();

        value  = exec( frame, node->child(1) );
        object = frame->add( identifier, value );
    }
    else{
        object = exec( frame, node->child(0) );
        value  = exec( frame, node->child(1) );

        ob_assign( object, value );
    }

    return object;
}

Object *Engine::onUminus( vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = exec( frame, node->child(0) );
    result = ob_uminus(o);

    return result;
}

Object *Engine::onRegex( vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *regexp = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = exec(  frame, node->child(0) );
    regexp = exec(  frame, node->child(1) );
    result = ob_apply_regexp( o, regexp );

    return result;
}

Object *Engine::onPlus( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_add( a, b );

    return c;
}

Object *Engine::onPluse( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_inplace_add( a, b );

    return a;
}

Object *Engine::onMinus( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_sub( a, b );

    return c;
}

Object *Engine::onMinuse( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_inplace_sub( a, b );

    return a;
}

Object *Engine::onMul( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_mul( a, b );

    return c;
}

Object *Engine::onMule( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_inplace_mul( a, b );

    return a;
}

Object *Engine::onDiv( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_div( a, b );

    return c;
}

Object *Engine::onDive( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_inplace_div( a, b );

    return a;
}

Object *Engine::onMod( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_mod( a, b );

    return c;
}

Object *Engine::onMode( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_inplace_mod( a, b );

    return a;
}

Object *Engine::onInc( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = exec( frame, node->child(0) );

    ob_increment(o);

    return o;
}

Object *Engine::onDec( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = exec( frame, node->child(0) );

    ob_decrement(o);

    return o;
}

Object *Engine::onXor( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_bw_xor( a, b );

    return c;
}

Object *Engine::onXore( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_bw_inplace_xor( a, b );

    return a;
}

Object *Engine::onAnd( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_bw_and( a, b );

    return c;
}

Object *Engine::onAnde( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_bw_inplace_and( a, b );

    return a;
}

Object *Engine::onOr( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_bw_or( a, b );

    return c;
}

Object *Engine::onOre( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_bw_inplace_or( a, b );

    return a;
}

Object *Engine::onShiftl( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_bw_lshift( a, b );

    return c;
}

Object *Engine::onShiftle( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_bw_inplace_lshift( a, b );

    return a;
}

Object *Engine::onShiftr( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_bw_rshift( a, b );

    return c;
}

Object *Engine::onShiftre( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    ob_bw_inplace_rshift( a, b );

    return a;
}

Object *Engine::onFact( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = ob_factorial(o);

    return r;
}

Object *Engine::onNot( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = ob_bw_not(o);

    return r;
}

Object *Engine::onLnot( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = ob_l_not(o);

    return r;
}

Object *Engine::onLess( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_less( a, b );

    return c;
}

Object *Engine::onGreater( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_greater( a, b );

    return c;
}

Object *Engine::onGe( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_greater_or_same( a, b );

    return c;
}

Object *Engine::onLe( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_less_or_same( a, b );

    return c;
}

Object *Engine::onNe( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_diff( a, b );

    return c;
}

Object *Engine::onEq( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_same( a, b );

    return c;
}

Object *Engine::onLand( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_and( a, b );

    return c;
}

Object *Engine::onLor( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );
    c = ob_l_or( a, b );

    return c;
}
