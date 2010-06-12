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

/** builtin methods **/
Object *__vector_size( vm_t *vm, Object *me, vframe_t *data ){
	return (Object *)gc_new_integer( ob_vector_ucast(me)->items );
}

Object *__vector_pop( vm_t *vm, Object *me, vframe_t *data ){
	return (Object *)ob_cl_pop( me );
}

Object *__vector_remove( vm_t *vm, Object *me, vframe_t *data ){
	if( vm_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "method 'remove' requires 1 parameter (called with %d)", vm_argc() );
	}

	return (Object *)ob_cl_remove( me, vm_argv(0) );
}

Object *__vector_contains( vm_t *vm, Object *me, vframe_t *data ){
	if( vm_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "method 'contains' requires 1 parameter (called with %d)", vm_argc() );
	}

	Object *array = me,
		   *find  = vm_argv(0);
	Integer index(0);
	int size( ob_get_size(array) );

	for( ; index.value < size; ++index.value ){
		if( ob_cmp( ob_cl_at( array, (Object *)&index ), find ) == 0 ){
			return (Object *)gc_new_boolean(true);
		}
	}

	return (Object *)gc_new_boolean(false);
}

Object *__vector_unique( vm_t *vm, Object *me, vframe_t *data ){
	Object *array  = me,
		   *unique = (Object *)gc_new_vector(),
		   *item;
	Integer index(0);
	int size( ob_get_size(array) );
	bool contains;

	for( ; index.value < size; ++index.value ){
		item 	 = ob_cl_at( array, (Object *)&index );
		contains = false;

		Integer index_u(0);
		int size_u( ob_get_size(unique) );

		for( ; index_u.value < size_u && !contains; ++index_u.value ){
			if( ob_cmp( ob_cl_at( unique, (Object *)&index_u ), item ) == 0 ){
				contains = true;
			}
		}

		if( contains == false ){
			ob_cl_push( unique, item );
		}
	}

	return unique;
}

Object *__vector_join( vm_t *vm, Object *me, vframe_t *data ){
	if( vm_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "method 'join' requires 1 parameter (called with %d)", vm_argc() );
	}

	Object *array = me;
	string  glue  = ob_svalue( vm_argv(0) ),
			join;
	unsigned int i, items(ob_vector_ucast(array)->items);

	for( i = 0; i < items; ++i ){
		join += ob_svalue(ob_vector_ucast(array)->value[i]) + ( i < items - 1 ? glue : "");
	}

	return (Object *)gc_new_string(join.c_str());
}

Object *__vector_max( vm_t *vm, Object *me, vframe_t *data ){
	Object *array = me,
		   *obj,
		   *max   = NULL;
	Integer index(0);
	int size( ob_get_size(array) );

	for( ; index.value < size; ++index.value ){
		obj = ob_cl_at( array, (Object *)&index );
		if( max == NULL || ob_cmp( obj, max ) == 1 ){
			max = obj;
		}
	}

	return ob_clone(max);
}

Object *__vector_min( vm_t *vm, Object *me, vframe_t *data ){
	Object *array = me,
		   *obj,
		   *min   = NULL;
	Integer index(0);
	int size( ob_get_size(array) );

	for( ; index.value < size; ++index.value ){
		obj = ob_cl_at( array, (Object *)&index );
		if( min == NULL || ob_cmp( obj, min ) == -1 ){
			min = obj;
		}
	}

	return ob_clone(min);
}

/** generic function pointers **/
Object *vector_traverse( Object *me, int index ){
	return ((unsigned)index >= ((Vector *)me)->items ? NULL : ((Vector *)me)->value.at(index));
}

Object *vector_clone( Object *me ){
    VectorIterator i;
    Vector *vclone = gc_new_vector(),
           *vme    = ob_vector_ucast(me);

    vv_foreach( vector<Object *>, i, vme->value ){
        ob_cl_push_reference( (Object *)vclone, ob_clone( *i ) );
    }

    return (Object *)vclone;
}

void vector_free( Object *me ){
    Vector *vme = ob_vector_ucast(me);

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
        Vector *vme  = ob_vector_ucast(me),
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
    VectorIterator i;
    Vector *vme = ob_vector_ucast(me);
    Object       *item;
    int           j;

    for( j = 0; j < tabs; ++j ){
        fprintf( stdout, "\t" );
    }
    fprintf( stdout, "array {\n" );
    vv_foreach( vector<Object *>, i, vme->value ){
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

    return me = clone;
}

Object *vector_add( Object *me, Object *op ){
	Object *clone = ob_clone(me);

	if( ob_is_vector(op) ){
		size_t i, sz( ob_vector_ucast(op)->items );
		for( i = 0; i < sz; ++i ){
			ob_cl_push( clone, ob_vector_ucast(op)->value[i] );
		}
	}
	else{
		ob_cl_push( clone, op );
	}

	return clone;
}

Object *vector_sub( Object *me, Object *op ){
	Object *clone = ob_clone(me);

	if( ob_is_vector(op) ){
		Vector *vclone = ob_vector_ucast(clone),
			   *vop    = ob_vector_ucast(op);
		size_t i, sz_op( vop->items );
		VectorIterator vi( vclone->value.begin() );

		for( i = 0; i < sz_op; ++i ){
			while( vi != vclone->value.end() ){
				if( ob_cmp( *vi, vop->value[i] ) == 0 ){
					vclone->value.erase( vi );
					vclone->items--;
					/*
					 * We've just erased an item, so we have to reset begin and end
					 * pointers.
					 */
					vi = vclone->value.begin();
				}
				else{
					vi++;
				}
			}
		}
	}
	else{
		Vector *vclone = ob_vector_ucast(clone);
		VectorIterator vi( vclone->value.begin() );

		vi = vclone->value.begin();
		while( vi != vclone->value.end() ){
			if( ob_cmp( *vi, op ) == 0 ){
				vclone->value.erase( vi );
				vclone->items--;
				/*
				 * We've just erased an item, so we have to reset begin and end
				 * pointers.
				 */
				vi = vclone->value.begin();
			}
			else{
				vi++;
			}
		}
	}

	return clone;
}

Object *vector_inplace_add( Object *me, Object *op ){
	if( ob_is_vector(op) ){
		size_t i, sz( ob_vector_ucast(op)->items );
		for( i = 0; i < sz; ++i ){
			ob_cl_push( me, ob_vector_ucast(op)->value[i] );
		}
	}
	else{
		ob_cl_push( me, op );
	}

	return me;
}

Object *vector_inplace_sub( Object *me, Object *op ){
	if( ob_is_vector(op) ){
		Vector *vme = ob_vector_ucast(vme),
			   *vop = ob_vector_ucast(op);
		size_t i, sz_op( vop->items );
		VectorIterator vi( vme->value.begin() );

		for( i = 0; i < sz_op; ++i ){
			while( vi != vme->value.end() ){
				if( ob_cmp( *vi, vop->value[i] ) == 0 ){
					vme->value.erase( vi );
					/*
					 * We've just erased an item, so we have to reset begin and end
					 * pointers.
					 */
					vi = vme->value.begin();
					vme->items--;
				}
				else{
					vi++;
				}
			}
		}
	}
	else{
		Vector *vme = ob_vector_ucast(me);
		VectorIterator vi( vme->value.begin() );

		vi = vme->value.begin();
		while( vi != vme->value.end() ){
			if( ob_cmp( *vi, op ) == 0 ){
				vme->value.erase( vi );
				/*
				 * We've just erased an item, so we have to reset begin and end
				 * pointers.
				 */
				vi = vme->value.begin();
				vme->items--;
			}
			else{
				vi++;
			}
		}
	}

	return me;
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
    int last_idx = ob_vector_ucast(me)->items - 1;
    if( last_idx < 0 ){
    	return vm_raise_exception( "could not pop an element from an empty array" );
    }

    Object *last_item = ob_vector_ucast(me)->value[last_idx];
    ob_vector_ucast(me)->value.pop_back();
    ob_vector_ucast(me)->items--;

    return last_item;
}

Object *vector_cl_remove( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_vector_ucast(me)->items ){
    	return vm_raise_exception( "index out of bounds" );
    }

    Object *item = ob_vector_ucast(me)->value[idx];
    ob_vector_ucast(me)->value.erase( ob_vector_ucast(me)->value.begin() + idx );
    ob_vector_ucast(me)->items--;

    return item;
}

Object *vector_cl_at( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_vector_ucast(me)->items ){
    	return vm_raise_exception( "index out of bounds" );
    }

    return ob_vector_ucast(me)->value[idx];
}

Object *vector_cl_set( Object *me, Object *i, Object *v ){
    return ob_cl_set_reference( me, i, ob_clone(v) );
}

Object *vector_cl_set_reference( Object *me, Object *i, Object *v ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_vector_ucast(me)->items ){
    	return vm_raise_exception( "index out of bounds" );
    }

    Object *old = ob_vector_ucast(me)->value[idx];

    ob_free(old);

    ob_vector_ucast(me)->value[idx] = v;

    return me;
}

Object *vector_call_method( vm_t *vm, vframe_t *frame, Object *me, char *me_id, char *method_id, Node *argv ){
	ob_type_builtin_method_t *method = NULL;

	if( (method = ob_get_builtin_method( me, method_id )) == NULL ){
		hyb_error( H_ET_SYNTAX, "Vector type does not have a '%s' method", method_id );
	}

	ll_item_t *iitem;
	Object  *value,
			*result;
	vframe_t stack;
	size_t   i, argc = argv->children.items;

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );

	stack.owner = ob_typename(me) + string("::") + method_id;
	/*
	 * Evaluate each object and insert it into the stack
	 */
	ll_foreach_to( &argv->children, iitem, i, argc ){
		value = vm_exec( vm, frame, ll_node( iitem ) );

		if( frame->state.is(Exception) ){
			vm_pop_frame( vm );
			return frame->state.e_value;
		}
		else if( frame->state.is(Return) ){
			vm_pop_frame( vm );
			return frame->state.r_value;
		}

		stack.push( value );
	}

	/* execute the method */
	result = ((ob_type_builtin_method_t)method)( vm, me, &stack );

	/*
	 * Dismiss the stack.
	 */
	vm_pop_frame( vm );

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

static ob_builtin_method_t vector_builtin_methods[] = {
	{ "size",     (ob_type_builtin_method_t *)__vector_size },
	{ "pop",      (ob_type_builtin_method_t *)__vector_pop },
	{ "remove",   (ob_type_builtin_method_t *)__vector_remove },
	{ "contains", (ob_type_builtin_method_t *)__vector_contains },
	{ "join",     (ob_type_builtin_method_t *)__vector_join },
	{ "min",	  (ob_type_builtin_method_t *)__vector_min },
	{ "max",	  (ob_type_builtin_method_t *)__vector_max },
	{ "unique",   (ob_type_builtin_method_t *)__vector_unique },
	{ OB_BUILIN_METHODS_END_MARKER }
};

IMPLEMENT_TYPE(Vector) {
    /** type code **/
    otVector,
	/** type name **/
    "vector",
	/** type basic size **/
    OB_COLLECTION_SIZE,
    /** type builtin methods **/
    vector_builtin_methods,
	/** generic function pointers **/
    0, // type_name
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
    vector_add, // add
    vector_sub, // sub
    0, // mul
    0, // div
    0, // mod
    vector_inplace_add, // inplace_add
    vector_inplace_sub, // inplace_sub
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

