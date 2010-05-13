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
#include "types.h"

#define REF_IS_NULL_PTR(r) if( ob_ref_ucast(r)->value == NULL ){ \
							   return (Object *)r; \
						   }

#define REF_IS_NULL_PTR_RET(r,v) if( ob_ref_ucast(r)->value == NULL ){ \
								   return v; \
								 }

/** generic function pointers **/
const char *ref_typename( Object *o ){
	return ("reference<" + string( ob_ref_ucast(o)->value ? ob_typename( ob_ref_ucast(o)->value ) : "NULL" ) + ">").c_str();
}

Object *ref_traverse( Object *me, int index ){
	return (index > 0 ? NULL : ((ReferenceObject *)me)->value);
}

Object *ref_clone( Object *me ){
	ReferenceObject *rme	= ob_ref_ucast(me),
					*rclone = gc_new_reference( rme->value );

    return (me = (Object *)(rclone));
}

size_t ref_get_size( Object *me ){
	return ob_ref_ucast(me)->value ? ob_get_size( ob_ref_ucast(me)->value ) : 0;
}

byte *ref_serialize( Object *me, size_t size ){
	return ob_serialize( ob_ref_ucast(me)->value, size );
}

Object *ref_deserialize( Object *me, byte *buffer, size_t size ){
	REF_IS_NULL_PTR(me);

	return ob_deserialize( ob_ref_ucast(me)->value, buffer, size );
}

Object *ref_to_fd( Object *me, int fd, size_t size ){
	REF_IS_NULL_PTR(me);

	return ob_to_fd( ob_ref_ucast(me)->value, fd, size );
}

Object *ref_from_fd( Object *me, int fd, size_t size ){
	REF_IS_NULL_PTR(me);

	return ob_from_fd( ob_ref_ucast(me)->value, fd, size );
}

int ref_cmp( Object *me, Object *cmp ){
	if( ob_is_reference(cmp) && ob_ref_ucast(me)->value == ob_ref_ucast(cmp)->value ){
		return 0;
	}
	return -1;
}

long ref_ivalue( Object *me ){
    return ob_ref_ucast(me)->value ? ob_ivalue( ob_ref_ucast(me)->value ) : 0;
}

double ref_fvalue( Object *me ){
    return ob_ref_ucast(me)->value ? ob_fvalue( ob_ref_ucast(me)->value ) : 0.0f;
}

bool ref_lvalue( Object *me ){
    return ob_ref_ucast(me)->value ? ob_lvalue( ob_ref_ucast(me)->value ) : false;
}

string ref_svalue( Object *me ){
	return ob_ref_ucast(me)->value ? ob_svalue( ob_ref_ucast(me)->value ) : string("<null>");
}

void ref_print( Object *me, int tabs ){
	if( ob_ref_ucast(me)->value ){
		ob_print( ob_ref_ucast(me)->value, tabs );
	}
	else{
		ob_print( (Object *)gc_new_string("<null>"), tabs );
	}
}

void ref_scanf( Object *me ){
    if( ob_ref_ucast(me)->value ){
    	ob_input( ob_ref_ucast(me)->value );
    }
}

Object * ref_to_string( Object *me ){
	return ob_ref_ucast(me)->value ? ob_to_string( ob_ref_ucast(me)->value ) : (Object *)gc_new_string("<null>");
}

Object * ref_to_int( Object *me ){
    return ob_ref_ucast(me)->value ? ob_to_int( ob_ref_ucast(me)->value ) : (Object *)gc_new_integer(0);
}

Object *ref_range( Object *me, Object *op ){
	return ob_ref_ucast(me)->value ? ob_range( ob_ref_ucast(me)->value, op ) : (Object *)gc_new_vector();
}

Object *ref_regexp( Object *me, Object *op ){
	return ob_ref_ucast(me)->value ? ob_apply_regexp( ob_ref_ucast(me)->value, op ) : (Object *)gc_new_vector();
}

/** arithmetic operators **/
Object *ref_assign( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

    return ob_assign( ob_ref_ucast(me)->value, op );
}

Object *ref_factorial( Object *me ){
	REF_IS_NULL_PTR(me);

	return ob_factorial( ob_ref_ucast(me)->value );
}

Object *ref_increment( Object *me ){
	REF_IS_NULL_PTR(me);

	return ob_increment( ob_ref_ucast(me)->value );
}

Object *ref_decrement( Object *me ){
	REF_IS_NULL_PTR(me);

	return ob_decrement( ob_ref_ucast(me)->value );
}

Object *ref_minus( Object *me ){
	REF_IS_NULL_PTR(me);

	return ob_uminus( ob_ref_ucast(me)->value );
}

Object *ref_add( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_add( ob_ref_ucast(me)->value, op );
}

Object *ref_sub( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_sub( ob_ref_ucast(me)->value, op );
}

Object *ref_mul( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_mul( ob_ref_ucast(me)->value, op );
}

Object *ref_div( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_div( ob_ref_ucast(me)->value, op );
}

Object *ref_mod( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_mod( ob_ref_ucast(me)->value, op );
}

Object *ref_inplace_add( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_inplace_add( ob_ref_ucast(me)->value, op );
}

Object *ref_inplace_sub( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_inplace_sub( ob_ref_ucast(me)->value, op );
}

Object *ref_inplace_mul( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_inplace_mul( ob_ref_ucast(me)->value, op );
}

Object *ref_inplace_div( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_inplace_div( ob_ref_ucast(me)->value, op );
}

Object *ref_inplace_mod( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_inplace_mod( ob_ref_ucast(me)->value, op );
}

/** bitwise operators **/
Object *ref_bw_and( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_and( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_or( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_or( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_not( Object *me ){
	return ob_bw_not( ob_ref_ucast(me)->value );
}

Object *ref_bw_xor( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_xor( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_lshift( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_lshift( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_rshift( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_rshift( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_inplace_and( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_inplace_and( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_inplace_or( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_inplace_or( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_inplace_xor( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_inplace_xor( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_inplace_lshift( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_inplace_lshift( ob_ref_ucast(me)->value, op );
}

Object *ref_bw_inplace_rshift( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_bw_inplace_rshift( ob_ref_ucast(me)->value, op );
}

/** logic operators **/
Object *ref_l_not( Object *me ){
	if( ob_ref_ucast(me)->value == NULL ){
		return (Object *)gc_new_boolean(true);
	}
	return ob_l_not( ob_ref_ucast(me)->value );
}

Object *ref_l_same( Object *me, Object *op ){
	if( ob_is_reference(op) ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value == ob_ref_ucast(op)->value );
	}
	return ob_l_same( ob_ref_ucast(me)->value, op );
}

Object *ref_l_diff( Object *me, Object *op ){
	if( ob_is_reference(op) ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value != ob_ref_ucast(op)->value );
	}
	return ob_l_diff( ob_ref_ucast(me)->value, op );
}

Object *ref_l_less( Object *me, Object *op ){
	if( ob_is_reference(op) && !ob_ref_ucast(op)->value ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value < ob_ref_ucast(op)->value );
	}
	return ob_l_less( ob_ref_ucast(me)->value, op );
}

Object *ref_l_greater( Object *me, Object *op ){
	if( ob_is_reference(op) && !ob_ref_ucast(op)->value ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value > ob_ref_ucast(op)->value );
	}
	return ob_l_greater( ob_ref_ucast(me)->value, op );
}

Object *ref_l_less_or_same( Object *me, Object *op ){
	if( ob_is_reference(op) && !ob_ref_ucast(op)->value ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value <= ob_ref_ucast(op)->value );
	}
	return ob_l_less_or_same( ob_ref_ucast(me)->value, op );
}

Object *ref_l_greater_or_same( Object *me, Object *op ){
	if( ob_is_reference(op) && !ob_ref_ucast(op)->value ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value >= ob_ref_ucast(op)->value );
	}
	return ob_l_greater_or_same( ob_ref_ucast(me)->value, op );
}

Object *ref_l_or( Object *me, Object *op ){
	if( ob_is_reference(op) && !ob_ref_ucast(op)->value ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value || ob_ref_ucast(op)->value );
	}
	return ob_l_or( ob_ref_ucast(me)->value, op );
}

Object *ref_l_and( Object *me, Object *op ){
	if( ob_is_reference(op) ){
		return (Object *)gc_new_boolean( ob_ref_ucast(me)->value && ob_ref_ucast(op)->value );
	}
	return ob_l_and( ob_ref_ucast(me)->value, op );
}

/* collection operators */
Object *ref_cl_push( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_cl_push( ob_ref_ucast(me)->value, op );
}

Object *ref_cl_push_reference( Object *me, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_cl_push_reference( ob_ref_ucast(me)->value, op );
}

Object *ref_cl_pop( Object *me ){
	REF_IS_NULL_PTR(me);

	return ob_cl_pop( ob_ref_ucast(me)->value );
}

Object *ref_cl_remove( Object *me, Object *i ){
	REF_IS_NULL_PTR(me);

	return ob_cl_remove( ob_ref_ucast(me)->value, i );
}

Object *ref_cl_at( Object *me, Object *index ){
	REF_IS_NULL_PTR(me);

	return ob_cl_at( ob_ref_ucast(me)->value, index );
}

Object *ref_cl_set( Object *me, Object *index, Object *op ){
	REF_IS_NULL_PTR(me);

	return ob_cl_set( ob_ref_ucast(me)->value, index, op );
}

Object *ref_cl_set_reference( Object *me, Object *i, Object *v ){
	REF_IS_NULL_PTR(me);

	return ob_cl_set_reference( ob_ref_ucast(me)->value, i, v );
}

/** class operators **/
void ref_define_attribute( Object *me, char *name, access_t access, bool is_static /*= false*/ ){
	REF_IS_NULL_PTR_RET(me,/**/);

	ob_define_attribute( ob_ref_ucast(me)->value, name, access, is_static );
}

access_t ref_attribute_access( Object *me, char *name ){
	REF_IS_NULL_PTR_RET(me,asPublic);

	return ob_attribute_access( ob_ref_ucast(me)->value, name );
}

bool ref_attribute_is_static( Object *me, char *name ){
	REF_IS_NULL_PTR_RET(me,false);

	return ob_attribute_is_static( ob_ref_ucast(me)->value, name );
}

void ref_set_attribute_access( Object *me, char *name, access_t access ){
	REF_IS_NULL_PTR_RET(me,/**/);

	ob_set_attribute_access( ob_ref_ucast(me)->value, name, access );
}

void ref_add_attribute( Object *me, char *name ){
	REF_IS_NULL_PTR_RET(me,/**/);

	ob_add_attribute( ob_ref_ucast(me)->value, name );
}

Object *ref_get_attribute( Object *me, char *name, bool with_descriptor /* = true */ ){
	REF_IS_NULL_PTR(me);

    return ob_get_attribute( ob_ref_ucast(me)->value, name, with_descriptor );
}

void ref_set_attribute_reference( Object *me, char *name, Object *value ){
	REF_IS_NULL_PTR_RET(me,/**/);

    ob_set_attribute_reference( ob_ref_ucast(me)->value, name, value );
}

void ref_set_attribute( Object *me, char *name, Object *value ){
	REF_IS_NULL_PTR_RET(me,/**/);

    return ob_set_attribute_reference( ob_ref_ucast(me)->value, name, ob_clone(value) );
}

void ref_define_method( Object *me, char *name, Node *code ){
	REF_IS_NULL_PTR_RET(me,/**/);

	ob_define_method( ob_ref_ucast(me)->value, name, code );
}

Node *ref_get_method( Object *me, char *name, int argc ){
	REF_IS_NULL_PTR_RET(me,NULL);

	return ob_get_method( ob_ref_ucast(me)->value, name, argc );
}

Object *ref_call_method( engine_t *engine, vframe_t *frame, Object *me, char *me_id, char *method_id, Node *argv ){
	REF_IS_NULL_PTR_RET(me,NULL);

	return ob_call_method( engine, frame, me, me_id, method_id, argv );
}

IMPLEMENT_TYPE(Reference) {
    /** type code **/
    otReference,
	/** type name **/
    "reference",
	/** type basic size **/
    sizeof( Object * ),
    /** type builtin methods **/
    { OB_BUILIN_METHODS_END_MARKER },

	/** generic function pointers **/
    ref_typename, // type_name
    ref_traverse, // traverse
	ref_clone, // clone
	0, // free
	ref_get_size, // get_size
	ref_serialize, // serialize
	ref_deserialize, // deserialize
	ref_to_fd, // to_fd
	ref_from_fd, // from_fd
	ref_cmp, // cmp
	ref_ivalue, // ivalue
	ref_fvalue, // fvalue
	ref_lvalue, // lvalue
	ref_svalue, // svalue
	ref_print, // print
	ref_scanf, // scanf
	ref_to_string, // to_string
	ref_to_int, // to_int
	ref_range, // range
	ref_regexp, // regexp

	/** arithmetic operators **/
	ref_assign, // assign
    ref_factorial, // factorial
    ref_increment, // increment
    ref_decrement, // decrement
    ref_minus, // minus
    ref_add, // add
    ref_sub, // sub
    ref_mul, // mul
    ref_div, // div
    ref_mod, // mod
    ref_inplace_add, // inplace_add
    ref_inplace_sub, // inplace_sub
    ref_inplace_mul, // inplace_mul
    ref_inplace_div, // inplace_div
    ref_inplace_mod, // inplace_mod

	/** bitwise operators **/
	ref_bw_and, // bw_and
    ref_bw_or, // bw_or
    ref_bw_not, // bw_not
    ref_bw_xor, // bw_xor
    ref_bw_lshift, // bw_lshift
    ref_bw_rshift, // bw_rshift
    ref_bw_inplace_and, // bw_inplace_and
    ref_bw_inplace_or, // bw_inplace_or
    ref_bw_inplace_xor, // bw_inplace_xor
    ref_bw_inplace_lshift, // bw_inplace_lshift
    ref_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    ref_l_not, // l_not
    ref_l_same, // l_same
    ref_l_diff, // l_diff
    ref_l_less, // l_less
    ref_l_greater, // l_greater
    ref_l_less_or_same, // l_less_or_same
    ref_l_greater_or_same, // l_greater_or_same
    ref_l_or, // l_or
    ref_l_and, // l_and

	/** collection operators **/
	ref_cl_push, // cl_push
	ref_cl_push_reference, // cl_push_reference
	ref_cl_pop, // cl_pop
	ref_cl_remove, // cl_remove
	ref_cl_at, // cl_at
	ref_cl_set, // cl_set
	ref_cl_set_reference, // cl_set_reference

	/** structure operators **/
	ref_define_attribute, // define_attribute
	ref_attribute_access, // attribute_access
	ref_attribute_is_static, // attribute_is_static
	ref_set_attribute_access, // set_attribute_access
    ref_add_attribute, // add_attribute
    ref_get_attribute, // get_attribute
    ref_set_attribute, // set_attribute
    ref_set_attribute_reference,  // set_attribute_reference
    ref_define_method, // define_method
    ref_get_method,  // get_method
    ref_call_method // call_method
};

