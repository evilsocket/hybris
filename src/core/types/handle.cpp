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

/** generic function pointers **/
Object *handle_traverse( Object *me, int index ){
	return (index > 0 ? NULL : (Object *)((Handle *)me)->value);
}

Object *handle_clone( Object *me ){
    return (Object *)gc_new_handle( ob_handle_ucast(me)->value );
}

int handle_cmp( Object *me, Object *cmp ){
    long ivalue = ob_ivalue(cmp),
         mvalue = ob_ivalue(me);

    if( mvalue == ivalue ){
        return 0;
    }
    else if( mvalue > ivalue ){
        return 1;
    }
    else{
        return -1;
    }
}

long handle_ivalue( Object *me ){
    return (long)H_ADDRESS_OF(ob_handle_ucast(me)->value);
}

double handle_fvalue( Object *me ){
    return (double)H_ADDRESS_OF(ob_handle_ucast(me)->value);
}

bool handle_lvalue( Object *me ){
    return (bool)H_ADDRESS_OF(ob_handle_ucast(me)->value);
}

string handle_svalue( Object *me ){
    char svalue[0xFF] = {0};

    sprintf( svalue, "%p", H_ADDRESS_OF(ob_handle_ucast(me)->value) );

    return string(svalue);
}

void handle_print( Object *me, int tabs ){
    for( int i = 0; i < tabs; ++i ){
        fprintf( stdout, "\t" );
    }
    fprintf( stdout, "%p", H_ADDRESS_OF(ob_handle_ucast(me)->value) );
}

Object * handle_to_string( Object *me ){
    return (Object *)gc_new_string( ob_svalue(me).c_str() );
}

Object * handle_to_int( Object *me ){
    return (Object *)gc_new_integer( H_ADDRESS_OF(ob_handle_ucast(me)->value) );
}

Object *handle_assign( Object *me, Object *op ){
	((Handle *)me)->value = NULL;

    Object *clone = ob_clone(op);

    return (me = clone);
}

/** logic operators **/
Object *handle_l_not( Object *me ){
    return (Object *)gc_new_integer( !ob_lvalue(me) );
}

Object *handle_l_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) == ivalue );
}

Object *handle_l_diff( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) != ivalue );
}

Object *handle_l_less( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) < ivalue );
}

Object *handle_l_greater( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) > ivalue );
}

Object *handle_l_less_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) <= ivalue );
}

Object *handle_l_greater_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) >= ivalue );
}

Object *handle_l_or( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) || ivalue );
}

Object *handle_l_and( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( ob_lvalue(me) && ivalue );
}

IMPLEMENT_TYPE(Handle) {
    /** type code **/
    otHandle,
	/** type name **/
    "handle",
	/** type basic size **/
    sizeof(void *),
    /** type builtin methods **/
    NO_BUILTIN_METHODS,
	/** generic function pointers **/
    0, // type_name
    handle_traverse, // traverse
	handle_clone, // clone
	0, // free
	0, // get_size
	0, // serialize
	0, // deserialize
	0, // to_fd
	0, // from_fd
	handle_cmp, // cmp
	handle_ivalue, // ivalue
	handle_fvalue, // fvalue
	handle_lvalue, // lvalue
	handle_svalue, // svalue
	handle_print, // print
	0, // scanf
	handle_to_string, // to_string
	handle_to_int, // to_int
	0, // range
	0, // regexp

	/** arithmetic operators **/
	handle_assign, // assign
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
    handle_l_not, // l_not
    handle_l_same, // l_same
    handle_l_diff, // l_diff
    handle_l_less, // l_less
    handle_l_greater, // l_greater
    handle_l_less_or_same, // l_less_or_same
    handle_l_greater_or_same, // l_greater_or_same
    handle_l_or, // l_or
    handle_l_and, // l_and

	/** collection operators **/
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	0, // cl_at
	0, // cl_set
	0, // cl_set_reference

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
    0  // call_method
};

