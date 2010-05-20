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

/** generic function pointers **/
const char *bool_typename( Object *o ){
	return o->type->name;
}

Object *bool_traverse( Object *me, int index ){
	return NULL;
}

Object *bool_clone( Object *me ){
    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value );
}

size_t bool_get_size( Object *me ){
	return sizeof(bool);
}

byte *bool_serialize( Object *o, size_t size ){
	byte *buffer = new byte;

	*buffer = (byte)ob_bool_ucast(o)->value;

	return buffer;
}

Object *bool_deserialize( Object *o, byte *buffer, size_t size ){
	if( size ){
		o = ob_dcast( gc_new_boolean(*buffer) );
	}

	return o;
}

Object *bool_to_fd( Object *o, int fd, size_t size ){
	size_t s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o));
	int    written;

	written = write( fd, &((ob_bool_ucast(o))->value), s );

	return ob_dcast( gc_new_integer(written) );
}

Object *bool_from_fd( Object *o, int fd, size_t size ){
	int rd = 0;
	if( size ){
		rd = read( fd, &((ob_bool_ucast(o))->value), size );
	}
	else{
		return ob_from_fd( o, fd, sizeof(bool) );
	}
	return ob_dcast( gc_new_integer(rd) );
}

int bool_cmp( Object *me, Object *cmp ){
    long ivalue = ob_ivalue(cmp),
         mvalue = ob_bool_ucast(me)->value;

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

long bool_ivalue( Object *me ){
    return (long)ob_bool_ucast(me)->value;
}

double bool_fvalue( Object *me ){
    return (double)ob_bool_ucast(me)->value;
}

bool bool_lvalue( Object *me ){
    return ob_bool_ucast(me)->value;
}

string bool_svalue( Object *me ){
    char svalue[0xFF] = {0};

    sprintf( svalue, "%s", ob_bool_ucast(me)->value ? "true" : "false");

    return string(svalue);
}

void bool_print( Object *me, int tabs ){
    for( int i = 0; i < tabs; ++i ){
        fprintf( stdout, "\t" );
    }
    fprintf( stdout, "%s",  ob_bool_ucast(me)->value ? "true" : "false" );
}

Object * bool_to_string( Object *me ){
    return (Object *)gc_new_string( ob_svalue(me).c_str() );
}

Object * bool_to_int( Object *me ){
    return (Object *)gc_new_integer( ob_bool_ucast(me)->value );
}

/** arithmetic operators **/
Object *bool_assign( Object *me, Object *op ){
    if( ob_is_boolean(op) ){
        ob_bool_ucast(me)->value = ob_bool_ucast(op)->value;
    }
    else {
        Object *clone = ob_clone(op);

        me = clone;
    }

    return me;
}

/** logic operators **/
Object *bool_l_not( Object *me ){
    return (Object *)gc_new_boolean( !ob_bool_ucast(me)->value );
}

Object *bool_l_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value == ivalue );
}

Object *bool_l_diff( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value != ivalue );
}

Object *bool_l_less( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value < ivalue );
}

Object *bool_l_greater( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value > ivalue );
}

Object *bool_l_less_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value <= ivalue );
}

Object *bool_l_greater_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value >= ivalue );
}

Object *bool_l_or( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value || ivalue );
}

Object *bool_l_and( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_boolean( ob_bool_ucast(me)->value && ivalue );
}

IMPLEMENT_TYPE(Boolean) {
    /** type code **/
    otBoolean,
	/** type name **/
    "boolean",
	/** type basic size **/
    sizeof(bool),
    /** type builtin methods **/
    { OB_BUILIN_METHODS_END_MARKER },

	/** generic function pointers **/
    bool_typename, // type_name
    bool_traverse, // traverse
	bool_clone, // clone
	0, // free
	bool_get_size, // get_size
	bool_serialize, // serialize
	bool_deserialize, // deserialize
	bool_to_fd, // to_fd
	bool_from_fd, // from_fd
	bool_cmp, // cmp
	bool_ivalue, // ivalue
	bool_fvalue, // fvalue
	bool_lvalue, // lvalue
	bool_svalue, // svalue
	bool_print, // print
	0, // scanf
	bool_to_string, // to_string
	bool_to_int, // to_int
	0, // range
	0, // regexp

	/** arithmetic operators **/
	bool_assign, // assign
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
    bool_l_not, // l_not
    bool_l_same, // l_same
    bool_l_diff, // l_diff
    bool_l_less, // l_less
    bool_l_greater, // l_greater
    bool_l_less_or_same, // l_less_or_same
    bool_l_greater_or_same, // l_greater_or_same
    bool_l_or, // l_or
    bool_l_and, // l_and

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

