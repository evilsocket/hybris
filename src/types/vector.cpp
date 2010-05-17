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
#include "vm.h"

/** builtin methods **/
Object *__vector_size( engine_t *engine, Object *me, vframe_t *data ){
	return (Object *)gc_new_integer( ob_vector_ucast(me)->items );
}

Object *__vector_pop( engine_t *engine, Object *me, vframe_t *data ){
	return (Object *)ob_cl_pop( me );
}

Object *__vector_remove( engine_t *engine, Object *me, vframe_t *data ){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "method 'remove' requires 1 parameter (called with %d)", ob_argc() );
	}

	return (Object *)ob_cl_remove( me, ob_argv(0) );
}

Object *__vector_contains( engine_t *engine, Object *me, vframe_t *data ){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "method 'contains' requires 1 parameter (called with %d)", ob_argc() );
	}

	Object *array = me,
		   *find  = ob_argv(0);
	IntegerObject index(0);
	unsigned int size( ob_get_size(array) );

	for( ; index.value < size; ++index.value ){
		if( ob_cmp( ob_cl_at( array, (Object *)&index ), find ) == 0 ){
			return (Object *)gc_new_boolean(true);
		}
	}

	return (Object *)gc_new_boolean(false);
}

Object *__vector_join( engine_t *engine, Object *me, vframe_t *data ){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "method 'join' requires 1 parameter (called with %d)", ob_argc() );
	}

	Object *array = me;
	string  glue  = ob_svalue( ob_argv(0) ),
			join;
	unsigned int i, items(ob_vector_ucast(array)->items);

	for( i = 0; i < items; ++i ){
		join += ob_svalue(ob_vector_ucast(array)->value[i]) + ( i < items - 1 ? glue : "");
	}

	return (Object *)gc_new_string(join.c_str());
}

/** generic function pointers **/
const char *vector_typename( Object *o ){
	return o->type->name;
}

Object *vector_traverse( Object *me, int index ){
	return (index >= ((VectorObject *)me)->value.size() ? NULL : ((VectorObject *)me)->value.at(index));
}

Object *vector_clone( Object *me ){
    VectorObjectIterator i;
    VectorObject *vclone = gc_new_vector(),
                 *vme    = ob_vector_ucast(me);

    for( i = vme->value.begin(); i != vme->value.end(); i++ ){
        ob_cl_push_reference( (Object *)vclone, ob_clone( *i ) );
    }

    return (Object *)vclone;
}

void vector_free( Object *me ){
    VectorObject *vme = ob_vector_ucast(me);

    vme->items = 0;
    vme->value.clear();
}

size_t vector_get_size( Object *me ){
	return ob_vector_ucast(me)->items;
}

Object *vector_to_fd( Object *o, int fd, size_t size ){
	size_t i, s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o));
	int    written(0);

	for( i = 0; i < s; ++i ){
		written += ob_int_val( ob_to_fd( ob_vector_ucast(o)->value[i], fd, 0 ) );
	}

	return ob_dcast( gc_new_integer(written) );
}

int vector_cmp( Object *me, Object *cmp ){
    if( !ob_is_vector(cmp) ){
        return 1;
    }
    else {
        VectorObject *vme  = ob_vector_ucast(me),
                     *vcmp = ob_vector_ucast(cmp);
        size_t        vme_size( vme->value.size() ),
                      vcmp_size( vcmp->value.size() );

        if( vme_size > vcmp_size ){
            return 1;
        }
        if( vme_size < vcmp_size ){
            return -1;
        }
        /*
         * Same type and same size, let's check the elements.
         */
        else{
            size_t i;
            int    diff;

            for( i = 0; i < vme_size; ++i ){
                diff = ob_cmp( vme->value[i], vcmp->value[i] );
                if( diff != 0 ){
                    return diff;
                }
            }
            return 0;
        }
    }
}

long vector_ivalue( Object *me ){
    return static_cast<long>( ob_vector_ucast(me)->items );
}

double vector_fvalue( Object *me ){
    return static_cast<double>( ob_vector_ucast(me)->items );
}

bool vector_lvalue( Object *me ){
    return static_cast<bool>( ob_vector_ucast(me)->items );
}

string vector_svalue( Object *o ){
	return string( "<vector>" );
}

void vector_print( Object *me, int tabs ){
    VectorObjectIterator i;
    VectorObject *vme = ob_vector_ucast(me);
    Object       *item;
    int           j;

    for( j = 0; j < tabs; ++j ){
        fprintf( stdout, "\t" );
    }
    fprintf( stdout, "array {\n" );
    for( i = vme->value.begin(); i != vme->value.end(); i++ ){
        item = *i;
        ob_print( item, tabs + 1 );
        fprintf( stdout, "\n" );
    }
    for( j = 0; j < tabs; ++j ) fprintf( stdout, "\t" );
    fprintf( stdout, "}\n" );
}

/** arithmetic operators **/
Object *vector_assign( Object *me, Object *op ){
    /*
     * Decrement my items reference count (not mine).
     */
    vector_free(me);

    Object *clone = ob_clone(op);

    return clone;
}

/** collection operators **/
Object *vector_cl_push( Object *me, Object *o ){
    return ob_cl_push_reference( me, ob_clone(o) );
}

Object *vector_cl_push_reference( Object *me, Object *o ){
    ob_vector_ucast(me)->value.push_back( o );
    ob_vector_ucast(me)->items++;

    return me;
}

Object *vector_cl_pop( Object *me ){
    size_t last_idx = ob_vector_ucast(me)->items - 1;

    if( last_idx < 0 ){
        hyb_error( H_ET_GENERIC, "could not pop an element from an empty array" );
    }

    Object *last_item = ob_vector_ucast(me)->value[last_idx];
    ob_vector_ucast(me)->value.pop_back();
    ob_vector_ucast(me)->items--;

    return last_item;
}

Object *vector_cl_remove( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_vector_ucast(me)->items ){
        hyb_error( H_ET_GENERIC, "index out of bounds" );
    }

    Object *item = ob_vector_ucast(me)->value[idx];
    ob_vector_ucast(me)->value.erase( ob_vector_ucast(me)->value.begin() + idx );
    ob_vector_ucast(me)->items--;

    return item;
}

Object *vector_cl_at( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_vector_ucast(me)->items ){
    	hyb_error( H_ET_GENERIC, "index out of bounds" );
    }

    return ob_vector_ucast(me)->value[idx];
}

Object *vector_cl_set( Object *me, Object *i, Object *v ){
    return ob_cl_set_reference( me, i, ob_clone(v) );
}

Object *vector_cl_set_reference( Object *me, Object *i, Object *v ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_vector_ucast(me)->items ){
    	hyb_error( H_ET_GENERIC, "index out of bounds" );
    }

    Object *old = ob_vector_ucast(me)->value[idx];

    ob_free(old);

    ob_vector_ucast(me)->value[idx] = v;

    return me;
}

Object *vector_call_method( engine_t *engine, vframe_t *frame, Object *me, char *me_id, char *method_id, Node *argv ){
	ob_type_builtin_method_t *method = NULL;

	if( (method = ob_get_builtin_method( me, method_id )) == NULL ){
		hyb_error( H_ET_SYNTAX, "Vector type does not have a '%s' method", method_id );
	}

	Object  *value,
			*result;
	vframe_t stack;
	size_t   i, argc = argv->children();

	stack.owner = ob_typename(me) + string("::") + method_id;
	/*
	 * Evaluate each object and insert it into the stack
	 */
	for( i = 0; i < argc; ++i ){
		value = engine_exec( engine, frame, argv->child(i) );
		stack.push( value );
	}
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );

	/* execute the method */
	result = ((ob_type_builtin_method_t)method)( engine, me, &stack );

	/*
	 * Dismiss the stack.
	 */
	vm_pop_frame( engine->vm );

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

static ob_builtin_methods_t __vector_builtin_methods;

IMPLEMENT_TYPE(Vector) {
    /** type code **/
    otVector,
	/** type name **/
    "vector",
	/** type basic size **/
    OB_COLLECTION_SIZE,
    /** type builtin methods **/
    {
		{ "size",     (ob_type_builtin_method_t *)__vector_size },
		{ "pop",      (ob_type_builtin_method_t *)__vector_pop },
		{ "remove",   (ob_type_builtin_method_t *)__vector_remove },
		{ "contains", (ob_type_builtin_method_t *)__vector_contains },
		{ "join",     (ob_type_builtin_method_t *)__vector_join },
		{ OB_BUILIN_METHODS_END_MARKER }
    },

	/** generic function pointers **/
    vector_typename, // type_name
    vector_traverse, // traverse
	vector_clone, // clone
	vector_free, // free
	vector_get_size, // get_size
	0, // serialize
	0, // deserialize
	vector_to_fd, // to_fd
	0, // from_fd
	vector_cmp, // cmp
	vector_ivalue, // ivalue
	vector_fvalue, // fvalue
	vector_lvalue, // lvalue
	vector_svalue, // svalue
	vector_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	0, // range
	0, // regexp

	/** arithmetic operators **/
	vector_assign, // assign
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
	vector_cl_push, // cl_push
	vector_cl_push_reference, // cl_push_reference
	vector_cl_pop, // cl_pop
	vector_cl_remove, // cl_remove
	vector_cl_at, // cl_at
	vector_cl_set, // cl_set
	vector_cl_set_reference, // cl_set_reference

	/** structure operators **/
	0, // define_attribute
	0, // attribute_access
	0, // attribute_is_static
	0, // set_attribute_access
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference
    0, // define_method
    0, // get_method
    vector_call_method  // call_method
};

