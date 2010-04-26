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


Engine::Engine( VM *context ) :
    vmachine(context),
    vm(&context->vmem),
    vc(&context->vcode),
    vt(&context->vtypes) {
}

Engine::~Engine(){

}

Object *Engine::exec( vframe_t *frame, Node *node ){
    /*
	 * An exception has been thrown, wait for a try-catch statement or,
	 * when the frame will be deleted (vframe_t class destructor), exit
	 * with a non handled exception.
	 */
	if( frame->state._exception ){
		return frame->state.value;
	}
    /*
	 * A return statement was succesfully executed, skip everything
	 * now on until the frame will be destroyed and return appropriate
	 * value.
	 */
	else if( frame->state._return ){
		return frame->state.value;
	}
    /*
     * A next statement was found, so skip nodes execution
     * until one of the loop handlers will reset the flag.
     */
    else if( frame->state._next ){
    	return H_DEFAULT_RETURN;
    }

    switch( node->type() ){
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
            return onNewOperator( frame, node );
        /* function call */
        case H_NT_CALL       :
            return onFunctionCall( frame, node );
        case H_NT_METHOD_CALL :
        	return onMethodCall( frame, node );
        /* struct type declaration */
        case H_NT_STRUCT :
            return onStructureDeclaration( frame, node );
        /* class type declaration */
        case H_NT_CLASS :
			return onClassDeclaration( frame, node );

		/* statements */
        case H_NT_STATEMENT :
        	/*
        	 * Call the garbage collection routine every new statement.
        	 * If the routine would be called on expressions too, there would be a high
        	 * risk of loosing tmp values such as evaluations, ecc.
        	 */
        	gc_collect();

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
                    return onForeachMapping( frame, node );
				/* break; */
				case T_BREAK :
					frame->state._break = true;
				break;
				/* next; */
				case T_NEXT :
					frame->state._next = true;
				break;
				/* return */
				case T_RETURN :
					return onReturn( frame, node );
                /* (condition ? expression : expression) */
                case T_QUESTION :
                    return onQuestion( frame, node );
				/* switch statement */
                case T_SWITCH :
                    return onSwitch( frame, node );
                /* throw expression; */
                case T_THROW :
					return onThrow( frame, node );
				/* try-catch statement */
                case T_TRY :
                	return onTryCatch( frame, node );
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
                    return onInplaceDot( frame, node );
                /* -expression */
                case T_UMINUS :
                    return onUminus( frame, node );
                /* expression ~= expression */
                case T_REGEX_OP :
                    return onRegex( frame, node );
                /* expression + expression */
                case T_PLUS    :
                    return onAdd( frame, node );
                /* expression += expression */
                case T_PLUSE   :
                    return onInplaceAdd( frame, node );
                /* expression - expression */
                case T_MINUS    :
                    return onSub( frame, node );
                /* expression -= expression */
                case T_MINUSE   :
                    return onInplaceSub( frame, node );
                /* expression * expression */
                case T_MUL	:
                    return onMul( frame, node );
                /* expression *= expression */
                case T_MULE	:
                    return onInplaceMul( frame, node );
                /* expression / expression */
                case T_DIV    :
                    return onDiv( frame, node );
                /* expression /= expression */
                case T_DIVE   :
                    return onInplaceDiv( frame, node );
                /* expression % expression */
                case T_MOD    :
                    return onMod( frame, node );
                /* expression %= expression */
                case T_MODE   :
                    return onInplaceMod( frame, node );
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
                    return onInplaceXor( frame, node );
                /* expression & expression */
                case T_AND    :
                    return onAnd( frame, node );
                /* expression &= expression */
                case T_ANDE   :
                    return onInplaceAnd( frame, node );
                /* expression | expression */
                case T_OR     :
                    return onOr( frame, node );
                /* expression |= expression */
                case T_ORE    :
                    return onInplaceOr( frame, node );
                /* expression << expression */
                case T_SHIFTL  :
                    return onShiftl( frame, node );
                /* expression <<= expression */
                case T_SHIFTLE :
                    return onInplaceShiftl( frame, node );
                /* expression >> expression */
                case T_SHIFTR  :
                    return onShiftr( frame, node );
                /* expression >>= expression */
                case T_SHIFTRE :
                    return onInplaceShiftr( frame, node );
                /* expression! */
                case T_FACT :
                    return onFact( frame, node );
                /* ~expression */
                case T_NOT    :
                    return onNot( frame, node );
                /* !expression */
                case T_L_NOT   :
                    return onLnot( frame, node );
                /* expression < expression */
                case T_LESS    :
                    return onLess( frame, node );
                /* expression > expression */
                case T_GREATER    :
                    return onGreater( frame, node );
                /* expression >= expression */
                case T_GREATER_EQ     :
                    return onGe( frame, node );
                /* expression <= expression */
                case T_LESS_EQ     :
                    return onLe( frame, node );
                /* expression != expression */
                case T_NOT_SAME     :
                    return onNe( frame, node );
                /* expression == expression */
                case T_SAME     :
                    return onEq( frame, node );
                /* expression && expression */
                case T_L_AND   :
                    return onLand( frame, node );
                /* expression || expression */
                case T_L_OR    :
                    return onLor( frame, node );
            }
    }

    return H_DEFAULT_RETURN;
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
		hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
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
        hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
    }
	/*
	 * Ok, definetly it's defined "somewhere", but we don't know exactly where.
	 * That could be the user types definition segments, who knows (the gc should),
	 * anyway, check if it's REALLY a user defined type.
	 */
    else if( ob_is_struct(owner) == false && ob_is_class(owner) == false ){
        hyb_error( H_ET_SYNTAX, "'%s' does not name a structure nor a class", identifier );
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
       		hyb_error( H_ET_SYNTAX, "'%s' is not a member of %s", child_id, owner_id );
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
        		access_t access   = ob_attribute_access( owner, child_id );

        		if( access != asPublic ){
					/*
					 * The attribute is protected.
					 */
					if( access == asProtected ){
						/*
						 * Protected attributes can be accessed only by derived classes.
						 */
						if( strcmp( owner_id, "me" ) != 0 ){
							hyb_error( H_ET_SYNTAX, "Protected attribute '%s' can be accessed only by derived classes of '%s'", child_id, cobj->name.c_str() );
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
							hyb_error( H_ET_SYNTAX, "Private attribute '%s' can be accessed only within '%s' class", child_id, cobj->name.c_str() );
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
	 * syntax tree and finds out a constant.
	 * For this reason, constants do NOT go out of scope, out
	 * of references, out of fuel, out-what-u-like-most.
	 * If the garbage collector detects an object to be a constant,
	 * (the object->attributes bitmask is set to H_OT_CONSTANT), it
	 * simply skips it in the loop.
	 */
    return node->value.m_constant;
}

Object *Engine::onFunctionDeclaration( vframe_t *frame, Node *node ){
    char *function_name = (char *)node->value.m_function.c_str();

    /* check for double definition */
    if( vc->get(function_name) != H_UNDEFINED ){
        hyb_error( H_ET_SYNTAX, "function '%s' already defined", function_name );
    }
    else if( vmachine->getFunction( function_name ) != H_UNDEFINED ){
        hyb_error( H_ET_SYNTAX, "function '%s' already defined as a language function", function_name );
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
    vmachine->defineType( s_name, s );

    return H_UNDEFINED;
}

Object *Engine::onClassDeclaration( vframe_t *frame, Node *node ){
	int        i, j, members( node->children() );
	char      *classname = (char *)node->value.m_identifier.c_str(),
			  *attrname;
	/* class prototypes are not garbage collected */
	Object    *c         = (Object *)(new ClassObject());

	for( i = 0; i < members; ++i ){
		/*
		 * Define an attribute
		 */
		if( node->child(i)->type() == H_NT_IDENTIFIER ){
			attrname = (char *)node->child(i)->value.m_identifier.c_str();

			ob_add_attribute( c, attrname );
			ob_set_attribute_access( c, attrname, node->child(i)->value.m_access );
		}
		/*
		 * Define a method
		 */
		else if( node->child(i)->type() == H_NT_METHOD ){
			ob_define_method( c, (char *)node->child(i)->value.m_method.c_str(), node->child(i) );
		}
		else{
			hyb_error( H_ET_GENERIC, "unexpected node type for class declaration" );
		}
	}
	/*
	 * Check if the class extends some other class.
	 */
	ClassNode *cnode = (ClassNode *)node;
	if( cnode->m_extends.size() > 0 ){
		for( NodeList::iterator ni = cnode->m_extends.begin(); ni != cnode->m_extends.end(); ni++ ){
			Object *type = vmachine->getType( (char *)(*ni)->value.m_identifier.c_str() );
			if( type == H_UNDEFINED ){
				hyb_error( H_ET_SYNTAX, "'%s' undeclared class type", (*ni)->value.m_identifier.c_str() );
			}
			else if( ob_is_class(type) == false ){
				hyb_error( H_ET_SYNTAX, "couldn't extend from '%s' type", type->type->name );
			}

			ClassObject 			   		   *cobj = ob_class_ucast(type);
			ClassObjectAttributeIterator 		ai;
			ClassObjectMethodIterator 	 		mi;
			ClassObjectMethodVariationsIterator mvi;

			for( ai = cobj->c_attributes.begin(); ai != cobj->c_attributes.end(); ai++ ){
				attrname = (char *)(*ai)->label.c_str();

				ob_set_attribute( c, attrname, (*ai)->value->value );
				ob_set_attribute_access( c, attrname, (*ai)->value->access );
			}

			for( mi = cobj->c_methods.begin(); mi != cobj->c_methods.end(); mi++ ){
				for( mvi = (*mi)->value->method.begin(); mvi != (*mi)->value->method.end(); mvi++ ){
					ob_define_method( c, (char *)(*mi)->label.c_str(), *mvi );
				}
			}
		}
	}

	/*
	 * ::defineType will take care of the class attributes
	 * to prevent it to be garbage collected (see ::onConstant).
	 */
	vmachine->defineType( classname, c );
}

Object *Engine::onBuiltinFunctionCall( vframe_t *frame, Node * call ){
    char        *callname = (char *)call->value.m_call.c_str();
    function_t   function;
    vframe_t     stack;
    unsigned int i(0),
                 children( call->children() );
    Object      *value  = H_UNDEFINED,
                *result = H_UNDEFINED;

    if( (function = vmachine->getFunction( callname )) == H_UNDEFINED ){
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

    /*
	 * An exception has been thrown inside an exec call.
	 */
	if( frame->state._exception ){
		return frame->state.value;
	}

    vmachine->trace( callname, &stack );
    vmachine->setCurrentFrame( &stack );
    /* call the function */
    result = function( vmachine, &stack );
    vmachine->setCurrentFrame( frame );
    vmachine->detrace();

    /* return function evaluation value */
    return result;
}

Object *Engine::onThreadedCall( string function_name, vframe_t *frame, vmem_t *argv ){
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
		vmachine->lock();
		hyb_error( H_ET_SYNTAX, "'%s' undeclared user function identifier", function_name.c_str() );
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
	   vmachine->depool();
	   vmachine->lock();
	   hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
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

	vmachine->trace( (char *)function_name.c_str(), &stack );
	vmachine->setCurrentFrame( &stack );

	/* call the function */
	result = exec( &stack, body );

	vmachine->setCurrentFrame( frame );
	vmachine->detrace();

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
           vmachine->depool();
        }
        hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
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

    vmachine->trace( callname, &stack );
    vmachine->setCurrentFrame( &stack );

    /* call the function */
    result = exec( &stack, body );

    vmachine->setCurrentFrame( frame );
    vmachine->detrace();

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state._exception == true ){
		stack.state._exception  = false;
		frame->state._exception = true;
		frame->state.value = stack.state.value;
	}

    /* return function evaluation value */
    return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

Object *Engine::onNewOperator( vframe_t *frame, Node *type ){
    char      *type_name = (char *)type->value.m_identifier.c_str();
    Object    *user_type = H_UNDEFINED,
              *newtype   = H_UNDEFINED,
              *object    = H_UNDEFINED;
    int        i, children( type->children() );

	/*
	 * Search for the used defined type calls, most like C++
	 * class constructors but without strict prototypes.
	 */ 
    if( (user_type = vmachine->getType(type_name)) == H_UNDEFINED ){
    	hyb_error( H_ET_SYNTAX, "'%s' undeclared type", type_name );
    }
    newtype = ob_clone(user_type);

	/*
	 * It's ok to initialize less attributes that the structure/class
	 * has (non ini'ed attributes are set to 0 by default), but
	 * you can not set more attributes than the structure/class have.
	 */
	if( ob_is_struct(newtype) ){
		if( children > ob_struct_ucast(newtype)->items ){
			hyb_error( H_ET_SYNTAX, "structure '%s' has %d attributes, initialized with %d",
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
				hyb_error( H_ET_SYNTAX, "class '%s' constructor requires %d arguments, called with %d",
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

			vmachine->trace( type_name, &stack );
			vmachine->setCurrentFrame( &stack );

			/* call the ctor */
			exec( &stack, ctor->callBody() );

			vmachine->setCurrentFrame( frame );
			vmachine->detrace();

			/*
			 * Check for unhandled exceptions and put them on the root
			 * memory frame.
			 */
			if( stack.state._exception == true ){
				stack.state._exception  = false;
				frame->state._exception = true;
				frame->state.value = stack.state.value;
			}
		}
		else{
			if( children > ob_class_ucast(newtype)->c_attributes.size() ){
				hyb_error( H_ET_SYNTAX, "class '%s' has %d attributes, initialized with %d",
									 type_name,
									 ob_class_ucast(newtype)->c_attributes.size(),
									 children );
			}

			for( i = 0; i < children; ++i ){
				object = exec( frame, type->child(i) );
				ob_set_attribute( newtype, (char *)ob_class_ucast(newtype)->c_attributes.at(i)->name.c_str(), object );
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
    function_t dllcall = vmachine->getFunction( "dllcall" );

    if( (fn_pointer = frame->get( callname )) == H_UNDEFINED ){
        if( threaded ){
            vmachine->depool();
        }
        return H_UNDEFINED;
    }
    else if( ob_is_extern(fn_pointer) == false ){
        if( threaded ){
            vmachine->depool();
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

    vmachine->trace( callname, &stack );
    vmachine->setCurrentFrame( &stack );

    /* call the function */
    result = dllcall( vmachine, &stack );

    vmachine->setCurrentFrame( frame );
    vmachine->detrace();

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
		hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", owner_id.c_str() );
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
		if( (method = ob_get_method( owner, (char *)child_id.c_str(), argc )) != H_UNDEFINED ){
			/*
			 * The last child of a method is its body itself, so we compare
			 * call children with method->children() - 1 to ignore the body.
			 */
			method_argc = method->children() - 1;
			break;
		}
		/*
		 * Update owner-child relationship and continue the loop.
		 */
		else if( (child = ob_get_attribute( owner, (char *)child_id.c_str() )) != H_UNDEFINED ){
			owner    = child;
			owner_id = child_id;
		}
		else{
			hyb_error( H_ET_SYNTAX, "'%s' is not a member of '%s'", child_id.c_str(), owner_id.c_str() );
		}
	}
	/*
	 * Nothing found, neither an attribute nor a method!
	 */
	if( method == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' does not name a method neither an attribute of '%s'", child_id.c_str(), owner_id.c_str() );
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
				hyb_error( H_ET_SYNTAX, "Protected method '%s' can be accessed only by derived classes of '%s'", method->value.m_method.c_str(), method_owner.c_str() );
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
					hyb_error( H_ET_SYNTAX, "Private method '%s' can be accessed only within '%s' class", method->value.m_method.c_str(), method_owner.c_str() );
				}
			}
			/*
			 * No way dude, you called a private method!
			 */
			else{
				hyb_error( H_ET_SYNTAX, "Private method '%s' can be accessed only within '%s' class", method->value.m_method.c_str(), method_owner.c_str() );
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
		vmachine->depool();
		hyb_error( H_ET_SYNTAX, "method '%s' requires %d parameters (called with %d)",
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

	vmachine->trace( (char *)child_id.c_str(), &stack );
	vmachine->setCurrentFrame( &stack );

	/* call the method */
	result = exec( &stack, method->callBody() );

	vmachine->setCurrentFrame( frame );
	vmachine->detrace();

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state._exception == true ){
		stack.state._exception  = false;
		frame->state._exception = true;
		frame->state.value = stack.state.value;
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

Object *Engine::onFunctionCall( vframe_t *frame, Node *call, int threaded /*= 0*/ ){
    Object *result = H_UNDEFINED;

    /* check if function is a builtin function */
    if( (result = onBuiltinFunctionCall( frame, call )) != H_UNDEFINED ){
    	return result;
    }
    /* check for an user defined function */
    else if( (result = onUserFunctionCall( frame, call, threaded )) != H_UNDEFINED ){
    	return result;
    }
    /* check if the function is an extern identifier loaded by dll importing routines */
    else if( (result = onDllFunctionCall( frame, call, threaded )) != H_UNDEFINED ){
    	return result;
    }
    /* check for a method */
    else if( (result = onMethodCall( frame, call )) != H_UNDEFINED ){
    	return result;
    }
    else{
    	hyb_error( H_ET_SYNTAX, "'%s' undeclared function identifier", call->value.m_call.c_str() );
    }

    return result;
}

Object *Engine::onDollar( vframe_t *frame, Node *node ){
    Object *o    = H_UNDEFINED,
           *name = H_UNDEFINED;

    o    = exec( frame, node->child(0) );
    name = ob_to_string(o);

    if( (o = frame->get( (char *)ob_string_ucast(name)->value.c_str() )) == H_UNDEFINED ){
        hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", ob_string_ucast(name)->value.c_str() );
    }

    return o;
}

Object *Engine::onReturn( vframe_t *frame, Node *node ){
	/*
	 * Set break and return state to make every loop and/or condition
	 * statement to exit with this return value.
	 */
    frame->state.value   = exec( frame, node->child(0) );
    frame->state._break  = true;
    frame->state._return = true;

    return frame->state.value;
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
		array 	   = exec( frame, node->child(1) );
		identifier = exec( frame, node->child(0) );
		index      = exec( frame, node->child(2) );

		ob_assign( identifier,
					ob_cl_at( array, index )
				  );

		result = identifier;
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

    array = exec( frame, node->child(0) );
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

   		frame->state._next = false;

        if( frame->state._break ){
        	frame->state._break = false;
			break;
        }
    }

    return result;
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

		frame->state._next = false;

        if( frame->state._break ){
			frame->state._break = false;
			break;
		}
    }
    while( ob_lvalue( (boolean = exec(  frame, condition )) ) );

    return result;
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
         (inc   = exec(  frame, increment )) ){

		result = exec( frame, body );

		frame->state._next = false;

        if( frame->state._break ){
        	frame->state._break = false;
			break;
		}
    }

    return result;
}

Object *Engine::onForeach( vframe_t *frame, Node *node ){
    int     size;
    Node   *body;
    Object *v      = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *identifier;
    IntegerObject index(0);

    identifier = (char *)node->child(0)->value.m_identifier.c_str();
    v          = exec( frame, node->child(1) );
    body       = node->child(2);
    size       = ob_get_size(v);

    for( ; index.value < size; ++index.value ){
        frame->add( identifier, ob_cl_at( v, (Object *)&index ) );

        result = exec( frame, body );

        frame->state._next = false;

        if( frame->state._break ){
			frame->state._break = false;
			break;
		}
    }

    return result;
}

Object *Engine::onForeachMapping( vframe_t *frame, Node *node ){
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

        result = exec( frame, body );

        frame->state._next = false;

        if( frame->state._break ){
			frame->state._break = false;
			break;
		}
    }

    return result;
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

Object *Engine::onThrow( vframe_t *frame, Node *node ){
	frame->state.value = exec( frame, node->child(0) );
	frame->state._exception = true;

	return frame->state.value;
}

Object *Engine::onTryCatch( vframe_t *frame, Node *node ){
	Node *main_body    = node->child(0),
		 *ex_ident     = node->child(1),
		 *catch_body   = node->child(2),
		 *finally_body = node->child(3);

	exec( frame, main_body );

	if( frame->state._exception ){
		assert( frame->state.value != H_UNDEFINED );
		frame->add( (char *)ex_ident->value.m_identifier.c_str(), frame->state.value );
		frame->state._exception = false;
		exec( frame, catch_body );
	}

	if( finally_body != NULL ){
		exec( frame, finally_body );
	}

	return H_DEFAULT_RETURN;
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

Object *Engine::onInplaceDot( vframe_t *frame, Node *node ){
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
    		hyb_error( H_ET_SYNTAX, "'me' is a reserved word" );
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
			hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
		}
		/*
		 * Ok, definetly it's defined "somewhere", but we don't know exactly where.
		 * That could be the user types definition segments, who knows (the gc should),
		 * anyway, check if it's REALLY a user defined type.
		 */
		else if( ob_is_struct(owner) == false && ob_is_class(owner) == false ){
			hyb_error( H_ET_SYNTAX, "'%s' does not name a structure nor a class", identifier );
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
				hyb_error( H_ET_SYNTAX, "'%s' is not a member of %s", child_id, owner_id );
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

    o = exec(  frame, node->child(0) );
	regexp = exec(  frame, node->child(1) );
	result = ob_apply_regexp( o, regexp );

	return result;
}

Object *Engine::onAdd( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
	b = exec( frame, node->child(1) );
	c = ob_add( a, b );

	return c;
}

Object *Engine::onInplaceAdd( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = exec( frame, node->child(0) );
	b = exec( frame, node->child(1) );

	ob_inplace_add( a, b );

	return a;
}

Object *Engine::onSub( vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = exec( frame, node->child(0) );
	b = exec( frame, node->child(1) );
	c = ob_sub( a, b );

	return c;
}

Object *Engine::onInplaceSub( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceMul( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceDiv( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceMod( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceXor( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceAnd( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceOr( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceShiftl( vframe_t *frame, Node *node ){
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

Object *Engine::onInplaceShiftr( vframe_t *frame, Node *node ){
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

    o = exec( frame, node->child(0) );
   	r = ob_bw_not(o);

   	return r;
}

Object *Engine::onLnot( vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = exec( frame, node->child(0) );
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
