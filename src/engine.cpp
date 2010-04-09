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
                    return onEostmt( frame, node );
                /* return */
                case T_RETURN :
                    return onReturn( frame, node );
                /* $ */
                case T_DOLLAR :
                    return onDollar( frame, node );
                /* * */
                case T_PTR :
                    return onPointer( frame, node );
                case T_OBJ :
                    return onObject( frame, node );
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
	Object *alias = H_UNDEFINED;
	if( (alias = frame->get( callname )) != H_UNDEFINED && alias->type == H_OT_ALIAS ){
	    strcpy( name, vc->label( alias->value.m_alias ) );
		return vc->at( alias->value.m_alias );
	}
	/* try to evaluate the call as an alias itself */
	if( call->value.m_alias_call != NULL ){
		alias = exec( frame, call->value.m_alias_call );
		if( alias->type == H_OT_ALIAS ){
		    strcpy( name, vc->label( alias->value.m_alias ) );
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

    // search for the identifier on the function frame
    o = frame->get( identifier );
    if( o == H_UNDEFINED && H_ADDRESS_OF(frame) != H_ADDRESS_OF(vm) ){
        // search it on the global frame if it's different from local frame
        o = vm->get( identifier );
    }
    // search for it as a function name
    if( o == H_UNDEFINED ){
        idx = vc->index( identifier );
        if( idx != -1 ){
            o = new Object((unsigned int)idx);
        }
        // identifier not found
        else{
            hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
        }
    }

    return o;
}

Object *Engine::onAttribute( vframe_t *frame, Node *node ){
    Object    *owner = H_UNDEFINED,
              *child = H_UNDEFINED;
    int        i, j, attributes(node->children());
    char      *identifier = (char *)node->value.m_identifier.c_str(),
              *owner_id   = identifier,
              *child_id   = NULL;

    // search for the identifier on the function frame
    owner = frame->get( identifier );
    if( owner == H_UNDEFINED && H_ADDRESS_OF(frame) != H_ADDRESS_OF(vm) ){
        // search it on the global frame if it's different from local frame
        owner = vm->get( identifier );
    }
    if( owner == H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
    }
    else if( owner->type != H_OT_STRUCT ){
        hyb_throw( H_ET_SYNTAX, "'%s' is not a structure identifier", identifier );
    }

    for( i = 0; i < attributes; ++i ){
        child_id = (char *)node->child(i)->value.m_identifier.c_str();
        child    = owner->getAttribute( child_id );

        if( child == H_UNDEFINED ){
            hyb_throw( H_ET_SYNTAX, "'%s' is not an attribute of %s", child_id, owner_id );
        }
        else if( child->type == H_OT_STRUCT ){
            owner    = child;
            owner_id = child_id;
        }
        else{
            return child;
        }
    }
}

Object *Engine::onConstant( vframe_t *frame, Node *node ){
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
    Object    *s      = new Object();

    for( i = 0; i < attributes; ++i ){
        s->addAttribute( (char *)node->child(i)->value.m_identifier.c_str() );
    }

    ctx->defineType( s_name, s );

    return H_UNDEFINED;
}

Object *Engine::onBuiltinFunctionCall( vframe_t *frame, Node * call ){
    char        *callname = (char *)call->value.m_call.c_str();
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
        /* prevent value from being deleted, we'll take care of it */
        if( value != H_UNDEFINED ){
            value->setGarbageAttribute( ~H_OA_GARBAGE );
        }
        stack.insert( HANONYMOUSIDENTIFIER, value );
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = function( ctx, &stack );

    for( i = 0; i < children; i++ ){
        value = stack.at(i);
        if( value != H_UNDEFINED && H_IS_NOT_CONSTANT(value) ){
            delete value;
        }
    }

    ctx->detrace();

    /* return function evaluation value */
    return (result == H_UNDEFINED ? new Object((unsigned int)0) : result);
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
        // this creates a copy of 'value' (fixes: #0000012)
        stack.add( (char *)identifiers[i].c_str(), value );
        /* at this point, the original value is not necessary anymore so
           if it meets the garbage conditions we can free it */
        H_FREE_GARBAGE(value);
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = exec( &stack, function->child(body) );

    for( i = 0; i < children; i++ ){
        value = stack.at(i);
        /* 'value' is obviously defined inside the 'stack', so
           we can just consider null pointer and constant values */
        if( value != H_UNDEFINED && H_IS_NOT_CONSTANT(value) ){
            delete value;
        }
    }

    ctx->detrace();

    /* return function evaluation value */
    return (result == H_UNDEFINED ? new Object((unsigned int)0) : result);
}

Object *Engine::onTypeCall( vframe_t *frame, Node *type ){
    char      *type_name = (char *)type->value.m_call.c_str();
    Object    *user_type = H_UNDEFINED,
              *structure = H_UNDEFINED,
              *object    = H_UNDEFINED;
    int        i, children( type->children() );

    if( (user_type = ctx->getType(type_name)) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    structure = new Object(user_type);

    // init structure attributes
    if( children > 0 ){
        if( children > structure->value.m_struct.size() ){
            hyb_throw( H_ET_SYNTAX, "structure '%s' has %d attributes, initialized with %d",
                                 type_name,
                                 structure->value.m_struct.size(),
                                 children );
        }

        for( i = 0; i < children; ++i ){
            object = exec( frame, type->child(i) );

            structure->setAttribute( (char *)structure->value.m_struct[i].name.c_str(), object );

            H_FREE_GARBAGE(object);
        }
    }

    return structure;
}

Object *Engine::onDllFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    char    *callname         = (char *)call->value.m_call.c_str(),
             identifier[0xFF] = {0};
    vframe_t stack;
    Node    *clone            = H_UNDEFINED;
    Object  *value            = H_UNDEFINED,
            *result           = H_UNDEFINED,
            *fn_pointer       = H_UNDEFINED;
    unsigned int children,
                 i;
    /* we assume that dll module is already loaded */
    function_t dllcall        = ctx->getFunction( "dllcall" );

    if( (fn_pointer = frame->get( callname )) == H_UNDEFINED ){
        if( threaded ){
            ctx->depool();
        }
        return H_UNDEFINED;
    }
    else if( (fn_pointer->attributes & H_OA_EXTERN) != H_OA_EXTERN ){
        if( threaded ){
            ctx->depool();
        }
        return H_UNDEFINED;
    }

    /* at this point we're sure that it's an extern function pointer, so build the frame for hdllcall */
    stack.insert( HANONYMOUSIDENTIFIER, fn_pointer );
    children = call->children();
    for( i = 0; i < children; ++i ){
        clone = call->child(i);
        value = exec( frame, clone );
        /* prevent value from being deleted, we'll take care of it */
        if( value != H_UNDEFINED ){
            value->setGarbageAttribute( ~H_OA_GARBAGE );
        }

        // create a temporary identifier to not overwrite the function pointer itself
        sprintf( identifier, "%s%d", HANONYMOUSIDENTIFIER, i );
        stack.insert( identifier, value );
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = dllcall( ctx, &stack );

    for( i = 0; i < children; i++ ){
        value = stack.at(i);
        if( value != H_UNDEFINED && H_IS_NOT_CONSTANT(value) ){
            delete value;
        }
    }

    ctx->detrace();

    /* return function evaluation value */
    return (result == H_UNDEFINED ? new Object((unsigned int)0) : result);
}

Object *Engine::onFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    Object *result   = H_UNDEFINED;

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
    name = o->toString();

    H_FREE_GARBAGE(o);

    if( (o = frame->get( (char *)name->value.m_string.c_str() )) == H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", (char *)name->value.m_string.c_str() );
    }

    H_FREE_GARBAGE(name);

    return o;
}

Object *Engine::onPointer( vframe_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = exec( frame, node->child(0) );
    res = new Object( (unsigned int)( H_ADDRESS_OF(o) ) );

    H_FREE_GARBAGE(o);

    return res;
}

Object *Engine::onObject( vframe_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = exec( frame, node->child(0) );
    res = o->getObject();

    H_FREE_GARBAGE(o);

    return res;
}

Object *Engine::onReturn( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    return o;
}

Object *Engine::onRange( vframe_t *frame, Node *node ){
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

Object *Engine::onSubscriptAdd( vframe_t *frame, Node *node ){
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

Object *Engine::onSubscriptGet( vframe_t *frame, Node *node ){
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

Object *Engine::onSubscriptSet( vframe_t *frame, Node *node ){
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

Object *Engine::onWhile( vframe_t *frame, Node *node ){
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

Object *Engine::onDo( vframe_t *frame, Node *node ){
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

Object *Engine::onForeach( vframe_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *identifier;

    identifier = (char *)node->child(0)->value.m_identifier.c_str();
    map        = exec( frame, node->child(1) );
    body       = node->child(2);
    size       = map->value.m_array.size();

    for( i = 0; i < size; ++i ){
        frame->add( identifier, map->value.m_array[i] );
        result = exec( frame, body );
        H_FREE_GARBAGE(result);
    }

    H_FREE_GARBAGE(map);

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
    size             = map->value.m_map.size();

    for( i = 0; i < size; ++i ){
        frame->add( key_identifier,   map->value.m_map[i] );
        frame->add( value_identifier, map->value.m_array[i] );
        result = exec(  frame, body );
        H_FREE_GARBAGE(result);
    }

    H_FREE_GARBAGE(map);

    return H_UNDEFINED;
}

Object *Engine::onIf( vframe_t *frame, Node *node ){
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

Object *Engine::onQuestion( vframe_t *frame, Node *node ){
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
            if( target->equals(compare) ){
                H_FREE_GARBAGE(compare);
                H_FREE_GARBAGE(target);
                return exec( frame, stmt_node );
            }
            H_FREE_GARBAGE(compare);
        }
    }

    H_FREE_GARBAGE(target);

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

    H_FREE_GARBAGE(res_1);

    return res_2;
}

Object *Engine::onDot( vframe_t *frame, Node *node ){
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

Object *Engine::onDote( vframe_t *frame, Node *node ){
    Object *a      = H_UNDEFINED,
           *b      = H_UNDEFINED,
           *result = H_UNDEFINED;

    a      = exec( frame, node->child(0) );
    b      = exec( frame, node->child(1) );
    result = a->dotequal( b );

    H_FREE_GARBAGE(a);
    H_FREE_GARBAGE(b);

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

        object->assign(value);
    }

    H_FREE_GARBAGE(value);

    return object;
}

Object *Engine::onUminus( vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = exec( frame, node->child(0) );
    result = -(*o);

    H_FREE_GARBAGE(o);

    return result;
}

Object *Engine::onRegex( vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *regexp = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = exec(  frame, node->child(0) );
    regexp = exec(  frame, node->child(1) );
    result = o->regexp(regexp);

    H_FREE_GARBAGE(o);
    H_FREE_GARBAGE(regexp);

    return result;
}

Object *Engine::onPlus( vframe_t *frame, Node *node ){
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

Object *Engine::onPluse( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) += b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onMinus( vframe_t *frame, Node *node ){
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

Object *Engine::onMinuse( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) -= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onMul( vframe_t *frame, Node *node ){
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

Object *Engine::onMule( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) *= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onDiv( vframe_t *frame, Node *node ){
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

Object *Engine::onDive( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) /= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onMod( vframe_t *frame, Node *node ){
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

Object *Engine::onMode( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) %= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onInc( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = exec( frame, node->child(0) );
    ++(*o);
    return o;
}

Object *Engine::onDec( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = exec( frame, node->child(0) );
    --(*o);
    return o;
}

Object *Engine::onXor( vframe_t *frame, Node *node ){
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

Object *Engine::onXore( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) ^= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onAnd( vframe_t *frame, Node *node ){
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

Object *Engine::onAnde( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) &= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onOr( vframe_t *frame, Node *node ){
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

Object *Engine::onOre( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) |= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onShiftl( vframe_t *frame, Node *node ){
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

Object *Engine::onShiftle( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) <<= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onShiftr( vframe_t *frame, Node *node ){
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

Object *Engine::onShiftre( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
    b = exec( frame, node->child(1) );

    (*a) >>= b;

    H_FREE_GARBAGE(b);

    return a;
}

Object *Engine::onFact( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = o->factorial();

    H_FREE_GARBAGE(o);

    return r;
}

Object *Engine::onNot( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = ~(*o);

    H_FREE_GARBAGE(o);

    return r;
}

Object *Engine::onLnot( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec(  frame, node->child(0) );
    r = o->lnot();

    H_FREE_GARBAGE(o);

    return r;
}

Object *Engine::onLess( vframe_t *frame, Node *node ){
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

Object *Engine::onGreater( vframe_t *frame, Node *node ){
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

Object *Engine::onGe( vframe_t *frame, Node *node ){
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

Object *Engine::onLe( vframe_t *frame, Node *node ){
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

Object *Engine::onNe( vframe_t *frame, Node *node ){
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

Object *Engine::onEq( vframe_t *frame, Node *node ){
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

Object *Engine::onLand( vframe_t *frame, Node *node ){
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

Object *Engine::onLor( vframe_t *frame, Node *node ){
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
