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

engine_t *engine_init( vm_t* vm ){
	engine_t *engine = new engine_t;

	engine->vm    = vm;
	engine->mem   = &vm->vmem;
	engine->cnst  = &vm->vconst;
	engine->code  = &vm->vcode;
	engine->types = &vm->vtypes;

	return engine;
}


__force_inline void engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner,  Object *cobj, int argc, Node *prototype, Node *argv ){
	int 	i, n_ids(prototype->children());
	Object *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( engine->vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;
	/*
	 * Static methods can not use 'me' instance.
	 */
	if( prototype->value.m_static == false ){
		stack.insert( "me", cobj );
	}
	/*
	 * Evaluate each object and insert it into the stack
	 */
	for( i = 0; i < argc; ++i ){
		value = engine_exec( engine, root, argv->child(i) );
		if( root->state.is(Exception) ){
		   return;
		}
		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)prototype->child(i)->value.m_identifier.c_str(), value );
		}
	}
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );
}

__force_inline void engine_prepare_stack( engine_t *engine, vframe_t &stack, string owner, Object *cobj, Node *ids, int argc, ... ){
	va_list ap;
	int i, n_ids(ids->children());
	Object *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( engine->vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	stack.owner = owner;

	stack.insert( "me", cobj );
	/*
	 * Evaluate each object and insert it into the stack
	 */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		value = va_arg( ap, Object * );
		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ids->child(i)->value.m_identifier.c_str(), value );
		}
	}
	va_end(ap);

	vm_add_frame( engine->vm, &stack );
}

__force_inline void engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, vector<string> ids, vmem_t *argv ){
	int 	i, n_ids( ids.size() ), argc;
	Object *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( engine->vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;

	argc = argv->size();
	for( i = 0; i < argc; ++i ){
		value = argv->at(i);
		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ids[i].c_str(), value );
		}
	}

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );
}

__force_inline void engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, vector<string> ids, Node *argv ){
	int 	i, n_ids( ids.size() ), argc;
	Object *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( engine->vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;

	argc = argv->children();
	for( i = 0; i < argc; ++i ){
		value = engine_exec( engine, root, argv->at(i) );
		if( root->state.is(Exception) ){
		   return;
		}
		if( root->state.is(Exception) ){
		   return;
		}
		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ids[i].c_str(), value );
		}
	}

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );
}

__force_inline void engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, ExternObject *fn_pointer, Node *argv ){
	int 	i, argc;
	Object *value;

	/*
	 * Check for heavy recursions and/or nested calss.
	 */
	if( engine->vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;

	stack.push( (Object *)fn_pointer );
	argc = argv->children();
	for( i = 0; i < argc; ++i ){
		value = engine_exec( engine, root, argv->child(i) );
		if( root->state.is(Exception) ){
		   return;
		}
		stack.push( value );
	}

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );
}

__force_inline void engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, Node *argv ){
	int 	i, argc;
	Object *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( engine->vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;
	argc = argv->children();
	for( i = 0; i < argc; ++i ){
		value = engine_exec( engine, root, argv->child(i) );
		if( root->state.is(Exception) ){
		   return;
		}
		stack.push( value );
	}

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );
}

void engine_prepare_stack( engine_t *engine, vframe_t *root, named_function_t *function, vframe_t &stack, string owner, Node *argv ){
	int 	i, argc, f_argc, t;
	Object *value;
	H_OBJECT_TYPE type;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( engine->vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}

	/*
	 * First of all, check that the arguments number is the right one.
	 */
	argc = argv->children();
	for( i = 0 ;; ++i ){
		f_argc = function->argc[i];
		if( f_argc < 0 ){
			break;
		}
		else if( f_argc <= argc ){
			break;
		}
	}
	/*
	 * If f_argc is -1 and i is 0, the first argc descriptor is H_ANY_ARGC so
	 * the function pointer accept any number of arguments, otherwise, if i != 0
	 * and f_argc is -1, we reached the end marker without a match, so the argc
	 * is wrong.
	 */
	if( f_argc == -1 && i != 0 ){
		hyb_error( H_ET_SYNTAX, "Function '%s' requires %s%d argument%s, %d given",
							    function->identifier.c_str(),
							    function->argc[1] >= 0 ? "at least " : "",
							    function->argc[0],
							    function->argc[0] > 1  ? "s" : "",
							    argc );
	}
	/*
	 * Ok, argc is the right one (or one of the right ones), now evaluate each
	 * object and check the type.
	 */
	stack.owner = owner;
	for( i = 0; i < argc; ++i ){
		value = engine_exec( engine, root, argv->child(i) );
		if( root->state.is(Exception) ){
		   return;
	    }
		if( f_argc != -1 && i < f_argc ){
			for( t = 0 ;; ++t ){
				type = function->types[i][t];
				if( type <= otVoid ){
					break;
				}
				else if( value->type->code == type ){
					break;
				}
			}
			/*
			 * Same as before, check if H_ANY_TYPE given or report error.
			 */
			if( type <= otVoid && t != 0 ){
				std::stringstream error;

				error << "Invalid " << ob_typename(value)
					  << " type for argument " << i + 1
					  << " of '"
					  << function->identifier.c_str()
					  << "' function, required type"
					  << (function->types[i][1] > 0 ? "s are " : " is ");

				for( t = 0 ;; ++t ){
					type = function->types[i][t];
					if( type <= otVoid ){
						break;
					}
					bool prev_last = ( function->types[i][t + 2] <= otVoid );
					bool last      = ( function->types[i][t + 1] <= otVoid );
					error << ob_type_to_string(type) << ( last ? "" : (prev_last ? " or " : ", ") );
				}

				hyb_error( H_ET_SYNTAX, error.str().c_str() );
			}
		}

		stack.push( value );
	}

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );
}

__force_inline void engine_dismiss_stack( engine_t *engine, vframe_t &stack ){
	vm_pop_frame( engine->vm );
}

__force_inline Node * engine_find_function( engine_t *engine, vframe_t *frame, Node *call ){
    char *callname = (char *)call->value.m_call.c_str();

    /* search first in the code segment */
	Node *function = H_UNDEFINED;
	if( (function = engine->code->get(callname)) != H_UNDEFINED ){
		return function;
	}
	/* then search for a function alias */
	AliasObject *alias = (AliasObject *)frame->get( callname );
	if( alias != H_UNDEFINED && ob_is_alias(alias) ){
		return (Node *)alias->value;
	}
	/* try to evaluate the call as an alias itself */
	if( call->value.m_alias_call != NULL ){
		alias = (AliasObject *)engine_exec( engine, frame, call->value.m_alias_call );
		if( ob_is_alias(alias) ){
			return (Node *)alias->value;
		}
	}
	/* function is not defined */
	return H_UNDEFINED;
}

Object *engine_exec( engine_t *engine, vframe_t *frame, Node *node ){
    /*
	 * An exception has been thrown, wait for a try-catch statement or,
	 * when the frame will be deleted (vframe_t class destructor), exit
	 * with a non handled exception.
	 */
	if( frame->state.is(Exception) ){
		return frame->state.value;
	}
    /*
	 * A return statement was succesfully executed, skip everything
	 * now on until the frame will be destroyed and return appropriate
	 * value.
	 */
	else if( frame->state.is(Return) ){
		return frame->state.value;
	}
    /*
     * A next statement was found, so skip nodes execution
     * until one of the loop handlers will reset the flag.
     */
    else if( frame->state.is(Next) ){
    	return H_DEFAULT_RETURN;
    }
	/*
	 * Null node, probably a function or method without statements.
	 */
    else if( node == H_UNDEFINED ){
    	return H_DEFAULT_RETURN;
    }

    switch( node->type() ){
        /* identifier */
        case H_NT_IDENTIFIER :
            return engine_on_identifier( engine, frame, node );
        /* attribute */
        case H_NT_ATTRIBUTE  :
            return engine_on_attribute_request( engine, frame, node );
		/* attribute */
		case H_NT_METHOD_CALL :
			return engine_on_method_call( engine, frame, node );
        /* constant value */
        case H_NT_CONSTANT   :
            return engine_on_constant( engine, frame, node );
        /* function definition */
        case H_NT_FUNCTION   :
            return engine_on_function_declaration( engine, frame, node );
        /* structure or class creation */
        case H_NT_NEW :
            return engine_on_new_operator( engine, frame, node );
        /* function call */
        case H_NT_CALL       :
            return engine_on_function_call( engine, frame, node );
        /* struct type declaration */
        case H_NT_STRUCT :
            return engine_on_structure_declaration( engine, frame, node );
        /* class type declaration */
        case H_NT_CLASS :
			return engine_on_class_declaration( engine, frame, node );

		/* statements */
        case H_NT_STATEMENT :
        	/*
        	 * Call the garbage collection routine every new statement.
        	 * If the routine would be called on expressions too, there would be a high
        	 * risk of loosing tmp values such as evaluations, ecc.
        	 */
        	gc_collect( engine->vm );

            switch( node->value.m_statement ){
				/* statement unless expression */
				case T_UNLESS :
					return engine_on_unless( engine, frame, node );
                /* if( condition ) */
                case T_IF     :
                    return engine_on_if( engine, frame, node );
                /* while( condition ){ body } */
                case T_WHILE  :
                    return engine_on_while( engine, frame, node );
                /* do{ body }while( condition ); */
                case T_DO  :
                    return engine_on_do( engine, frame, node );
                /* for( initialization; condition; variance ){ body } */
                case T_FOR    :
                    return engine_on_for( engine, frame, node );
                /* foreach( item of array ) */
                case T_FOREACH :
                    return engine_on_foreach( engine, frame, node );
                /* foreach( label -> item of map ) */
                case T_FOREACHM :
                    return engine_on_foreach_mapping( engine, frame, node );
				/* break; */
				case T_BREAK :
					engine_on_break_state( frame );
				break;
				/* next; */
				case T_NEXT :
					engine_on_next_state( frame );
				break;
				/* return */
				case T_RETURN :
					return engine_on_return( engine, frame, node );
                /* (condition ? expression : expression) */
                case T_QUESTION :
                    return engine_on_question( engine, frame, node );
				/* switch statement */
                case T_SWITCH :
                    return engine_on_switch( engine, frame, node );
                case T_EXPLODE :
                	return engine_on_explode( engine, frame, node );
                /* throw expression; */
                case T_THROW :
					return engine_on_throw( engine, frame, node );
				/* try-catch statement */
                case T_TRY :
                	return engine_on_try_catch( engine, frame, node );
            }
        break;

        /* expressions */
        case H_NT_EXPRESSION   :
            switch( node->value.m_expression ){
                /* identifier = expression */
                case T_ASSIGN    :
                    return engine_on_assign( engine, frame, node );
                /* expression ; */
                case T_EOSTMT  :
                    return engine_on_eostmt( engine, frame, node );
                /* [ a, b, c, d ] */
                case T_ARRAY :
					return engine_on_array( engine, frame, node );
				/* [ a : b, c : d ] */
                case T_MAP :
					return engine_on_map( engine, frame, node );
                /* & expression */
                case T_REF :
                	return engine_on_reference( engine, frame, node );
                break;
                /* `string` */
                case T_BACKTICK :
					return engine_on_backtick( engine, frame, node );
                break;
                /* $ */
                case T_DOLLAR :
                    return engine_on_dollar( engine, frame, node );
                /* @ */
                case T_VARGS :
                	return engine_on_vargs( engine, frame, node );
                /* expression .. expression */
                case T_RANGE :
                    return engine_on_range( engine, frame, node );
                /* array[] = object; */
                case T_SUBSCRIPTADD :
                    return engine_on_subscript_push( engine, frame, node );
                /* (identifier)? = object[ expression ]; */
                case T_SUBSCRIPTGET :
                    return engine_on_subscript_get( engine, frame, node );
                /* object[ expression ] = expression */
                case T_SUBSCRIPTSET :
                    return engine_on_subscript_set( engine, frame, node );
                /* -expression */
                case T_UMINUS :
                    return engine_on_uminus( engine, frame, node );
                /* expression ~= expression */
                case T_REGEX_OP :
                    return engine_on_regex( engine, frame, node );
                /* expression + expression */
                case T_PLUS    :
                    return engine_on_add( engine, frame, node );
                /* expression += expression */
                case T_PLUSE   :
                    return engine_on_inplace_add( engine, frame, node );
                /* expression - expression */
                case T_MINUS    :
                    return engine_on_sub( engine, frame, node );
                /* expression -= expression */
                case T_MINUSE   :
                    return engine_on_inplace_sub( engine, frame, node );
                /* expression * expression */
                case T_MUL	:
                    return engine_on_mul( engine, frame, node );
                /* expression *= expression */
                case T_MULE	:
                    return engine_on_inplace_mul( engine, frame, node );
                /* expression / expression */
                case T_DIV    :
                    return engine_on_div( engine, frame, node );
                /* expression /= expression */
                case T_DIVE   :
                    return engine_on_inplace_div( engine, frame, node );
                /* expression % expression */
                case T_MOD    :
                    return engine_on_mod( engine, frame, node );
                /* expression %= expression */
                case T_MODE   :
                    return engine_on_inplace_mod( engine, frame, node );
                /* expression++ */
                case T_INC    :
                    return engine_on_inc( engine, frame, node );
                /* expression-- */
                case T_DEC    :
                    return engine_on_dec( engine, frame, node );
                /* expression ^ expression */
                case T_XOR    :
                    return engine_on_xor( engine, frame, node );
                /* expression ^= expression */
                case T_XORE   :
                    return engine_on_inplace_xor( engine, frame, node );
                /* expression & expression */
                case T_AND    :
                    return engine_on_and( engine, frame, node );
                /* expression &= expression */
                case T_ANDE   :
                    return engine_on_inplace_and( engine, frame, node );
                /* expression | expression */
                case T_OR     :
                    return engine_on_or( engine, frame, node );
                /* expression |= expression */
                case T_ORE    :
                    return engine_on_inplace_or( engine, frame, node );
                /* expression << expression */
                case T_SHIFTL  :
                    return engine_on_shiftl( engine, frame, node );
                /* expression <<= expression */
                case T_SHIFTLE :
                    return engine_on_inplace_shiftl( engine, frame, node );
                /* expression >> expression */
                case T_SHIFTR  :
                    return engine_on_shiftr( engine, frame, node );
                /* expression >>= expression */
                case T_SHIFTRE :
                    return engine_on_inplace_shiftr( engine, frame, node );
                /* expression! */
                case T_FACT :
                    return engine_on_fact( engine, frame, node );
                /* ~expression */
                case T_NOT    :
                    return engine_on_not( engine, frame, node );
                /* !expression */
                case T_L_NOT   :
                    return engine_on_lnot( engine, frame, node );
                /* expression < expression */
                case T_LESS    :
                    return engine_on_less( engine, frame, node );
                /* expression > expression */
                case T_GREATER    :
                    return engine_on_greater( engine, frame, node );
                /* expression >= expression */
                case T_GREATER_EQ     :
                    return engine_on_ge( engine, frame, node );
                /* expression <= expression */
                case T_LESS_EQ     :
                    return engine_on_le( engine, frame, node );
                /* expression != expression */
                case T_NOT_SAME     :
                    return engine_on_ne( engine, frame, node );
                /* expression == expression */
                case T_SAME     :
                    return engine_on_eq( engine, frame, node );
                /* expression && expression */
                case T_L_AND   :
                    return engine_on_land( engine, frame, node );
                /* expression || expression */
                case T_L_OR    :
                    return engine_on_lor( engine, frame, node );
            }
    }

    return H_DEFAULT_RETURN;
}

__force_inline Object *engine_on_identifier( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    Node   *function   = H_UNDEFINED;
    char   *identifier = (char *)node->value.m_identifier.c_str();

    /*
   	 * First thing first, check for a constant object name.
   	 */
   	if( (o = engine->cnst->get(identifier)) != H_UNDEFINED ){
   		return o;
   	}
   	/*
	 * Search for the identifier definition on
	 * the function local stack frame.
	 */
   	else if( (o = frame->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Let's check if the address of this frame si different from
	 * global frame one, in that case try to search the definition
	 * on the global frame too.
	 */
	else if( H_ADDRESS_OF(frame) != H_ADDRESS_OF(engine->mem) && (o = engine->mem->get( identifier )) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Check for an user defined object (structure or class) name.
	 */
	else if( (o = engine->types->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * So, it's neither defined on local frame nor in the global one,
	 * let's search for it in the engine->code frame.
	 */
	else if( (function = engine->code->find( identifier )) != H_UNDEFINED ){
		/*
		 * Create an alias to that engine->code region (basically its index).
		 */
		return ob_dcast( gc_new_alias( H_ADDRESS_OF(function) ) );
	}
	/*
	 * Ok ok, got it! It's undefined, raise an error.
	 */
	else{
		/*
		 * Check for the special 'me' keyword.
		 * If 'me' instance is not defined anywhere, we are in the
		 * main program body or inside a static method.
		 */
		if( strcmp( identifier, "me" ) != 0 ){
			hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
		}
		else{
			hyb_error( H_ET_SYNTAX, "couldn't use 'me' instance inside a global or static scope" );
		}
	}
}

__force_inline Object *engine_on_attribute_request( engine_t *engine, vframe_t *frame, Node *node ){
	Object  *cobj      = H_UNDEFINED,
		    *attribute = H_UNDEFINED;
	char    *name,
			*owner_id;
	access_t access;
	Node    *member = node->value.m_member;

	cobj      = engine_exec( engine, frame, node->value.m_owner );
	owner_id  = (char *)node->value.m_owner->value.m_identifier.c_str();
	name      = (char *)member->value.m_identifier.c_str();
	attribute = ob_get_attribute( cobj, name, true );

	if( attribute == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' is not an attribute of object '%s'", name, ob_typename(cobj) );
	}
	/*
	 * Check attribute access.
	 */
	access = ob_attribute_access( cobj, name );
	/*
	 * If the attribute has public access, skip the access checking
	 * because everyone can access it.
	 */
	if( access != asPublic ){
		/*
		 * Protected attributes can be accessed only by the class itself
		 * or derived classes.
		 */
		if( access == asProtected && strcmp( owner_id, "me" ) != 0 ){
			hyb_error( H_ET_SYNTAX, "Protected attribute '%s' can be accessed only by derived classes of '%s'", name, ob_typename(cobj) );
		}
		/*
		 * The attribute is private, so only the owner can use it.
		 * Let's check if the class pointed by 'me' it's the owner of
		 * the private attribute.
		 */
		else if( access == asPrivate && strcmp( owner_id, "me" ) != 0 ){
			hyb_error( H_ET_SYNTAX, "Private attribute '%s' can be accessed only within '%s' class", name, ob_typename(cobj) );
		}
	}

	return attribute;
}

__force_inline Object *engine_on_method_call( engine_t *engine, vframe_t *frame, Node *node ){
	Object  *cobj   = H_UNDEFINED;
	char    *name,
			*owner_id;
	Node    *member = node->value.m_member;

	cobj 	 = engine_exec( engine, frame, node->value.m_owner );
	owner_id = (char *)node->value.m_owner->value.m_identifier.c_str();
	name 	 = (char *)member->value.m_call.c_str();

	return ob_call_method( engine, frame, cobj, owner_id, name, member );
}

__force_inline Object *engine_on_constant( engine_t *engine, vframe_t *frame, Node *node ){
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

__force_inline Object *engine_on_function_declaration( engine_t *engine, vframe_t *frame, Node *node ){
    char *function_name = (char *)node->value.m_function.c_str();

    /* check for double definition */
    if( engine->code->get(function_name) != H_UNDEFINED ){
        hyb_error( H_ET_SYNTAX, "function '%s' already defined", function_name );
    }
    else if( vm_get_function( engine->vm, function_name ) != H_UNDEFINED ){
        hyb_error( H_ET_SYNTAX, "function '%s' already defined as a language function", function_name );
    }
    /* add the function to the engine->code segment */
    engine->code->add( function_name, node );

    return H_UNDEFINED;
}

__force_inline Object *engine_on_structure_declaration( engine_t *engine, vframe_t *frame, Node * node ){
    int        i, attributes( node->children() );
    char      *structname = (char *)node->value.m_identifier.c_str();

	if( engine->types->find(structname) != H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "Structure '%s' already defined", structname );
	}

	/* structure prototypes are not garbage collected */
    Object *s = (Object *)(new StructureObject());

    for( i = 0; i < attributes; ++i ){
        ob_add_attribute( s, (char *)node->child(i)->value.m_identifier.c_str() );
    }
    /*
     * ::defineType will take care of the structure attributes
	 * to prevent it to be garbage collected (see ::onConstant).
     */
    vm_define_type( engine->vm, structname, s );

    return H_UNDEFINED;
}

__force_inline Object *engine_on_class_declaration( engine_t *engine, vframe_t *frame, Node *node ){
	int        i, members( node->children() );
	char      *classname = (char *)node->value.m_identifier.c_str(),
			  *attrname;
	Node      *attribute;
	Object    *static_attr_value;

	if( engine->types->find(classname) != H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "Class '%s' already defined", classname );
	}

	/* class prototypes are not garbage collected */
	Object *c = (Object *)(new ClassObject());
	/*
	 * Set specific class name.
	 */
	((ClassObject *)c)->name = classname;

	for( i = 0; i < members; ++i ){
		/*
		 * Define an attribute
		 */
		if( node->child(i)->type() == H_NT_IDENTIFIER ){
			attribute = node->child(i);
			/*
			 * Initialize static attributes.
			 */
			if( attribute->value.m_static ){
				static_attr_value = engine_exec( engine, frame, attribute->child(0) );
				/*
				 * Static attributes are not garbage collectable until the end of
				 * the program is reached because they reside in a global scope.
				 */
				GC_SET_UNTOUCHABLE(static_attr_value);
				/*
				 * Initialize the attribute definition in the prototype.
				 */
				ob_class_ucast(c)->c_attributes.insert( attribute->id(),
														new class_attribute_t( attribute->id(),
																			   attribute->value.m_access,
																			   static_attr_value,
																			   attribute->value.m_static ) );
			}
			else{
				/*
				 * Non static attribute, just define it with a void value.
				 */
				ob_define_attribute( c, attribute->id(), attribute->value.m_access, attribute->value.m_static );
			}
		}
		/*
		 * Define a method
		 */
		else if( node->child(i)->type() == H_NT_METHOD_DECL ){
			ob_define_method( c, (char *)node->child(i)->value.m_method.c_str(), node->child(i) );
		}
		/*
		 * WTF this should not happen!
		 * The parser should not accept anything that's not an attribute
		 * or a method declaration.
		 */
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
			Object *type = vm_get_type( engine->vm, (char *)(*ni)->value.m_identifier.c_str() );
			if( type == H_UNDEFINED ){
				hyb_error( H_ET_SYNTAX, "'%s' undeclared class type", (*ni)->value.m_identifier.c_str() );
			}
			else if( ob_is_class(type) == false ){
				hyb_error( H_ET_SYNTAX, "couldn't extend from '%s' type", type->type->name );
			}

			ClassObject 			     *cobj = ob_class_ucast(type);
			ClassObjectAttributeIterator  ai;
			ClassObjectMethodIterator 	  mi;
			ClassObjectPrototypesIterator pi;

			for( ai = cobj->c_attributes.begin(); ai != cobj->c_attributes.end(); ai++ ){
				attrname  = (char *)(*ai)->label.c_str();

				ob_define_attribute( c, attrname, (*ai)->value->access, (*ai)->value->is_static );

				/*
				 * Initialize static attributes.
				 */
				if( (*ai)->value->is_static){
					ob_set_attribute_reference( c, attrname, (*ai)->value->value );
				}
			}

			for( mi = cobj->c_methods.begin(); mi != cobj->c_methods.end(); mi++ ){
				for( pi = (*mi)->value->prototypes.begin(); pi != (*mi)->value->prototypes.end(); pi++ ){
					ob_define_method( c, (char *)(*mi)->label.c_str(), *pi );
				}
			}
		}
	}

	/*
	 * ::defineType will take care of the class attributes
	 * to prevent it to be garbage collected (see ::onConstant).
	 */
	vm_define_type( engine->vm, classname, c );
}

__force_inline Object *engine_on_builtin_function_call( engine_t *engine, vframe_t *frame, Node * call ){
    char        *callname = (char *)call->value.m_call.c_str();
    named_function_t* function;
    vframe_t     stack;
    Object      *result = H_UNDEFINED;

    if( (function = vm_get_function( engine->vm, callname )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    engine_prepare_stack( engine, frame, function, stack, string(callname), call );

    engine_check_frame_exit(frame);

    /* call the function */
    result = function->function( engine->vm, &stack );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

    engine_dismiss_stack( engine, stack );

    /* return function evaluation value */
    return result;
}

__force_inline Object *engine_on_threaded_call( engine_t *engine, string function_name, vframe_t *frame, vmem_t *argv ){
	Node    *function             = H_UNDEFINED;
	vframe_t stack;
	Object  *result               = H_UNDEFINED;
	Node    *body                 = H_UNDEFINED;

	vector<string> identifiers;
	unsigned int i(0), children;

	/* search first in the engine->code segment */
	if( (function = engine->code->get((char *)function_name.c_str())) == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' undeclared user function identifier", function_name.c_str() );
	}

    children = function->children();
    for( i = 0, body = function->child(0); i < children; ++i ){
    	body = function->child(i);
    	if( body->type() == H_NT_IDENTIFIER ){
    		identifiers.push_back( body->value.m_identifier );
    	}
    	else{
    		break;
    	}
    }

    if( function->value.m_vargs ){
    	if( argv->size() < identifiers.size() ){
			hyb_error( H_ET_SYNTAX, "function '%s' requires at least %d parameters (called with %d)",
									function_name.c_str(),
									identifiers.size(),
									argv->size() );
    	}
    }
    else{
    	if( identifiers.size() != argv->size() ){
			hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
								   function_name.c_str(),
								   identifiers.size(),
								   argv->size() );
    	}
	}

	engine_prepare_stack( engine, frame, stack, function_name, identifiers, argv );

	engine_check_frame_exit(frame);

	/* call the function */
	result = engine_exec( engine, &stack, body );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

	engine_dismiss_stack( engine, stack );

	/* return function evaluation value */
	return result;
}

__force_inline Object *engine_on_threaded_call( engine_t *engine, Node *function, vframe_t *frame, vmem_t *argv ){
	vframe_t stack;
	Object  *result = H_UNDEFINED;
	Node    *body   = H_UNDEFINED;

	vector<string> identifiers;
	unsigned int i(0), children;

    children = function->children();
    for( i = 0, body = function->child(0); i < children; ++i ){
    	body = function->child(i);
    	if( body->type() == H_NT_IDENTIFIER ){
    		identifiers.push_back( body->value.m_identifier );
    	}
    	else{
    		break;
    	}
    }

	if( identifiers.size() != argv->size() ){
		hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
							    function->value.m_function.c_str(),
							    identifiers.size(),
							    argv->size() );
	}

	engine_prepare_stack( engine, frame, stack, function->value.m_function, identifiers, argv );

	engine_check_frame_exit(frame);

	/* call the function */
	result = engine_exec( engine, &stack, body );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

	engine_dismiss_stack( engine, stack );

	/* return function evaluation value */
	return result;
}

__force_inline Object *engine_on_user_function_call( engine_t *engine, vframe_t *frame, Node *call ){
    Node    *function = H_UNDEFINED;
    vframe_t stack;
    Object  *result   = H_UNDEFINED;
    Node    *body     = H_UNDEFINED;

    vector<string> identifiers;
    unsigned int i(0), children;

    if( (function = engine_find_function( engine, frame, call )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    children = function->children();
    for( i = 0, body = function->child(0); i < children; ++i ){
    	body = function->child(i);
    	if( body->type() == H_NT_IDENTIFIER ){
    		identifiers.push_back( body->value.m_identifier );
    	}
    	else{
    		break;
    	}
    }

    if( function->value.m_vargs ){
    	if( call->children() < identifiers.size() ){
   			hyb_error( H_ET_SYNTAX, "function '%s' requires at least %d parameters (called with %d)",
									function->value.m_function.c_str(),
   									identifiers.size(),
   									call->children() );
       }
   	}
    else{
		if( identifiers.size() != call->children() ){
			hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
									function->value.m_function.c_str(),
									identifiers.size(),
									call->children() );
		}
    }

    engine_prepare_stack( engine, frame, stack, function->value.m_function, identifiers, call );

    engine_check_frame_exit(frame);

    /* call the function */
    result = engine_exec( engine, &stack, body );

    engine_dismiss_stack( engine, stack );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

    /* return function evaluation value */
    return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

__force_inline Object *engine_on_new_operator( engine_t *engine, vframe_t *frame, Node *type ){
    char      *type_name = (char *)type->value.m_identifier.c_str();
    Object    *user_type = H_UNDEFINED,
              *newtype   = H_UNDEFINED,
              *object    = H_UNDEFINED;
    size_t     i, children( type->children() );

	/*
	 * Search for the used defined type calls, most like C++
	 * class constructors but without strict prototypes.
	 */ 
    if( (user_type = vm_get_type( engine->vm,type_name)) == H_UNDEFINED ){
    	hyb_error( H_ET_SYNTAX, "'%s' undeclared type", type_name );
    }
    newtype = ob_clone(user_type);

	/*
	 * It's ok to initialize less attributes that the structure/class
	 * has (non ini'ed attributes are set to 0 by default), but
	 * you can not set more attributes than the structure/class have.
	 */
	if( ob_is_struct(newtype) ){
		StructureObject *stype = (StructureObject *)newtype;

		if( children > stype->items ){
			hyb_error( H_ET_SYNTAX, "structure '%s' has %d attributes, initialized with %d",
								 type_name,
								 stype->items,
								 children );
		}

		for( i = 0; i < children; ++i ){
			object = engine_exec( engine, frame, type->child(i) );
			ob_set_attribute( newtype, (char *)stype->s_attributes.label(i), object );
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
			if( ctor->value.m_vargs ){
				if( children < ctor->callDefinedArgc() ){
					hyb_error( H_ET_SYNTAX, "class '%s' constructor requires at least %d arguments, called with %d",
											 type_name,
											 ctor->callDefinedArgc(),
											 children );
			   }
			}
			else{
				if( children > ctor->callDefinedArgc() ){
					hyb_error( H_ET_SYNTAX, "class '%s' constructor requires %d arguments, called with %d",
											 type_name,
											 ctor->callDefinedArgc(),
											 children );
				}
			}

			vframe_t stack;

			engine_prepare_stack( engine,
								  frame,
								  stack,
								  string(type_name) + "::" + string(type_name),
								  newtype,
								  children,
								  ctor,
								  type );

			engine_check_frame_exit(frame);

			/* call the ctor */
			engine_exec( engine, &stack, ctor->callBody() );

			engine_dismiss_stack( engine, stack );

			/*
			 * Check for unhandled exceptions and put them on the root
			 * memory frame.
			 */
			if( stack.state.is(Exception) ){
				frame->state.set( Exception, stack.state.value );
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
				object = engine_exec( engine, frame, type->child(i) );
				ob_set_attribute( newtype, (char *)ob_class_ucast(newtype)->c_attributes.at(i)->name.c_str(), object );
			}
		}
	}

    return newtype;
}

__force_inline Object *engine_on_dll_function_call( engine_t *engine, vframe_t *frame, Node *call ){
    char    *callname   = (char *)call->value.m_call.c_str();
    vframe_t stack;
    Object  *result     = H_UNDEFINED,
            *fn_pointer = H_UNDEFINED;

    /*
     * We assume that dll module is already loaded, otherwise there shouldn't be
     * any onDllFunctionCall call .
     */
    named_function_t *dllcall = vm_get_function( engine->vm, (char *)"dllcall" );

    if( (fn_pointer = frame->get( callname )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }
    else if( ob_is_extern(fn_pointer) == false ){
        return H_UNDEFINED;
    }

    engine_prepare_stack( engine, frame, stack, string(callname), (ExternObject *)fn_pointer, call );

    engine_check_frame_exit(frame);

    /* call the function */
    result = dllcall->function( engine->vm, &stack );

    engine_dismiss_stack( engine, stack );

    /* return function evaluation value */
    return result;
}

__force_inline Object *engine_on_function_call( engine_t *engine, vframe_t *frame, Node *call ){
    Object *result = H_UNDEFINED;

    /* check if function is a builtin function */
    if( (result = engine_on_builtin_function_call( engine, frame, call )) != H_UNDEFINED ){
    	return result;
    }
    /* check for an user defined function */
    else if( (result = engine_on_user_function_call( engine, frame, call )) != H_UNDEFINED ){
    	return result;
    }
    /* check if the function is an extern identifier loaded by dll importing routines */
    else if( (result = engine_on_dll_function_call( engine, frame, call )) != H_UNDEFINED ){
    	return result;
    }
    else{
    	hyb_error( H_ET_SYNTAX, "'%s' undeclared function identifier", call->value.m_call.c_str() );
    }

    return result;
}

__force_inline Object *engine_on_reference( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );

    return (Object *)gc_new_reference(o);
}

__force_inline Object *engine_on_dollar( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    Node *function = H_UNDEFINED;
    string svalue;
    char *identifier;

    o 	   	   = engine_exec( engine, frame, node->child(0) );
    svalue 	   = ob_svalue(o);
    identifier = (char *)svalue.c_str();

    /*
   	 * Same as engine_on_identifier.
   	 */
   	if( (o = engine->cnst->get(identifier)) != H_UNDEFINED ){
   		return o;
   	}
   	/*
	 * Search for the identifier definition on
	 * the function local stack frame.
	 */
   	else if( (o = frame->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Let's check if the address of this frame si different from
	 * global frame one, in that case try to search the definition
	 * on the global frame too.
	 */
	else if( H_ADDRESS_OF(frame) != H_ADDRESS_OF(engine->mem) && (o = engine->mem->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Check for an user defined object (structure or class) name.
	 */
	else if( (o = engine->types->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * So, it's neither defined on local frame nor in the global one,
	 * let's search for it in the engine->code frame.
	 */
	else if( (function = engine->code->find(identifier)) != H_UNDEFINED ){
		/*
		 * Create an alias to that engine->code region (basically its index).
		 */
		return ob_dcast( gc_new_alias( H_ADDRESS_OF(function) ) );
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
	}
}

__force_inline Object *engine_on_return( engine_t *engine, vframe_t *frame, Node *node ){
	/*
	 * Set break and return state to make every loop and/or condition
	 * statement to exit with this return value.
	 */
    frame->state.value = engine_exec( engine, frame, node->child(0) );
    frame->state.set( Break );
    frame->state.set( Return );

    return frame->state.value;
}

__force_inline Object *engine_on_backtick( engine_t *engine, vframe_t *frame, Node *node ){
	Object *cmd  = engine_exec( engine, frame, node->child(0) );
	FILE   *pipe = popen( ob_svalue(cmd).c_str(), "r" );

	if( !pipe ){
		return H_DEFAULT_ERROR;
	}

	char buffer[128];
	std::string result = "";

	while( !feof(pipe) ){
		if( fgets( buffer, 128, pipe ) != NULL ){
			result += buffer;
		}
	}
	pclose(pipe);

	return (Object *)gc_new_string( result.c_str() );
}

__force_inline Object *engine_on_vargs( engine_t *engine, vframe_t *frame, Node *node ){
	Object *vargs = (Object *)gc_new_vector();
	int i, argc( frame->size() );

	for( i = 0; i < argc; ++i ){
		if( strcmp( frame->label(i), "me" ) != 0 ){
			ob_cl_push( vargs, frame->at(i) );
		}
	}

	return vargs;
}

__force_inline Object *engine_on_range( engine_t *engine, vframe_t *frame, Node *node ){
    Object *range = H_UNDEFINED,
           *from  = H_UNDEFINED,
           *to    = H_UNDEFINED;

    from  = engine_exec( engine, frame, node->child(0) );
   	to    = engine_exec( engine, frame, node->child(1) );

   	engine_check_frame_exit(frame)

	range = ob_range( from, to );

	return range;
}

__force_inline Object *engine_on_subscript_push( engine_t *engine, vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *object = H_UNDEFINED,
           *res    = H_UNDEFINED;

    array  = engine_exec( engine, frame, node->child(0) );

	object = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	res    = ob_cl_push( array, object );

	return res;
}

__force_inline Object *engine_on_subscript_get( engine_t *engine, vframe_t *frame, Node *node ){
    Object *identifier = H_UNDEFINED,
           *array      = H_UNDEFINED,
           *index      = H_UNDEFINED,
           *result     = H_UNDEFINED;

    if( node->children() == 3 ){
		array 	   = engine_exec( engine, frame, node->child(1) );
		identifier = engine_exec( engine, frame, node->child(0) );
		index      = engine_exec( engine, frame, node->child(2) );

		engine_check_frame_exit(frame)

		ob_assign( identifier,
					ob_cl_at( array, index )
				  );

		result = identifier;
	}
	else{
		array  = engine_exec( engine, frame, node->child(0) );
		index  = engine_exec( engine, frame, node->child(1) );

		engine_check_frame_exit(frame)

		result = ob_cl_at( array, index );
	}

		return result;
}

__force_inline Object *engine_on_subscript_set( engine_t *engine, vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *index  = H_UNDEFINED,
           *object = H_UNDEFINED;

    array = engine_exec( engine, frame, node->child(0) );
   	index  = engine_exec( engine, frame, node->child(1) );
   	object = engine_exec( engine, frame, node->child(2) );

   	engine_check_frame_exit(frame)

   	ob_cl_set( array, index, object );

   	return array;
}

__force_inline Object *engine_on_while( engine_t *engine, vframe_t *frame, Node *node ){
    Node   *condition,
		   *body;
    Object *result  = H_UNDEFINED;

    condition = node->child(0);
    body      = node->child(1);

    while( ob_lvalue( engine_exec( engine, frame, condition ) ) ){
   		result = engine_exec( engine, frame, body );

   		engine_check_frame_exit(frame)

   		frame->state.unset(Next);
        if( frame->state.is(Break) ){
        	frame->state.unset(Break);
			break;
        }
    }

    return result;
}

__force_inline Object *engine_on_do( engine_t *engine, vframe_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *result  = H_UNDEFINED;

    body      = node->child(0);
    condition = node->child(1);
    do{
		result = engine_exec( engine, frame, body );

		engine_check_frame_exit(frame)

		frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }
    while( ob_lvalue( engine_exec( engine, frame, condition ) ) );

    return result;
}

__force_inline Object *engine_on_for( engine_t *engine, vframe_t *frame, Node *node ){
    Node   *condition,
           *increment,
		   *body;
    Object *result  = H_UNDEFINED;

    condition = node->child(1);
    increment = node->child(2);
    body      = node->child(3);

    engine_exec( engine, frame, node->child(0) );

    engine_check_frame_exit(frame)

    for( ; ob_lvalue( engine_exec( engine, frame, condition ) ); engine_exec( engine, frame, increment ) ){

    	engine_check_frame_exit(frame)

    	result = engine_exec( engine, frame, body );

		engine_check_frame_exit(frame)

		frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }

    return result;
}

__force_inline Object *engine_on_foreach( engine_t *engine, vframe_t *frame, Node *node ){
    int     size;
    Node   *body;
    Object *v      = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *identifier;
    IntegerObject index(0);

    identifier = (char *)node->child(0)->value.m_identifier.c_str();
    v          = engine_exec( engine, frame, node->child(1) );
    body       = node->child(2);
    size       = ob_get_size(v);

    /*
     * Prevent the vector from being garbage collected, because may cause
     * seg faults in situations like :
     *
     * 		foreach( i of 1..10 )
     */
    GC_SET_UNTOUCHABLE(v);

    for( ; index.value < size; ++index.value ){
        frame->add( identifier, ob_cl_at( v, (Object *)&index ) );

        result = engine_exec( engine, frame, body );

        if( frame->state.is(Exception) || frame->state.is(Return) ){
        	GC_RESET(v);
        	return frame->state.value;
	    }

        frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }

    GC_RESET(v);

    return result;
}

__force_inline Object *engine_on_foreach_mapping( engine_t *engine, vframe_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *key_identifier,
           *value_identifier;

    key_identifier   = (char *)node->child(0)->value.m_identifier.c_str();
    value_identifier = (char *)node->child(1)->value.m_identifier.c_str();
    map              = engine_exec( engine, frame, node->child(2) );
    body             = node->child(3);
    size             = ob_get_size(map);

    /*
     * Prevent the map from being garbage collected, because may cause
     * seg faults in situations like :
     *
     * 		foreach( i of map( ... ) )
     */
    GC_SET_UNTOUCHABLE(map);

    for( i = 0; i < size; ++i ){
        frame->add( key_identifier,   ob_map_ucast(map)->keys[i] );
        frame->add( value_identifier, ob_map_ucast(map)->values[i] );

        result = engine_exec( engine, frame, body );

        if( frame->state.is(Exception) || frame->state.is(Return) ){
			GC_RESET(map);
			return frame->state.value;
		}

        frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }

    GC_RESET(map);

    return result;
}

__force_inline Object *engine_on_unless( engine_t *engine, vframe_t *frame, Node *node ){
	Object *boolean = H_UNDEFINED,
		   *result  = H_UNDEFINED;

	boolean = engine_exec( engine, frame, node->child(1) );

	if( !ob_lvalue(boolean) ){
		result = engine_exec( engine, frame, node->child(0) );
	}

	engine_check_frame_exit(frame)

	return H_UNDEFINED;
}

__force_inline Object *engine_on_if( engine_t *engine, vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = engine_exec( engine, frame, node->child(0) );

    if( ob_lvalue(boolean) ){
        result = engine_exec( engine, frame, node->child(1) );
    }
    /* handle else case */
    else if( node->children() > 2 ){
        result = engine_exec( engine, frame, node->child(2) );
    }

    engine_check_frame_exit(frame)

    return H_UNDEFINED;
}

__force_inline Object *engine_on_question( engine_t *engine, vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = engine_exec( engine, frame, node->child(0) );

    if( ob_lvalue(boolean) ){
        result = engine_exec( engine, frame, node->child(1) );
    }
    else{
        result = engine_exec( engine, frame, node->child(2) );
    }

    engine_check_frame_exit(frame)

    return result;
}

__force_inline Object *engine_on_switch( engine_t *engine, vframe_t *frame, Node *node){
    Node   *case_node = H_UNDEFINED,
           *stmt_node = H_UNDEFINED;
    Object *target    = H_UNDEFINED,
           *compare   = H_UNDEFINED,
           *result    = H_UNDEFINED;
    int     size( node->children() ),
            i;

    target = engine_exec( engine, frame, node->value.m_switch );

    // exec case labels
    for( i = 0; i < size; i += 2 ){
        stmt_node = node->child(i + 1);
        case_node = node->child(i);

        if( case_node != H_UNDEFINED && stmt_node != H_UNDEFINED ){
            compare = engine_exec( engine, frame, case_node );

            engine_check_frame_exit(frame)

            if( ob_cmp( target, compare ) == 0 ){
                return engine_exec( engine, frame, stmt_node );
            }
        }
    }

    // exec default case
    if( node->value.m_default != H_UNDEFINED ){
        result = engine_exec( engine, frame, node->value.m_default );

        engine_check_frame_exit(frame)
    }

    return result;
}

__force_inline Object *engine_on_explode( engine_t *engine, vframe_t *frame, Node *node ){
	Node   *expr  = H_UNDEFINED;
	Object *value = H_UNDEFINED;

	expr  = node->child(0);
	value = engine_exec( engine, frame, expr );

	size_t n_ids   = node->children() - 1,
		   n_items = ob_get_size(value),
		   n_end   = (n_ids > n_items ? n_items : n_ids),
		   i;

	/*
	 * Initialize all the identifiers with a <false>.
	 */
	for( i = 0; i < n_ids; ++i ){
		frame->add( (char *)node->child(i + 1)->value.m_identifier.c_str(), (Object *)gc_new_boolean(false) );
	}
	/*
	 * Fill initializers until the iterable object ends, leave
	 * the rest of them to <null>.
	 */
	IntegerObject index(0);
	for( ; (unsigned)index.value < n_end; ++index.value ){
		frame->add( (char *)node->child(index.value + 1)->value.m_identifier.c_str(), ob_cl_at( value, (Object *)&index ) );
	}

	return value;
}

__force_inline Object *engine_on_throw( engine_t *engine, vframe_t *frame, Node *node ){
	Object *exception = H_UNDEFINED;

	exception = engine_exec( engine, frame, node->child(0) );

	/*
	 * Make sure the exception object will not be freed until someone
	 * catches it or the program ends.
	 */
	GC_SET_UNTOUCHABLE(exception);

	frame->state.set( Exception, exception );

	return exception;
}

__force_inline Object *engine_on_try_catch( engine_t *engine, vframe_t *frame, Node *node ){
	Node   *main_body    = node->value.m_try_block,
		   *ex_ident     = node->value.m_exp_id,
		   *catch_body   = node->value.m_catch_block,
		   *finally_body = node->value.m_finally_block;
	Object *exception    = H_UNDEFINED;

	engine_exec( engine, frame, main_body );

	if( frame->state.is(Exception) ){
		exception = frame->state.value;

		assert( exception != H_UNDEFINED );

		frame->add( (char *)ex_ident->value.m_identifier.c_str(), exception );

		frame->state.unset(Exception);

		engine_exec( engine, frame, catch_body );

		/*
		 * See engine_on_throw for more details about this.
		 *
		 * Now the exception is garbage collectable.
		 */
		GC_RESET(exception);
	}

	if( finally_body != NULL ){
		engine_exec( engine, frame, finally_body );
	}

	return H_DEFAULT_RETURN;
}

__force_inline Object *engine_on_eostmt( engine_t *engine, vframe_t *frame, Node *node ){
    Object *res_1 = H_UNDEFINED,
           *res_2 = H_UNDEFINED;

    res_1 = engine_exec( engine, frame, node->child(0) );
    res_2 = engine_exec( engine, frame, node->child(1) );

    engine_check_frame_exit(frame)

    return res_2;
}

__force_inline Object *engine_on_array( engine_t *engine, vframe_t *frame, Node *node ){
	VectorObject *v = gc_new_vector();
	size_t i, items( node->children() );

	for( i = 0; i < items; ++i ){
		ob_cl_push_reference( (Object *)v, engine_exec( engine, frame, node->child(i) ) );
	}

	return (Object *)v;
}

__force_inline Object *engine_on_map( engine_t *engine, vframe_t *frame, Node *node ){
	MapObject *m = gc_new_map();
	size_t i, items( node->children() );

	for( i = 0; i < items; i += 2 ){
		ob_cl_set_reference( (Object *)m,
							 engine_exec( engine, frame, node->child(i) ),
							 engine_exec( engine, frame, node->child(i + 1) ) );
	}

	return (Object *)m;
}

__force_inline Object *engine_on_assign( engine_t *engine, vframe_t *frame, Node *node ){
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

    	value  = engine_exec( engine, frame, node->child(1) );

    	engine_check_frame_exit(frame)

		object = frame->add( (char *)lexpr->value.m_identifier.c_str(), value );

		return object;
    }
    /*
     * If not, we evaluate the first node as a "owner->child->..." sequence,
     * just like the engine_on_attribute_request handler.
     */
    else if( lexpr->type() == H_NT_ATTRIBUTE ){
    	Node *member    = lexpr,
			 *owner     = member->value.m_owner,
			 *attribute = member->value.m_member;

    	Object *obj = engine_exec( engine, frame, owner ),
    		   *value;

    	engine_check_frame_exit(frame)

		value = engine_exec( engine, frame, node->child(1) );

    	engine_check_frame_exit(frame)

    	ob_set_attribute( obj, (char *)attribute->value.m_identifier.c_str(), value );

    	return obj;
    }
    else{
    	hyb_error( H_ET_SYNTAX, "Unexpected constant expression for = operator" );
    }
}

__force_inline Object *engine_on_uminus( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *result = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );

    engine_check_frame_exit(frame)

    result = ob_uminus(o);

    return result;
}

__force_inline Object *engine_on_regex( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *regexp = H_UNDEFINED,
           *result = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );
	regexp = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	result = ob_apply_regexp( o, regexp );

	return result;
}

__force_inline Object *engine_on_add( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_add( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_add( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_inplace_add( a, b );

	return a;
}

__force_inline Object *engine_on_sub( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_sub( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_sub( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_inplace_sub( a, b );

	return a;
}

__force_inline Object *engine_on_mul( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_mul( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_mul( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_inplace_mul( a, b );

	return a;
}

__force_inline Object *engine_on_div( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_div( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_div( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_inplace_div( a, b );

	return a;
}

__force_inline Object *engine_on_mod( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_mod( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_mod( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_inplace_mod( a, b );

	return a;
}

__force_inline Object *engine_on_inc( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );

	engine_check_frame_exit(frame)

	return ob_increment(o);
}

__force_inline Object *engine_on_dec( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );

	engine_check_frame_exit(frame)

	return ob_decrement(o);
}

__force_inline Object *engine_on_xor( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_bw_xor( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_xor( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_bw_inplace_xor( a, b );

	return a;
}

__force_inline Object *engine_on_and( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_bw_and( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_and( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_bw_inplace_and( a, b );

	return a;
}

__force_inline Object *engine_on_or( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_bw_or( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_or( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_bw_inplace_or( a, b );

	return a;
}

__force_inline Object *engine_on_shiftl( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	if( frame->state.is(Exception) ){
		return frame->state.value;
	}

	engine_check_frame_exit(frame)

	c = ob_bw_lshift( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_shiftl( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_bw_inplace_lshift( a, b );

	return a;
}

__force_inline Object *engine_on_shiftr( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_bw_rshift( a, b );

	return c;
}

__force_inline Object *engine_on_inplace_shiftr( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	ob_bw_inplace_rshift( a, b );

	return a;
}

__force_inline Object *engine_on_fact( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );

	engine_check_frame_exit(frame)

    r = ob_factorial(o);

    return r;
}

__force_inline Object *engine_on_not( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );

	engine_check_frame_exit(frame)

   	r = ob_bw_not(o);

   	return r;
}

__force_inline Object *engine_on_lnot( engine_t *engine, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = engine_exec( engine, frame, node->child(0) );

	engine_check_frame_exit(frame)

	r = ob_l_not(o);

	return r;
}

__force_inline Object *engine_on_less( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_less( a, b );

	return c;
}

__force_inline Object *engine_on_greater( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_greater( a, b );

	return c;
}

__force_inline Object *engine_on_ge( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_greater_or_same( a, b );

	return c;
}

__force_inline Object *engine_on_le( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_less_or_same( a, b );

	return c;
}

__force_inline Object *engine_on_ne( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_diff( a, b );

	return c;
}

__force_inline Object *engine_on_eq( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_same( a, b );

	return c;
}

__force_inline Object *engine_on_land( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_and( a, b );

	return c;
}

__force_inline Object *engine_on_lor( engine_t *engine, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = engine_exec( engine, frame, node->child(0) );
	b = engine_exec( engine, frame, node->child(1) );

	engine_check_frame_exit(frame)

	c = ob_l_or( a, b );

	return c;
}
