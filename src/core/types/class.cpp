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
/*
 * Special function to execute a __method class descriptor.
 */
Object *class_call_undefined_method( vm_t *vm, Object *c, char *c_name, char *method_name, Node *argv ){
	Node    *method = H_UNDEFINED;
	vframe_t stack,
			*frame;
	ll_item_t *iitem;
	Object  *value  = H_UNDEFINED,
			*result = H_UNDEFINED;
	size_t i, argc(argv->children());

	method = ob_get_method( c, (char *)"__method", 2 );
	if( method == H_UNDEFINED ){
		return H_UNDEFINED;
	}

	frame = vm_frame(vm);

	Vector *args = gc_new_vector();

	vm_add_frame( vm, &stack );
	/*
	 * Prevent args from being garbage collected.
	 */
	frame->push( (Object *)args );

	stack.owner = string(c_name) + "::" + string("__method");

	stack.insert( "me", c );
	stack.add( "name", (Object *)gc_new_string(method_name) );
	ll_foreach_to( &argv->m_children, iitem, i, argc ){
		value = vm_exec( vm, frame, ll_node( iitem ) );

		if( frame->state.is(Exception) || frame->state.is(Return) ){
			vm_pop_frame( vm );
			return frame->state.value;
	    }

		ob_cl_push( (Object *)args, value );
	}
	stack.add( "argv", (Object *)args );

	/* call the method */
	result = vm_exec( vm, &stack, method->body() );

	vm_pop_frame( vm );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm_frame( vm )->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

/*
 * Call the operator 'op_name' of class 'me' with 'argc' arguments if overloaded,
 * otherwise print a syntax error.
 */
Object *class_call_overloaded_operator( Object *me, const char *op_name, int argc, ... ){
	Node    *op = H_UNDEFINED;
	vframe_t stack;
	ll_item_t *iitem;
	Object  *result = H_UNDEFINED,
			*value  = H_UNDEFINED;
	unsigned int i, op_argc;
	va_list ap;
	extern vm_t *__hyb_vm;

	if( (op = ob_get_method( me, (char *)op_name, argc )) == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "class %s does not overload '%s' operator", ob_typename(me), op_name );
	}

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(__hyb_vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}

	op_argc = op->children() - 1;
	op_argc = (op_argc < 0 ? 0 : op_argc);

	/*
	 * The last child of a method is its body itself, so we compare
	 * call children with method->children() - 1 to ignore the body.
	 */
	if( argc != op_argc ){
		hyb_error( H_ET_SYNTAX, "operator '%s' requires %d parameters (called with %d)",
								 op_name,
								 op_argc,
								 argc );
	}

	stack.owner = string(ob_typename(me)) + ":: operator " + string(op_name);

	vm_add_frame( __hyb_vm, &stack );

	stack.insert( "me", me );
	va_start( ap, argc );
	for( i = 0, iitem = op->m_children.head; i < argc; ++i, iitem = iitem->next ){
		value = va_arg( ap, Object * );
		stack.insert( (char *)ll_node( iitem )->value.m_identifier.c_str(), value );
	}
	va_end(ap);

	/* call the operator */
	result = vm_exec( __hyb_vm, &stack, op->body() );

	vm_pop_frame( __hyb_vm );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm_frame( __hyb_vm )->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

/*
 * Call the descriptor 'ds_name' of class 'me' with 'argc' arguments if overloaded,
 * if the descriptor is not overloaded, return NULL if lazy = true or print a
 * syntax error.
 */
Object *class_call_overloaded_descriptor( Object *me, const char *ds_name, bool lazy, int argc, ... ){
	Node    *ds = H_UNDEFINED;
	vframe_t stack;
	ll_item_t *iitem;
	Object  *result = H_UNDEFINED,
			*value  = H_UNDEFINED;
	unsigned int i, ds_argc;
	va_list ap;
	extern vm_t *__hyb_vm;

	if( (ds = ob_get_method( me, (char *)ds_name, argc )) == H_UNDEFINED ){
		if( lazy == false ){
			hyb_error( H_ET_SYNTAX, "class %s does not overload '%s' descriptor", ob_typename(me), ds_name );
		}
		else{
			return H_UNDEFINED;
		}
	}

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(__hyb_vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}

	ds_argc = ds->children() - 1;

	/*
	 * The last child of a method is its body itself, so we compare
	 * call children with method->children() - 1 to ignore the body.
	 */
	if( argc != ds_argc ){
		hyb_error( H_ET_SYNTAX, "descriptor '%s' requires %d parameters (called with %d)",
								 ds_name,
								 ds_argc,
								 argc );
	}

	vm_add_frame( __hyb_vm, &stack );

	/*
	 * Create the "me" reference to the class itself, used inside
	 * methods for me->... calls.
	 */
	stack.owner = string(ob_typename(me)) + "::" + string(ds_name);

	stack.insert( "me", me );
	va_start( ap, argc );
	for( i = 0, iitem = ds->m_children.head; i < argc; ++i, iitem = iitem->next ){
		value = va_arg( ap, Object * );
		stack.insert( (char *)ll_node( iitem )->value.m_identifier.c_str(), value );
	}
	va_end(ap);

	/* call the descriptor */
	result = vm_exec( __hyb_vm, &stack, ds->body() );

	vm_pop_frame( __hyb_vm );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm_frame( __hyb_vm )->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

/** generic function pointers **/
const char *class_typename( Object *o ){
	return ob_class_ucast(o)->name.c_str();
}

Object *class_traverse( Object *me, int index ){
	Class       *cme  = (Class *)me;
	class_attribute_t *attr = (index >= cme->c_attributes.size() ? NULL : cme->c_attributes.at(index));
	return (attr ? attr->value : NULL);
}

Object *class_clone( Object *me ){
    Class *cclone = gc_new_class(),
                *cme    = ob_class_ucast(me);
    ClassAttributeIterator  ai;
    ClassMethodIterator     mi;
    ClassPrototypesIterator pi;
    prototypes_t 				  prototypes;

    itree_foreach( class_attribute_t, ai, cme->c_attributes ){
    	/*
    	 * If the attribute is not static, clone the entire structure.
    	 */
    	if( (*ai)->value->is_static == false ){
			cclone->c_attributes.insert( (char *)(*ai)->value->name.c_str(),
										 new class_attribute_t(
												 (*ai)->value->name,
												 (*ai)->value->access,
												 ob_clone((*ai)->value->value)
										 )
									   );
    	}
    	/*
    	 * Otherwise, insert a reference to the static attribute structure.
    	 */
    	else{
			cclone->c_attributes.insert( (char *)(*ai)->value->name.c_str(), (*ai)->value );
    	}
    }

    itree_foreach( class_method_t, mi, cme->c_methods ){
    	prototypes.clear();
    	vv_foreach( vector<Node *>, pi, (*mi)->value->prototypes ){
    		prototypes.push_back( (*pi)->clone() );
    	}

    	cclone->c_methods.insert( (char *)(*mi)->value->name.c_str(),
								  new class_method_t(
										  (*mi)->value->name,
										  prototypes
								  )
							    );
    }

    cclone->name = cme->name;

    return (Object *)(cclone);
}

size_t class_get_size( Object *me ){
	Object *size = class_call_overloaded_descriptor( me, "__size", false, 0 );
	return ob_ivalue(size);
}

void class_free( Object *me ){
    ClassAttributeIterator ai;
    ClassMethodIterator    mi;
    ClassPrototypesIterator pi;
    Class *cme = ob_class_ucast(me);
    class_method_t *method;
    class_attribute_t *attribute;

    /*
     * Check if the class has a destructors and call it.
     */
    if( (method = cme->c_methods.find( "__expire" )) ){
		vv_foreach( vector<Node *>, pi, method->prototypes ){
			Node *dtor = (*pi);
			vframe_t stack;
			extern vm_t *__hyb_vm;

			stack.owner = string(ob_typename(me)) + "::__expire";
			stack.insert( "me", me );

			vm_add_frame( __hyb_vm, &stack );

			vm_exec( __hyb_vm, &stack, dtor->body() );

			vm_pop_frame( __hyb_vm );
		}
    }
	/*
	 * Delete c_methods structure pointers.
	 */
	itree_foreach( class_method_t, mi, cme->c_methods ){
		method = (*mi)->value;
		delete method;
	}
	cme->c_methods.clear();
	/*
	 * Delete c_attributes structure pointers and decrement values references.
	 */
	itree_foreach( class_attribute_t, ai, cme->c_attributes ){
		attribute = (*ai)->value;
		/*
		 * Static attributes are just references to the main prototype that is
		 * not garbage collected, but it's goin to be freed manually at the end
		 * of the program.
		 */
		if( attribute->is_static == false ){
			delete attribute;
		}
	}
	cme->c_attributes.clear();
}

long class_ivalue( Object *me ){
    return static_cast<long>( class_get_size(me) );
}

double class_fvalue( Object *me ){
    return static_cast<double>( class_get_size(me) );
}

bool class_lvalue( Object *me ){
    return static_cast<bool>( class_get_size(me) );
}

string class_svalue( Object *me ){
	Object *svalue = H_UNDEFINED;

	if( (svalue = class_call_overloaded_descriptor( me, "__to_string", true, 0 )) == H_UNDEFINED ){
		return "<" + ob_class_ucast(me)->name + ">";
	}
	else{
		return ob_svalue(svalue);
	}
}

void class_print( Object *me, int tabs ){
	Object *svalue = class_call_overloaded_descriptor( me, "__to_string", false, 0 );

	ob_print( svalue, tabs );
}

Object *class_range( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "..", 1, op );
}

Object *class_regexp( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "~=", 1, op );
}

/** arithmetic operators **/
Object *class_assign( Object *me, Object *op ){
    class_free(me);

    Object *clone = ob_clone(op);

    return (me = clone);
}

Object *class_increment( Object *me ){
	return class_call_overloaded_operator( me, "++", 0 );
}

Object *class_decrement( Object *me ){
	return class_call_overloaded_operator( me, "--", 0 );
}

Object *class_add( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "+", 1, op );
}

Object *class_sub( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "-", 1, op );
}

Object *class_mul( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "*", 1, op );
}

Object *class_div( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "/", 1, op );
}

Object *class_mod( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "%", 1, op );
}

Object *class_inplace_add( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "+=", 1, op );
}

Object *class_inplace_sub( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "-=", 1, op );
}

Object *class_inplace_mul( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "*=", 1, op );
}

Object *class_inplace_div( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "/=", 1, op );
}

Object *class_inplace_mod( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "%=", 1, op );
}

/** bitwise operators **/
Object *class_bw_and( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "&", 1, op );
}

Object *class_bw_or( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "|", 1, op );
}

Object *class_bw_not( Object *me ){
	return class_call_overloaded_operator( me, "~", 0 );
}

Object *class_bw_xor( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "^", 1, op );
}

Object *class_bw_lshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<<", 1, op );
}

Object *class_bw_rshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">>", 1, op );
}

Object *class_bw_inplace_and( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "&=", 1, op );
}

Object *class_bw_inplace_or( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "|=", 1, op );
}

Object *class_bw_inplace_xor( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "^=", 1, op );
}

Object *class_bw_inplace_lshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<<=", 1, op );
}

Object *class_bw_inplace_rshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">>=", 1, op );
}

/** logic operators **/
Object *class_l_not( Object *me ){
	return class_call_overloaded_operator( me, "!", 0 );
}

Object *class_l_same( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "==", 1, op );
}

Object *class_l_diff( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "!=", 1, op );
}

Object *class_l_less( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<", 1, op );
}

Object *class_l_greater( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">", 1, op );
}

Object *class_l_less_or_same( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<=", 1, op );
}

Object *class_l_greater_or_same( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">=", 1, op );
}

Object *class_l_or( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "||", 1, op );
}

Object *class_l_and( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "&&", 1, op );
}

/* collection operators */
Object *class_cl_push( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "[]=", 1, op );
}

Object *class_cl_at( Object *me, Object *index ){
	return class_call_overloaded_operator( me, "[]", 1, index );
}

Object *class_cl_set( Object *me, Object *index, Object *op ){
	return class_call_overloaded_operator( me, "[]<", 2, index, op );
}

/** class operators **/
void class_define_attribute( Object *me, char *name, access_t access, bool is_static /*= false*/ ){
	Class *cme = ob_class_ucast(me);

	cme->c_attributes.insert( name, new class_attribute_t( name, access, H_VOID_VALUE, is_static ) );
}

access_t class_attribute_access( Object *me, char *name ){
	Class *cme = ob_class_ucast(me);
	class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		return attribute->access;
	}

	return asPublic;
}

bool class_attribute_is_static( Object *me, char *name ){
	Class *cme = ob_class_ucast(me);
	class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		return attribute->is_static;
	}

	return false;
}

void class_set_attribute_access( Object *me, char *name, access_t access ){
	Class *cme = ob_class_ucast(me);
	class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		attribute->access = access;
	}
}

void class_add_attribute( Object *me, char *name ){
	class_define_attribute( me, name, asPublic, false );
}

Object *class_get_attribute( Object *me, char *name, bool with_descriptor /* = true */ ){
    Class *cme = ob_class_ucast(me);
    class_attribute_t *attribute;

    /*
     * If the attribute is defined, return it.
     */
	if( (attribute = cme->c_attributes.find(name)) != H_UNDEFINED ){
		return attribute->value;
	}
	/*
	 * Else, if the class overloads the __attribute descriptor
	 * and with_descriptor = true, call it.
	 */
	else if( with_descriptor ){
		return class_call_overloaded_descriptor( me, "__attribute", true, 1, (Object *)gc_new_string(name) );
	}
	/*
	 * Nothing found.
	 */
	else{
		return NULL;
	}
}

void class_set_attribute_reference( Object *me, char *name, Object *value ){
    Class *cme = ob_class_ucast(me);
    class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		/*
		 * Lock the attribute in case it's static.
		 */
		attribute->lock();
		/*
		 * Set the new value, ob_assign will decrement old value
		 * reference counter.
		 */
		attribute->value = ob_assign( attribute->value, value );
		/*
		 * Unlock it.
		 */
		attribute->unlock();
	}
	else{
		class_call_overloaded_descriptor( me, "__attribute", false, 2, (Object *)gc_new_string(name), value );
	}
}

void class_set_attribute( Object *me, char *name, Object *value ){
    return ob_set_attribute_reference( me, name, ob_clone(value) );
}

void class_define_method( Object *me, char *name, Node *code ){
	Class *cme = ob_class_ucast(me);
	class_method_t *method;

	/*
	 * Check if there's already a method with that name, in this case
	 * push the node to the variations vector.
	 */
	if( (method = cme->c_methods.find(name)) ){
		method->prototypes.push_back( code->clone() );
	}
	/*
	 * Otherwise define a new method.
	 */
	else{
		cme->c_methods.insert( name, new class_method_t( name, code->clone() ) );
	}
}

Node *class_get_method( Object *me, char *name, int argc ){
	Class *cme = ob_class_ucast(me);
	class_method_t *method;

	if( method = cme->c_methods.find(name) ){
		/*
		 * If no parameters number is specified, return the first method found.
		 */
		if( argc < 0 ){
			return (*method->prototypes.begin());
		}
		/*
		 * Otherwise, find the best match.
		 */
		ClassPrototypesIterator pi;
		Node *best_match = NULL;
		int   best_match_argc, match_argc;

		vv_foreach( vector<Node *>, pi, method->prototypes ){
			/*
			 * The last child of a method is its body itself, so we compare
			 * call children with method->children() - 1 to ignore the body.
			 */
			if( best_match == NULL ){
				best_match 		= *pi;
				best_match_argc = best_match->children() - 1;
			}
			else{
				match_argc = (*pi)->children() - 1;
				if( match_argc != best_match_argc && match_argc == argc ){
					return (*pi);
				}
			}
		}

		return best_match;
	}
	else{
		return NULL;
	}
}

Object *class_call_method( vm_t *vm, vframe_t *frame, Object *me, char *me_id, char *method_id, Node *argv ){
	size_t 	 method_argc,
			 i,
		 	 argc   = argv->children();
	ll_item_t *aitem, *iitem;
	Object  *value  = H_UNDEFINED,
			*result = H_UNDEFINED;
	Node    *method = class_get_method( me, method_id, argc );
	vframe_t stack;

	/*
	 * Method not found.
	 */
	if( method == H_UNDEFINED ){
		/*
		 * Try to call the __method descriptor if it's overloaded.
		 */
		result = class_call_undefined_method( vm, me, me_id, method_id, argv );
		if( result == H_UNDEFINED ){
			hyb_error( H_ET_SYNTAX, "'%s' is not a method of object '%s'", method_id, ob_typename(me) );
		}
		else{
			return result;
		}
	}

	/*
	 * If the method access specifier is not asPublic, only the identifier
	 * "me" can use the method.
	 */
	if( method->value.m_access != asPublic && strcmp( me_id, "me" ) != 0 ){
		/*
		 * The method is protected.
		 */
		if( method->value.m_access == asProtected ){
			hyb_error( H_ET_SYNTAX, "Protected method '%s' can be accessed only by derived classes of '%s'", method_id, ob_typename(me) );
		}
		/*
		 * The method is private..
		 */
		else{
			hyb_error( H_ET_SYNTAX, "Private method '%s' can be accessed only within '%s' class", method_id, ob_typename(me) );
		}
	}

	/*
	 * The last child of a method is its body itself, so we compare
	 * call children with method->children() - 1 to ignore the body.
	 */
	method_argc = method->children() - 1;
	method_argc = (method_argc < 0 ? 0 : method_argc);

	if( method->value.m_vargs ){
		if( argc < method_argc ){
			hyb_error( H_ET_SYNTAX, "method '%s' requires at least %d parameters (called with %d)",
									method_id,
									method_argc,
									argc );
	   }
	}
	else{
		if( argc != method_argc ){
			hyb_error( H_ET_SYNTAX, "method '%s' requires %d parameters (called with %d)",
									 method_id,
									 method_argc,
									 argc );
		}
	}

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		return vm_raise_exception( "Reached max number of nested calls" );
	}
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );
	/*
	 * Set the stack owner
	 */
	stack.owner = ob_typename(me) + string("::") + method_id;
	/*
	 * Static methods can not use 'me' instance.
	 */
	if( method->value.m_static == false ){
		stack.insert( "me", me );
	}
	/*
	 * Evaluate each object and insert it into the stack
	 */
	for( i = 0, aitem = argv->m_children.head, iitem = method->m_children.head; i < argc; ++i, aitem = aitem->next ){
		value = vm_exec( vm, frame, ll_node( aitem ) );
		/*
		 * Check if vm_exec raised an exception.
		 */
		if( frame->state.is(Exception) || frame->state.is(Return) ){
			vm_pop_frame( vm );
			return frame->state.value;
	    }
		/*
		 * Check if i >= argc for vargs methods :
		 *
		 * i.e. public method foo( bar, ... ){ }
		 */
		if( i >= argc ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ll_node(iitem)->value.m_identifier.c_str(), value );
			iitem = iitem->next;
		}
	}
	/* execute the method */
	result = vm_exec( vm, &stack, method->body() );

	/*
	 * Dismiss the stack.
	 */
	vm_pop_frame( vm );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

IMPLEMENT_TYPE(Class) {
    /** type code **/
    otClass,
	/** type name **/
    "class",
	/** type basic size **/
    OB_COLLECTION_SIZE,
    /** type builtin methods **/
    NO_BUILTIN_METHODS,
	/** generic function pointers **/
    class_typename, // type_name
    class_traverse, // traverse
	class_clone, // clone
	class_free, // free
	class_get_size, // get_size
	0, // serialize
	0, // deserialize
	0, // to_fd
	0, // from_fd
	0, // cmp
	class_ivalue, // ivalue
	class_fvalue, // fvalue
	class_lvalue, // lvalue
	class_svalue, // svalue
	class_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	class_range, // range
	class_regexp, // regexp

	/** arithmetic operators **/
	class_assign, // assign
    0, // factorial
    class_increment, // increment
    class_decrement, // decrement
    0, // minus
    class_add, // add
    class_sub, // sub
    class_mul, // mul
    class_div, // div
    class_mod, // mod
    class_inplace_add, // inplace_add
    class_inplace_sub, // inplace_sub
    class_inplace_mul, // inplace_mul
    class_inplace_div, // inplace_div
    class_inplace_mod, // inplace_mod

	/** bitwise operators **/
	class_bw_and, // bw_and
    class_bw_or, // bw_or
    class_bw_not, // bw_not
    class_bw_xor, // bw_xor
    class_bw_lshift, // bw_lshift
    class_bw_rshift, // bw_rshift
    class_bw_inplace_and, // bw_inplace_and
    class_bw_inplace_or, // bw_inplace_or
    class_bw_inplace_xor, // bw_inplace_xor
    class_bw_inplace_lshift, // bw_inplace_lshift
    class_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    class_l_not, // l_not
    class_l_same, // l_same
    class_l_diff, // l_diff
    class_l_less, // l_less
    class_l_greater, // l_greater
    class_l_less_or_same, // l_less_or_same
    class_l_greater_or_same, // l_greater_or_same
    class_l_or, // l_or
    class_l_and, // l_and

	/** collection operators **/
	class_cl_push, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	class_cl_at, // cl_at
	class_cl_set, // cl_set
	0, // cl_set_reference

	/** structure operators **/
	class_define_attribute, // define_attribute
	class_attribute_access, // attribute_access
	class_attribute_is_static, // attribute_is_static
	class_set_attribute_access, // set_attribute_access
    class_add_attribute, // add_attribute
    class_get_attribute, // get_attribute
    class_set_attribute, // set_attribute
    class_set_attribute_reference,  // set_attribute_reference
    class_define_method, // define_method
    class_get_method,  // get_method
    class_call_method  // call_method
};

