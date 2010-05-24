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

/** helpers **/
INLINE ob_type_builtin_method_t *ob_get_builtin_method( Object *c, char *method_id ){
	size_t 				  i;
	ob_builtin_methods_t *methods = c->type->builtin_methods;
	/*
	 * Builtin methods are supposed to be only a few, so a loop with a string
	 * comparision is faster than an hashmap/asciitree initialization and
	 * further search.
	 */
	for( i = 0; methods[i].method != NULL; ++i ){
		if( methods[i].name == method_id ){
			return methods[i].method;
		}
	}

	return NULL;
}

int map_find( Object *m, Object *key ){
    Map *mm = (Map *)m;
    size_t     i;

	for( i = 0; i < mm->items; ++i ){
		if( ob_cmp( mm->keys[i], key ) == 0 ){
			return i;
		}
	}
	return -1;
}

/** builtin methods **/
Object *__map_size( vm_t *vm, Object *me, vframe_t *data ){
	return (Object *)gc_new_integer( ob_map_ucast(me)->items );
}

Object *__map_pop( vm_t *vm, Object *me, vframe_t *data ){
	return ob_cl_pop( me );
}

Object *__map_unmap( vm_t *vm, Object *me, vframe_t *data ){
	if( vm_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'unmap' requires 1 parameter (called with %d)", vm_argc() );
	}

	return ob_cl_remove( me, vm_argv(0) );
}

Object *__map_has( vm_t *vm, Object *me, vframe_t *data ){
	if( vm_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'has' requires 1 parameter (called with %d)", vm_argc() );
	}

	return (Object *)gc_new_boolean( map_find( me, vm_argv(0) ) == -1 ? false : true );
}

Object *__map_keys( vm_t *vm, Object *me, vframe_t *data ){
	Map *mme  = ob_map_ucast(me);
	Object    *keys = (Object *)gc_new_vector();
	int		   i, sz( mme->keys.size() );

	for( i = 0; i < sz; ++i ){
		ob_cl_push( keys, mme->keys[i] );
	}

	return keys;
}

Object *__map_values( vm_t *vm, Object *me, vframe_t *data ){
	Map *mme    = ob_map_ucast(me);
	Object    *values = (Object *)gc_new_vector();
	int		   i, sz( mme->values.size() );

	for( i = 0; i < sz; ++i ){
		ob_cl_push( values, mme->values[i] );
	}

	return values;
}

/** generic function pointers **/
Object *map_traverse( Object *me, int index ){
	Map *mme = (Map *)me;
	size_t items( mme->items );

	if( index < items ){
		return mme->keys.at(index);
	}
	else{
		index -= items;
		if( index < items ){
			return mme->values.at(index);
		}
	}
	return NULL;
}

Object *map_clone( Object *me ){
    MapIterator ki, vi;
    Map *mclone = gc_new_map(),
              *mme    = (Map *)me;
    Object    *kclone,
              *vclone;

    for( ki = mme->keys.begin(), vi = mme->values.begin(); ki != mme->keys.end() && vi != mme->values.end(); ki++, vi++ ){
        kclone = ob_clone( *ki );
        vclone = ob_clone( *vi );

        mclone->keys.push_back(kclone);
        mclone->values.push_back(vclone);
        mclone->items++;
    }

    return (Object *)mclone;
}

void map_free( Object *me ){
	Map *mme = (Map *)me;

    mme->keys.clear();
    mme->values.clear();
    mme->items = 0;
}

size_t map_get_size( Object *me ){
	return ob_map_ucast(me)->items;
}

int map_cmp( Object *me, Object *cmp ){
    if( !ob_is_map(cmp) ){
        return 1;
    }
    else {
        Map *mme  = (Map *)me,
                  *mcmp = (Map *)cmp;
        size_t     mme_ksize( mme->keys.size() ),
                   mcmp_ksize( mcmp->keys.size() ),
                   mme_vsize( mme->values.size() ),
                   mcmp_vsize( mcmp->values.size() );

        if( mme_ksize > mcmp_ksize || mme_vsize > mcmp_vsize ){
            return 1;
        }
        else if( mme_ksize < mcmp_ksize || mme_vsize < mcmp_vsize ){
            return -1;
        }
        /*
         * Same type and same size, let's check the elements.
         */
        else{
            size_t i;
            int    diff;

            for( i = 0; i < mme_ksize; ++i ){
                diff = ob_cmp( mme->keys[i], mcmp->keys[i] );
                if( diff != 0 ){
                    return diff;
                }
                diff = ob_cmp( mme->values[i], mcmp->values[i] );
                if( diff != 0 ){
                    return diff;
                }
            }
            return 0;
        }
    }
}

long map_ivalue( Object *me ){
    return static_cast<long>( ob_map_ucast(me)->items );
}

double map_fvalue( Object *me ){
    return static_cast<double>( ob_map_ucast(me)->items );
}

bool map_lvalue( Object *me ){
    return static_cast<bool>( ob_map_ucast(me)->items );
}

string map_svalue( Object *o ){
	return string( "<map>" );
}

void map_print( Object *me, int tabs ){
    MapIterator ki, vi;
    Map *mme = (Map *)me;
    Object    *kitem,
              *vitem;
    int        j;

    for( j = 0; j < tabs; ++j ){
        fprintf( stdout, "\t" );
    }
    fprintf( stdout, "map {\n" );
    for( ki = mme->keys.begin(), vi = mme->values.begin(); ki != mme->keys.end() && vi != mme->values.end(); ki++, vi++ ){
        kitem = *ki;
        vitem = *vi;
        ob_print( kitem, tabs + 1 );
        fprintf( stdout, " -> " );
        ob_print( vitem, tabs + 1 );
        fprintf( stdout, "\n" );
    }
    for( j = 0; j < tabs; ++j ) fprintf( stdout, "\t" );
    fprintf( stdout, "}\n" );
}

/** arithmetic operators **/
Object *map_assign( Object *me, Object *op ){
    map_free(me);

    Object *clone = ob_clone(op);

    return me = clone;
}

/** collection operators **/
Object *map_cl_pop( Object *me ){
    size_t last_idx = ob_map_ucast(me)->items - 1;

    if( last_idx < 0 ){
    	return vm_raise_exception( "could not pop an element from an empty map" );
    }

    Object *kitem = ((Map *)me)->keys[last_idx],
           *vitem = ((Map *)me)->values[last_idx];

    ((Map *)me)->keys.pop_back();
    ((Map *)me)->values.pop_back();

    ob_map_ucast(me)->items--;

    ob_free(kitem);

    return vitem;
}

Object *map_cl_remove( Object *me, Object *k ){
    int idx = map_find( me, k );
    if( idx != -1 ){
        Object *kitem = ((Map *)me)->keys[idx],
               *vitem = ((Map *)me)->values[idx];

		((Map *)me)->keys.erase( ((Map *)me)->keys.begin() + idx );
		((Map *)me)->values.erase( ((Map *)me)->values.begin() + idx );

		ob_map_ucast(me)->items--;

		ob_free(kitem);

        return vitem;
    }
    return me;
}

Object *map_cl_at( Object *me, Object *k ){
    int idx = map_find( me, k );
    if( idx != -1 ){
        return ((Map *)me)->values[idx];
    }
    else{
    	return vm_raise_exception( "no mapped values for label '%s'", ob_svalue(k).c_str() );
    }
    return me;
}

Object *map_cl_set_reference( Object *me, Object *k, Object *v ){
    int idx = map_find( me, k );
    if( idx != -1 ){
        Object *item = ((Map *)me)->values[idx];
        ob_free(item);

        ((Map *)me)->values[idx] = v;
    }
    else{
        ((Map *)me)->keys.push_back( k );
        ((Map *)me)->values.push_back( v );
        ob_map_ucast(me)->items++;
    }

    return me;
}

Object *map_cl_set( Object *me, Object *k, Object *v ){
    return map_cl_set_reference( me, ob_clone(k), ob_clone(v) );
}

Object *map_call_method( vm_t *vm, vframe_t *frame, Object *me, char *me_id, char *method_id, Node *argv ){
	ob_type_builtin_method_t *method = NULL;

	if( (method = ob_get_builtin_method( me, method_id )) == NULL ){
		hyb_error( H_ET_SYNTAX, "Map type does not have a '%s' method", method_id );
	}

	Object  *value,
			*result;
	vframe_t stack;
	size_t   i, argc = argv->children();

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );

	stack.owner = ob_typename(me) + string("::") + method_id;
	/*
	 * Evaluate each object and insert it into the stack
	 */
	for( i = 0; i < argc; ++i ){
		value = vm_exec( vm, frame, argv->child(i) );

		if( frame->state.is(Exception) || frame->state.is(Return) ){
			vm_pop_frame( vm );
			return frame->state.value;
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

IMPLEMENT_TYPE(Map) {
    /** type code **/
    otMap,
	/** type name **/
    "map",
	/** type basic size **/
    OB_COLLECTION_SIZE,
    /** type builtin methods **/
    {
		{ "size",   (ob_type_builtin_method_t *)__map_size },
		{ "pop",    (ob_type_builtin_method_t *)__map_pop },
		{ "unmap",  (ob_type_builtin_method_t *)__map_unmap },
		{ "has",    (ob_type_builtin_method_t *)__map_has },
		{ "keys",   (ob_type_builtin_method_t *)__map_keys },
		{ "values", (ob_type_builtin_method_t *)__map_values },
		OB_BUILIN_METHODS_END_MARKER
    },

	/** generic function pointers **/
    0, // type_name
    map_traverse, // traverse
	map_clone, // clone
	map_free, // free
	map_get_size, // get_size
	0, // serialize
	0, // deserialize
	0, // to_fd
	0, // from_fd
	map_cmp, // cmp
	map_ivalue, // ivalue
	map_fvalue, // fvalue
	map_lvalue, // lvalue
	map_svalue, // svalue
	map_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	0, // range
	0, // regexp

	/** arithmetic operators **/
	map_assign, // assign
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
	0, // cl_push
	0, // cl_push_reference
	map_cl_pop, // cl_pop
	map_cl_remove, // cl_remove
	map_cl_at, // cl_at
	map_cl_set, // cl_set
	map_cl_set_reference, // cl_set_reference

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
    map_call_method  // call_method
};

