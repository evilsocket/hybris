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
#include "common.h"
#include "types.h"
#include "node.h"
#include "vmem.h"
#include "context.h"

/** generic function pointers **/
const char *class_typename( Object *o ){
	return ob_class_ucast(o)->name.c_str();
}

void class_set_references( Object *me, int ref ){
    ClassObjectAttributeIterator ai;
    ClassObject *cme = ob_class_ucast(me);

    me->ref += ref;

    for( ai = cme->c_attributes.begin(); ai != cme->c_attributes.end(); ai++ ){
        ob_set_references( (*ai)->value->value, ref );
    }
}

Object *class_clone( Object *me ){
    ClassObject *cclone = gc_new_class(),
                *cme    = ob_class_ucast(me);
    ClassObjectAttributeIterator ai;
    ClassObjectMethodIterator    mi;
    ClassObjectMethodVariationsIterator mvi;

    vector<Node *>				 method_variations;

    for( ai = cme->c_attributes.begin(); ai != cme->c_attributes.end(); ai++ ){
    	cclone->c_attributes.insert( (char *)(*ai)->value->name.c_str(),
									 new class_attribute_t(
									   (*ai)->value->name,
									   (*ai)->value->access,
									   ob_clone((*ai)->value->value)
							       )
								 );
    }

    for( mi = cme->c_methods.begin(); mi != cme->c_methods.end(); mi++ ){
    	method_variations.clear();
    	for( mvi = (*mi)->value->method.begin(); mvi != (*mi)->value->method.end(); mvi++ ){
    		method_variations.push_back( (*mvi)->clone() );
    	}

    	cclone->c_methods.insert( (char *)(*mi)->value->name.c_str(),
								  new class_method_t(
									(*mi)->value->name,
									method_variations
								  )
							    );
    }

    cclone->items = cme->items;
    cclone->name  = cme->name;

    return (Object *)(cclone);
}

size_t class_get_size( Object *me ){
	return ob_class_ucast(me)->items;
}

void class_free( Object *me ){
    ClassObjectAttributeIterator ai;
    ClassObjectMethodIterator    mi;
    ClassObjectMethodVariationsIterator mvi;
    ClassObject *cme = ob_class_ucast(me);
    class_method_t *method;
    class_attribute_t *attribute;

    /*
     * Check if the class has a destructors and call it.
     */
    if( (method = cme->c_methods.find( "__expire" )) ){
		for( mvi = method->method.begin(); mvi != method->method.end(); mvi++ ){
			Node *dtor = (*mvi);
			vframe_t stack;
			extern Context __context;

			stack.insert( "me", me );

			__context.trace( "__expire", &stack );

			__context.engine->exec( &stack, dtor->callBody() );

			__context.detrace();
		}
    }
	/*
	 * Delete c_methods structure pointers.
	 */
	for( mi = cme->c_methods.begin(); mi != cme->c_methods.end(); mi++ ){
		method = (*mi)->value;
		delete method;
	}
	cme->c_methods.clear();
	/*
	 * Delete c_attributes structure pointers and decrement values references.
	 */
	for( ai = cme->c_attributes.begin(); ai != cme->c_attributes.end(); ai++ ){
		attribute = (*ai)->value;
		if( attribute->value ){
			ob_free( attribute->value );
		}
		delete attribute;
	}
	cme->c_attributes.clear();

    cme->items = 0;
}

long class_ivalue( Object *me ){
    return static_cast<long>( ob_class_ucast(me)->items );
}

double class_fvalue( Object *me ){
    return static_cast<double>( ob_class_ucast(me)->items );
}

bool class_lvalue( Object *me ){
    return static_cast<bool>( ob_class_ucast(me)->items );
}

string class_svalue( Object *me ){
    return "<" + ob_class_ucast(me)->name + ">";
}

void class_print( Object *me, int tabs ){
	ClassObject *cme = ob_class_ucast(me);
	ClassObjectAttributeIterator ai;
	ClassObjectMethodIterator 	 mi;
	int j;

	for( j = 0; j < tabs; ++j ){
		printf( "\t" );
	}
	printf( "class {\n" );
	for( ai = cme->c_attributes.begin(); ai != cme->c_attributes.end(); ai++ ){
		for( j = 0; j < tabs + 1; ++j ) printf( "\t" );
		printf( "%s %s -> ", (*ai)->value->value->type->name, (*ai)->value->name.c_str() );
		ob_print( (*ai)->value->value, tabs + 1 );
		printf( "\n" );
	}
	printf( "\n" );
	for( mi = cme->c_methods.begin(); mi != cme->c_methods.end(); mi++ ){
		for( j = 0; j < tabs + 1; ++j ) printf( "\t" );
		if( (*mi)->value->name.find("__op@") == 0 ){
			printf( "operator %s { ... }\n", (*mi)->value->name.c_str() + strlen("__op@") );
		}
		else{
			printf( "method %s { ... }\n", (*mi)->value->name.c_str() );
		}
	}
	for( j = 0; j < tabs; ++j ) printf( "\t" );
	printf( "}\n" );
}

/** arithmetic operators **/
Object *class_assign( Object *me, Object *op ){
    class_free(me);

    Object *clone = ob_clone(op);

    ob_set_references( clone, +1 );

    return (me = clone);
}

/** structure operators **/
access_t class_attribute_access( Object *me, char *name ){
	ClassObject *cme = ob_class_ucast(me);
	class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		return attribute->access;
	}

	return asPublic;
}

void class_set_attribute_access( Object *me, char *name, access_t a ){
	ClassObject *cme = ob_class_ucast(me);
	class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		attribute->access = a;
	}
}

void class_add_attribute( Object *me, char *name ){
    ClassObject *cme = ob_class_ucast(me);

    cme->c_attributes.insert( name,
							  new class_attribute_t(
									  name,
									  asPublic,
									  (Object *)gc_new_integer(0)
							  )
							);
    cme->items++;
}

Object *class_get_attribute( Object *me, char *name ){
    ClassObject *cme = ob_class_ucast(me);
    class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		return attribute->value;
	}

    return NULL;
}

void class_set_attribute_reference( Object *me, char *name, Object *value ){
    ClassObject *cme = ob_class_ucast(me);
    class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		/*
		 * Set the new value, ob_assign will decrement old value
		 * reference counter.
		 */
		attribute->value = ob_assign( attribute->value, value );
	}
	else{
		cme->c_attributes.insert( name,
								  new class_attribute_t(
										  name,
										  asPublic,
										  value
								  )
								);
		cme->items++;
	}
}

void class_set_attribute( Object *me, char *name, Object *value ){
    return ob_set_attribute_reference( me, name, ob_clone(value) );
}

void class_define_method( Object *me, char *name, Node *code ){
	ClassObject *cme = ob_class_ucast(me);
	class_method_t *method;
	/*
	 * Check if there's already a method with that name, in this case
	 * push the node to the variations vector.
	 */
	if( (method = cme->c_methods.find(name)) ){
		method->method.push_back( code->clone() );
	}
	/*
	 * Otherwise define a new method.
	 */
	else{
		cme->c_methods.insert( name, new class_method_t( name, code->clone() ) );
	}
	/*
	 * In both cases increment item counter.
	 */
	cme->items++;
}

Node *class_get_method( Object *me, char *name, int argc ){
	ClassObject *cme = ob_class_ucast(me);
	class_method_t *method;

	if( (method = cme->c_methods.find(name)) ){
		/*
		 * If no parameters number is specified, return the first method found.
		 */
		if( argc < 0 ){
			return (*method->method.begin());
		}
		/*
		 * Otherwise, find the best match.
		 */
		ClassObjectMethodVariationsIterator mvi;
		Node *best_match = NULL;
		int   best_match_argc, match_argc;

		for( mvi = method->method.begin(); mvi != method->method.end(); mvi++ ){
			/*
			 * The last child of a method is its body itself, so we compare
			 * call children with method->children() - 1 to ignore the body.
			 */
			if( best_match == NULL ){
				best_match 		= *mvi;
				best_match_argc = best_match->children() - 1;
			}
			else{
				match_argc = (*mvi)->children() - 1;
				if( match_argc != best_match_argc && match_argc == argc ){
					return (*mvi);
				}
			}
		}

		return best_match;
	}
	else{
		/*
		 * Try with overloaded operators.
		 */
		char mangled_op_name[0xFF] = {0};
		sprintf( mangled_op_name, "__op@%s", name );

		return class_get_method( me, mangled_op_name, argc );
	}
}

IMPLEMENT_TYPE(Class) {
    /** type code **/
    otClass,
	/** type name **/
    "class",
	/** type basic size **/
    0,

	/** generic function pointers **/
    class_typename, // type_name
	class_set_references, // set_references
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
	0, // from_int
	0, // from_float
	0, // range
	0, // regexp

	/** arithmetic operators **/
	class_assign, // assign
    0, // factorial
    0, // increment
    0, // decrement
    0, // minus
    0, // add
    0, // sub
    0, // mul
    0, // div
    0, // mod
    0, // inplace_add
    0, // inplace_sub
    0, // inplace_mul
    0, // inplace_div
    0, // inplace_mod

	/** bitwise operators **/
	0, // bw_and
    0, // bw_or
    0, // bw_not
    0, // bw_xor
    0, // bw_lshift
    0, // bw_rshift
    0, // bw_inplace_and
    0, // bw_inplace_or
    0, // bw_inplace_xor
    0, // bw_inplace_lshift
    0, // bw_inplace_rshift

	/** logic operators **/
    0, // l_not
    0, // l_same
    0, // l_diff
    0, // l_less
    0, // l_greater
    0, // l_less_or_same
    0, // l_greater_or_same
    0, // l_or
    0, // l_and

	/** collection operators **/
	0, // cl_concat
	0, // cl_inplace_concat
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	0, // cl_at
	0, // cl_set
	0, // cl_set_reference

	/** structure operators **/
	class_attribute_access, // attribute_access
	class_set_attribute_access, // set_attribute_access
    class_add_attribute, // add_attribute
    class_get_attribute, // get_attribute
    class_set_attribute, // set_attribute
    class_set_attribute_reference,  // set_attribute_reference
    class_define_method, // define_method
    class_get_method  // get_method
};

