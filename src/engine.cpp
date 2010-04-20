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
        /* structure or class creation */
        case H_NT_NEW :
            return onNewType( frame, node );
        /* function call */
        case H_NT_CALL       :
            return onFunctionCall( frame, node );
        case H_NT_METHOD_CALL :
        	return onMethodCall( frame, node );

        /* struct type declaration */
        case H_NT_STRUCT :
            return onStructureDeclaration( frame, node );
        case H_NT_CLASS :
			return onClassDeclaration( frame, node );

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

Node * Engine::findEntryPoint( vframe_t *frame, Node *call ){
    char *callname = (char *)call->value.m_call.c_str();

    /* search first in the code segment */
	Node *function = H_UNDEFINED;
	if( (function = vc->get(callname)) != H_UNDEFINED ){
		return function;
	}
	/* then search for a function alias */
	AliasObject *alias = ob_alias_ucast( frame->get( callname ) );
	if( alias != H_UNDEFINED && ob_is_alias(alias) ){
		return (Node *)alias->value;
	}
	/* try to evaluate the call as an alias itself */
	if( call->value.m_alias_call != NULL ){
		alias = (AliasObject *)exec( frame, call->value.m_alias_call );
		if( ob_is_alias(alias) ){
			return (Node *)alias->value;
		}
	}
	/* function is not defined */
	return H_UNDEFINED;
}

Object *Engine::onIdentifier( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    int     idx;
    Node   *function   = H_UNDEFINED;
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
	else if( (function = vc->find( identifier )) != H_UNDEFINED ){
		/*
		 * Create an alias to that code region (basically its index).
		 */
		return ob_dcast( gc_new_alias( H_ADDRESS_OF(function) ) );
	}
	/*
	 * Ok ok, got it! It's undefined, raise an error.
	 */
	else{
		hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
	}
}

Object *Engine::onAttribute( vframe_t *frame, Node *node ){
    Object    *owner  = H_UNDEFINED,
              *child  = H_UNDEFINED;
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
    if( owner == H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
    }
	/*
	 * Ok, definetly it's defined "somewhere", but we don't know exactly where.
	 * That could be the user types definition segments, who knows (the gc should),
	 * anyway, check if it's REALLY a user defined type.
	 */
    else if( ob_is_struct(owner) == false && ob_is_class(owner) == false ){
        hyb_throw( H_ET_SYNTAX, "'%s' does not name a structure nor a class", identifier );
    }
	/* 
	 * Loop each element of the members chain :
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
       		hyb_throw( H_ET_SYNTAX, "'%s' is not a member of %s", child_id, owner_id );
        }
        /*
		 * Ok, let's consider the next element and shift
		 * relationships between owner and child pointers.
		 */
        if( ob_is_struct(child) || ob_is_class(child) ){
            owner    = child;
            owner_id = child_id;
        }
		/*
		 * Hello Mr. 'X', you are the last element of the chain!
		 */
        else{
        	/*
        	 * Check for access specifiers for class attributes.
        	 */
        	if( ob_is_class(owner) ){
        		/*
        		 * Get attribute access descriptor.
        		 */
        		ClassObject *cobj = ob_class_ucast(owner);
        		access_t     access;
        		ClassObjectAccessIterator ai;
				ClassObjectNameIterator   ni;

				for( ni = cobj->a_names.begin(), ai = cobj->a_access.begin();
					 ni != cobj->a_names.end();
					 ni++, ai++ ){
					if( (*ni) == child_id ){
						access = (*ai);
					}
				}


        		if( access != asPublic ){
					/*
					 * The attribute is protected.
					 */
					if( access == asProtected ){
						/*
						 * Protected attributes can be accessed only by derived classes.
						 */
						if( strcmp( owner_id, "me" ) != 0 ){
							hyb_throw( H_ET_SYNTAX, "Protected attribute '%s' can be accessed only by derived classes of '%s'", child_id, cobj->name.c_str() );
						}
					}
					/*
					 * The attribute is private, so only the owner can use it.
					 */
					else if( access == asPrivate ){
						/*
						 * Let's check if the class pointed by 'me' it's the owner of
						 * the private attribute.
						 */
						if( strcmp( owner_id, "me" ) != 0 ){
							hyb_throw( H_ET_SYNTAX, "Private attribute '%s' can be accessed only within '%s' class", child_id, cobj->name.c_str() );
						}
					}
				}
			}
            return child;
        }
    }
    return owner;
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
    Object    *s      = (Object *)gc_new_struct();

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

Object *Engine::onClassDeclaration( vframe_t *frame, Node *node ){
	int        i, j, members( node->children() );
	char      *classname = (char *)node->value.m_identifier.c_str();
	/* class prototypes are not garbage collected */
	Object    *c         = (Object *)(new ClassObject());

	for( i = 0; i < members; ++i ){
		/*
		 * Define an attribute
		 */
		if( node->child(i)->type() == H_NT_IDENTIFIER ){
			ob_add_attribute( c, (char *)node->child(i)->value.m_identifier.c_str() );
			ob_class_ucast(c)->a_access.push_back( node->child(i)->value.m_access );
		}
		/*
		 * Define a method
		 */
		else if( node->child(i)->type() == H_NT_METHOD ){
			ob_define_method( c, (char *)node->child(i)->value.m_method.c_str(), node->child(i) );
		}
		else{
			hyb_throw( H_ET_GENERIC, "unexpected node type for class declaration" );
		}
	}
	/*
	 * Check if the class extends some other class.
	 */
	ClassNode *cnode = (ClassNode *)node;
	if( cnode->m_extends.size() > 0 ){
		for( NodeList::iterator ni = cnode->m_extends.begin(); ni != cnode->m_extends.end(); ni++ ){
			Object *type = ctx->getType( (char *)(*ni)->value.m_identifier.c_str() );
			if( type == H_UNDEFINED ){
				hyb_throw( H_ET_SYNTAX, "'%s' undeclared class type", (*ni)->value.m_identifier.c_str() );
			}
			else if( ob_is_class(type) == false ){
				hyb_throw( H_ET_SYNTAX, "couldn't extend from '%s' type", type->type->name );
			}

			ClassObject 			 *cobj = ob_class_ucast(type);
			ClassObjectAccessIterator ai;
			ClassObjectNameIterator   ni;
			ClassObjectValueIterator  vi;
			ClassObjectMethodIterator mi;

			for( ni = cobj->a_names.begin(), vi = cobj->a_values.begin(), ai = cobj->a_access.begin();
				 ni != cobj->a_names.end();
				 ni++, vi++, ai++ ){
				ob_set_attribute( c, (char *)(*ni).c_str(), (*vi) );
				ob_class_ucast(c)->a_access.push_back( (*ai) );
			}
			for( ni = cobj->m_names.begin(), mi = cobj->m_values.begin(); ni != cobj->m_names.end(); ni++, mi++ ){
				ob_define_method( c, (char *)(*ni).c_str(), (*mi) );
			}
		}
	}

	/*
	 * ::defineType will take care of the class attributes
	 * to prevent it to be garbage collected (see ::onConstant).
	 */
	ctx->defineType( classname, c );
}

Object *Engine::onBuiltinFunctionCall( vframe_t *frame, Node * call ){
    char        *callname = (char *)call->value.m_call.c_str();
    function_t   function;
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
        value = exec( frame, call->child(i) );
		/*
		 * Value references count is set to zero now, builtins
		 * do not care about reference counting, so this object will
		 * be safely freed after the function call by the gc.
		 */
		stack.push( value );
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = function( ctx, &stack );

    ctx->detrace();

    /* return function evaluation value */
    return result;
}

Object *Engine::onThreadedCall( string function_name, vmem_t *argv ){
	Node    *function             = H_UNDEFINED,
			*identifier           = H_UNDEFINED;
	vframe_t stack;
	Object  *value                = H_UNDEFINED,
			*result               = H_UNDEFINED;
	Node    *body                 = H_UNDEFINED;

	vector<string> identifiers;
	unsigned int i(0), children;

	/* search first in the code segment */
	if( (function = vc->get((char *)function_name.c_str())) == H_UNDEFINED ){
		hyb_throw( H_ET_SYNTAX, "'%s' undeclared user function identifier", function_name.c_str() );
	}

	body = function->child(0);
	/* a function could be without arguments */
	if( body->type() == H_NT_IDENTIFIER ){
		children = function->children();
		do{
			identifier = body;
			identifiers.push_back( identifier->value.m_identifier );
			body = function->child(++i);
		}while( body->type() == H_NT_IDENTIFIER && i < children );
	}

	if( identifiers.size() != argv->size() ){
	   ctx->depool();
	   hyb_throw( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
						       function_name.c_str(),
						       identifiers.size(),
						       argv->size() );
	}

	children = argv->size();
	for( i = 0; i < children; ++i  ){
		/*
		 * Value references count is set to zero now, builtins
		 * do not care about reference counting, so this object will
		 * be safely freed after the function call by the gc.
		 */
		stack.insert( (char *)identifiers[i].c_str(), argv->at(i) );
	}

	ctx->trace( (char *)function_name.c_str(), &stack );

	/* call the function */
	result = exec( &stack, body );

	ctx->detrace();

	/* return function evaluation value */
	return result;
}

Object *Engine::onUserFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    char    *callname             = (char *)call->value.m_call.c_str();
    Node    *function             = H_UNDEFINED,
            *identifier           = H_UNDEFINED;
    vframe_t stack;
    Object  *value                = H_UNDEFINED,
            *result               = H_UNDEFINED;
    Node    *body                 = H_UNDEFINED;

    vector<string> identifiers;
    unsigned int i(0), children;

    if( (function = findEntryPoint( frame, call )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    body = function->child(0);
    /* a function could be without arguments */
    if( body->type() == H_NT_IDENTIFIER ){
        children = function->children();
        do{
        	identifier = body;
        	identifiers.push_back( identifier->value.m_identifier );
        	body = function->child(++i);
        }while( body->type() == H_NT_IDENTIFIER && i < children );
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
    result = exec( &stack, body );

    ctx->detrace();

    /* return function evaluation value */
    return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

Object *Engine::onNewType( vframe_t *frame, Node *type ){
    char      *type_name = (char *)type->value.m_identifier.c_str();
    Object    *user_type = H_UNDEFINED,
              *newtype   = H_UNDEFINED,
              *object    = H_UNDEFINED;
    int        i, children( type->children() );

	/*
	 * Search for the used defined type calls, most like C++
	 * class constructors but without strict prototypes.
	 */ 
    if( (user_type = ctx->getType(type_name)) == H_UNDEFINED ){
        return H_UNDEFINED;
    }
    newtype = ob_clone(user_type);

	/*
	 * It's ok to initialize less attributes that the structure/class
	 * has (non ini'ed attributes are set to 0 by default), but
	 * you can not set more attributes than the structure/class have.
	 */
	if( ob_is_struct(newtype) ){
		if( children > ob_struct_ucast(newtype)->items ){
			hyb_throw( H_ET_SYNTAX, "structure '%s' has %d attributes, initialized with %d",
								 type_name,
								 ob_struct_ucast(newtype)->items,
								 children );
		}

		for( i = 0; i < children; ++i ){
			object = exec( frame, type->child(i) );
			ob_set_attribute( newtype, (char *)ob_struct_ucast(newtype)->names[i].c_str(), object );
		}
	}
	else if( ob_is_class(newtype) ){
		/*
		 * Set specific class type name.
		 */
		((ClassObject *)newtype)->name = type_name;
		/*
		 * First of all, check if the user has declared an explicit
		 * class constructor, in that case consider it instead of the
		 * default "by arg" constructor.
		 */
		Node *ctor = ob_get_method( newtype, type_name, children );
		if( ctor != H_UNDEFINED ){
			if( children > ctor->callDefinedArgc() ){
				hyb_throw( H_ET_SYNTAX, "class '%s' constructor requires %d arguments, called with %d",
										 type_name,
										 ctor->callDefinedArgc(),
										 children );
			}

			Node    *arg = H_UNDEFINED;
			vframe_t stack;
			Object  *value = H_UNDEFINED;

			/*
			 * Create the "me" reference to the class itself, used inside
			 * methods for me->... calls.
			 */
			stack.insert( "me", newtype );
			for( i = 0; i < children; ++i ){
				arg   = type->child(i);
				value = exec( frame, arg );
				/*
				 * Value references count is set to zero now, builtins
				 * do not care about reference counting, so this object will
				 * be safely freed after the function call by the gc.
				 */
				stack.insert( (char *)ctor->child(i)->value.m_identifier.c_str(), value );
			}

			ctx->trace( type_name, &stack );

			/* call the ctor */
			exec( &stack, ctor->callBody() );

			ctx->detrace();
		}
		else{
			if( children > ob_class_ucast(newtype)->a_names.size() ){
				hyb_throw( H_ET_SYNTAX, "class '%s' has %d attributes, initialized with %d",
									 type_name,
									 ob_class_ucast(newtype)->a_names.size(),
									 children );
			}

			for( i = 0; i < children; ++i ){
				object = exec( frame, type->child(i) );
				ob_set_attribute( newtype, (char *)ob_class_ucast(newtype)->a_names[i].c_str(), object );
			}
		}
	}

    return newtype;
}

Object *Engine::onDllFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    char    *callname      = (char *)call->value.m_call.c_str();
    vframe_t stack;
    Object  *value         = H_UNDEFINED,
            *result        = H_UNDEFINED,
            *fn_pointer    = H_UNDEFINED;
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
    else if( ob_is_extern(fn_pointer) == false ){
        if( threaded ){
            ctx->depool();
        }
        return H_UNDEFINED;
    }

    /* at this point we're sure that it's an extern function pointer, so build the frame for hdllcall */
    stack.push( fn_pointer );
    children = call->children();
    for( i = 0; i < children; ++i ){
        value = exec( frame, call->child(i) );
        /*
         * Value references count is set to zero now, builtins
         * do not care about reference counting, so this object will
         * be safely freed after the function call by the gc.
         */
        stack.push( value );
    }

    ctx->trace( callname, &stack );

    /* call the function */
    result = dllcall( ctx, &stack );

    ctx->detrace();

    /* return function evaluation value */
    return result;
}

Object *Engine::onMethodCall( vframe_t *frame, Node *call ){
	Object    *owner = H_UNDEFINED,
			  *child = H_UNDEFINED;
	NodeList  method_call( call->value.m_method_call );

	if( method_call.size() == 0 ){
		return H_UNDEFINED;
	}

	Node     *method = H_UNDEFINED;
	int       calls( call->value.m_method_call.size() ),
			  argc( call->children() ),
			  method_argc;
	string    child_id = (*method_call.begin())->value.m_identifier,
			  owner_id = child_id;
	NodeIterator i( method_call.begin() );

	/*
	 * Search for the identifier on the function frame.
	 */
	owner = frame->get( (char *)owner_id.c_str() );
	if( owner == H_UNDEFINED && H_ADDRESS_OF(frame) != H_ADDRESS_OF(vm) ){
		/*
		 * Search it on the global frame if it's different from local frame.
		 */
		owner = vm->get( (char *)owner_id.c_str() );
	}
	/*
	 * Nope :( not defined anywhere.
	 */
	if( owner == H_UNDEFINED ){
		hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", owner_id.c_str() );
	}
	/*
	 * Loop from the second node (first one was used to initialize the first owner
	 * of the chain).
	 */
	for( advance( i, 1 ); i != method_call.end(); i++ ){
		child_id = (*i)->value.m_identifier;
		/*
		 * Try to find out if the child is an attribute or a method.
		 */
		if( (child = ob_get_attribute( owner, (char *)child_id.c_str() )) == H_UNDEFINED ){
			/*
			 * Owner is a class ?
			 */
			if( ob_is_class(owner) ){
				method = ob_get_method( owner, (char *)child_id.c_str(), argc );
				if( method ){
					/*
					 * The last child of a method is its body itself, so we compare
					 * call children with method->children() - 1 to ignore the body.
					 */
					method_argc = method->children() - 1;
				}
				break;
			}
			else{
				hyb_throw( H_ET_SYNTAX, "'%s' is not a member of '%s'", child_id.c_str(), owner_id.c_str() );
			}
		}
		/*
		 * Update owner-child relationship and continue the loop.
		 */
		else{
			owner    = child;
			owner_id = child_id;
		}
	}
	/*
	 * Nothing found, neither an attribute nor a method!
	 */
	if( method == H_UNDEFINED ){
		hyb_throw( H_ET_SYNTAX, "'%s' does not name a method neither an attribute of '%s'", child_id.c_str(), owner_id.c_str() );
	}

	if( method->value.m_access != asPublic ){
		/*
		 * Search for the type name of the class that owns the method.
		 * We're pretty sure that someone owns it, otherwise the method
		 * pointer would be undefined and previous error would be triggered.
		 */
		string method_owner;
		int k, tsz(vt->size() - 1);
		for( k = tsz; k >= 0; --k ){
			if( ob_is_class(vt->at(k)) && ob_get_method( vt->at(k), (char *)method->value.m_method.c_str() ) != H_UNDEFINED ){
				method_owner = vt->label(k);
			}
		}
		/*
		 * The method is protected.
		 */
		if( method->value.m_access == asProtected ){
			/*
			 * Protected methods can be accessed only by derived classes.
			 */
			if( owner_id != "me" ){
				hyb_throw( H_ET_SYNTAX, "Protected method '%s' can be accessed only by derived classes of '%s'", method->value.m_method.c_str(), method_owner.c_str() );
			}
		}
		/*
		 * The method is private, so only the owner can use it.
		 */
		else if( method->value.m_access == asPrivate ){
			/*
			 * Let's check if the class pointed by 'me' it's the owner of
			 * the private method.
			 */
			if( owner_id == "me" ){
				if( ((ClassObject *)owner)->name != method_owner ){
					hyb_throw( H_ET_SYNTAX, "Private method '%s' can be accessed only within '%s' class", method->value.m_method.c_str(), method_owner.c_str() );
				}
			}
			/*
			 * No way dude, you called a private method!
			 */
			else{
				hyb_throw( H_ET_SYNTAX, "Private method '%s' can be accessed only within '%s' class", method->value.m_method.c_str(), method_owner.c_str() );
			}
		}
	}

	Node    *identifier           = H_UNDEFINED;
	vframe_t stack;
	Object  *value                = H_UNDEFINED,
			*result               = H_UNDEFINED;
	unsigned int j;

	/*
	 * The last child of a method is its body itself, so we compare
	 * call children with method->children() - 1 to ignore the body.
	 */
	if( argc != method_argc ){
		ctx->depool();
		hyb_throw( H_ET_SYNTAX, "method '%s' requires %d parameters (called with %d)",
								 method->value.m_method.c_str(),
								 method_argc,
							 	 argc );
	}

	/*
	 * Create the "me" reference to the class itself, used inside
	 * methods for me->... calls.
	 */
	stack.insert( "me", owner );
	for( j = 0; j < argc; ++j ){
		identifier = call->child(j);
		value = exec( frame, identifier );
		/*
		 * Value references count is set to zero now, builtins
		 * do not care about reference counting, so this object will
		 * be safely freed after the method call by the gc.
		 */
		stack.insert( (char *)method->child(j)->value.m_identifier.c_str(), value );
	}

	ctx->trace( (char *)child_id.c_str(), &stack );

	/* call the method */
	result = exec( &stack, method->callBody() );

	ctx->detrace();

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

Object *Engine::onFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    Object *result = H_UNDEFINED;

    /* check if function is a builtin function */
    if( (result = onBuiltinFunctionCall( frame, call )) == H_UNDEFINED ){
        /* check for an user defined function */
        if( (result = onUserFunctionCall( frame, call, threaded )) == H_UNDEFINED ){
			/* check if the function is an extern identifier loaded by dll importing routines */
			if( (result = onDllFunctionCall( frame, call, threaded )) == H_UNDEFINED ){
				/* finally check for a method */
				if( (result = onMethodCall( frame, call )) == H_UNDEFINED ){
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

    if( (o = frame->get( (char *)ob_string_ucast(name)->value.c_str() )) == H_UNDEFINED ){
        hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", ob_string_ucast(name)->value.c_str() );
    }

    return o;
}

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
        frame->add( identifier, ob_vector_ucast(v)->value[i] );
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
        frame->add( key_identifier,   ob_map_ucast(map)->keys[i] );
        frame->add( value_identifier, ob_map_ucast(map)->values[i] );
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
    Object *object = H_UNDEFINED,
           *value  = H_UNDEFINED;
    Node   *lexpr  = node->child(0);

    /*
     * If the first child is an identifier, we are just defining
     * a new variable or assigning it a new value, nothing
     * complicated about it.
     */
    if( lexpr->type() == H_NT_IDENTIFIER ){
    	if( lexpr->value.m_identifier == "me" ){
    		hyb_throw( H_ET_SYNTAX, "'me' is a reserved word" );
    	}

    	value  = exec( frame, node->child(1) );
		object = frame->add( (char *)lexpr->value.m_identifier.c_str(), value );

		return object;
    }
    /*
     * If not, we evaluate the first node as a "owner->child->..." sequence,
     * just like the onAttribute handler.
     */
    else{
    	Object    *owner  = H_UNDEFINED,
				  *child  = H_UNDEFINED;
    	Node      *root   = lexpr;
		int        i, j, attributes(node->children());
		char      *identifier = (char *)root->value.m_identifier.c_str(),
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
		if( owner == H_UNDEFINED ){
			hyb_throw( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
		}
		/*
		 * Ok, definetly it's defined "somewhere", but we don't know exactly where.
		 * That could be the user types definition segments, who knows (the gc should),
		 * anyway, check if it's REALLY a user defined type.
		 */
		else if( ob_is_struct(owner) == false && ob_is_class(owner) == false ){
			hyb_throw( H_ET_SYNTAX, "'%s' does not name a structure nor a class", identifier );
		}
		/*
		 * Loop each element of the members chain :
		 *
		 * foo->bar->moo->...->X
		 *
		 * Until last element 'X' of this chain is found.
		 */
		for( i = 0; i < attributes; ++i ){
			child_id = (char *)root->child(i)->value.m_identifier.c_str();
			child    = ob_get_attribute( owner, child_id );
			/*
			 * Something went wrong dude!
			 */
			if( child == H_UNDEFINED ){
				hyb_throw( H_ET_SYNTAX, "'%s' is not a member of %s", child_id, owner_id );
			}
			/*
			 * Ok, let's consider the next element and shift
			 * relationships between owner and child pointers.
			 */
			if( ob_is_struct(child) || ob_is_class(child) ){
				owner    = child;
				owner_id = child_id;
			}
			/*
			 * Hello Mr. 'X', you are the last element of the chain!
			 */
			else{
				/*
				 * Finally, evaluate the new value for the object and assign it
				 * to the object itself.
				 */
				value = exec( frame, node->child(1) );
				ob_set_attribute_reference( owner, child_id, value );
				return owner;
			}
		}

		return object;
    }
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
