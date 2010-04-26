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
#include <math.h>

/** generic function pointers **/
const char *int_typename( Object *o ){
	return o->type->name;
}

const char *alias_typename( Object *o ){
	return o->type->name;
}

const char *extern_typename( Object *o ){
	return o->type->name;
}

void int_set_references( Object *me, int ref ){
    me->ref += ref;
}

Object *int_clone( Object *me ){
    return (Object *)gc_new_integer( (ob_int_ucast(me))->value );
}

Object *alias_clone( Object *me ){
    return (Object *)gc_new_alias( (ob_int_ucast(me))->value );
}

Object *extern_clone( Object *me ){
    return (Object *)gc_new_extern( (ob_int_ucast(me))->value );
}

size_t int_get_size( Object *me ){
	return sizeof(long);
}

byte *int_serialize( Object *o, size_t size ){
	size_t s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o) );
	byte  *buffer = new byte[s];

	memcpy( buffer, &((ob_int_ucast(o))->value), s );

	return buffer;
}

Object *int_deserialize( Object *o, byte *buffer, size_t size ){
	if( size ){
		o = ob_dcast( gc_new_integer(0) );
		memcpy( &((ob_int_ucast(o))->value), buffer, size );
	}
	return o;
}

Object *int_to_fd( Object *o, int fd, size_t size ){
	size_t s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o));
	int    written;

	written = write( fd, &((ob_int_ucast(o))->value), s );

	return ob_dcast( gc_new_integer(written) );
}

Object *int_from_fd( Object *o, int fd, size_t size ){
	int rd = 0;
	if( size ){
		rd = read( fd, &((ob_int_ucast(o))->value), size );
	}
	else{
		return ob_from_fd( o, fd, sizeof(long) );
	}
	return ob_dcast( gc_new_integer(rd) );
}

int int_cmp( Object *me, Object *cmp ){
    long ivalue = ob_ivalue(cmp),
         mvalue = (ob_int_ucast(me))->value;

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

long int_ivalue( Object *me ){
    return (ob_int_ucast(me))->value;
}

double int_fvalue( Object *me ){
    return (double)(ob_int_ucast(me))->value;
}

bool int_lvalue( Object *me ){
    return (bool)(ob_int_ucast(me))->value;
}

string int_svalue( Object *me ){
    long ivalue = (ob_int_ucast(me))->value;
    char svalue[0xFF] = {0};

    sprintf( svalue, "%ld", ivalue );

    return string(svalue);
}

void int_print( Object *me, int tabs ){
    for( int i = 0; i < tabs; ++i ){
        printf( "\t" );
    }
    printf( "%ld", (ob_int_ucast(me))->value );
}

void int_scanf( Object *me ){
    scanf( "%ld", &(ob_int_ucast(me))->value );
}

Object * int_to_string( Object *me ){
    DECLARE_TYPE(String);

    return String_Type.from_int(me);
}

Object * int_to_int( Object *me ){
    return me;
}

Object * int_from_int( Object *i ){
    return i;
}

Object * int_from_float( Object *f ){
    return (Object *)gc_new_integer( ob_float_ucast(f)->value );
}

Object *int_range( Object *a, Object *b ){
	int     i, start, end;
	Object *range = (Object *)gc_new_vector();

	if( ob_cmp( a, b ) == -1 ){
		start = (ob_int_ucast(a))->value;
		end   = (ob_int_ucast(b))->value;
	}
	else{
		start = (ob_int_ucast(b))->value;
		end   = (ob_int_ucast(a))->value;
	}

	for( i = start; i <= end; ++i ){
		ob_cl_push_reference( range, (Object *)gc_new_integer(i) );
	}

	return range;
}

/** arithmetic operators **/
Object *int_assign( Object *me, Object *op ){
    if( ob_is_int(op) ){
        (ob_int_ucast(me))->value = (ob_int_ucast(op))->value;
        return me;
    }
    else {
        Object *clone = ob_clone(op);

        ob_set_references( clone, +1 );

        return (me = clone);
    }
}

Object *int_factorial( Object *me ){
    long ivalue = (ob_int_ucast(me))->value,
         ifact  = 1,
         i;

    for( i = 1; i <= ivalue; ++i ){
        ifact *= i;
    }

    return (Object *)gc_new_integer(ifact);
}

Object *int_increment( Object *me ){
    (ob_int_ucast(me))->value++;

    return me;
}

Object *int_decrement( Object *me ){
    (ob_int_ucast(me))->value--;

    return me;
}

Object *int_minus( Object *me ){
    return (Object *)gc_new_integer( -(ob_int_ucast(me))->value );
}

Object *int_add( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value + ivalue );
}

Object *int_sub( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value - ivalue );
}

Object *int_mul( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value * ivalue );
}

Object *int_div( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value / ivalue );
}

Object *int_mod( Object *me, Object *op ){
    long a = (ob_int_ucast(me))->value,
         b = ob_ivalue(op),
         mod;

	/* b is 0 or 1 */
    if( b == 0 || b == 1 ){
        mod = 0;
    }
    /* b is a power of 2 */
    else if( (b & (b - 1)) == 0 ){
        mod = a & (b - 1);
    }
    else{
        mod = a % b;
    }

    return (Object *)gc_new_integer(mod);
}

Object *int_inplace_add( Object *me, Object *op ){
    (ob_int_ucast(me))->value += ob_ivalue(op);

    return me;
}

Object *int_inplace_sub( Object *me, Object *op ){
    (ob_int_ucast(me))->value -= ob_ivalue(op);

    return me;
}

Object *int_inplace_mul( Object *me, Object *op ){
    (ob_int_ucast(me))->value *= ob_ivalue(op);

    return me;
}

Object *int_inplace_div( Object *me, Object *op ){
    (ob_int_ucast(me))->value /= ob_ivalue(op);

    return me;
}

Object *int_inplace_mod( Object *me, Object *op ){
    long a = (ob_int_ucast(me))->value,
         b = ob_ivalue(op);

	/* b is 0 or 1 */
    if( b == 0 || b == 1 ){
        (ob_int_ucast(me))->value = 0;
    }
    /* b is a power of 2 */
    else if( (b & (b - 1)) == 0 ){
        (ob_int_ucast(me))->value = a & (b - 1);
    }
    else{
        (ob_int_ucast(me))->value = a % b;
    }

    return me;
}

/** bitwise operators **/
Object *int_bw_and( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value & ivalue );
}

Object *int_bw_or( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value | ivalue );
}

Object *int_bw_not( Object *me ){
    return (Object *)gc_new_integer( ~(ob_int_ucast(me))->value );
}

Object *int_bw_xor( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value ^ ivalue );
}

Object *int_bw_lshift( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value << ivalue );
}

Object *int_bw_rshift( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value >> ivalue );
}

Object *int_bw_inplace_and( Object *me, Object *op ){
    (ob_int_ucast(me))->value &= ob_ivalue(op);

    return me;
}

Object *int_bw_inplace_or( Object *me, Object *op ){
    (ob_int_ucast(me))->value &= ob_ivalue(op);

    return me;
}

Object *int_bw_inplace_xor( Object *me, Object *op ){
    (ob_int_ucast(me))->value ^= ob_ivalue(op);

    return me;
}

Object *int_bw_inplace_lshift( Object *me, Object *op ){
    (ob_int_ucast(me))->value <<= ob_ivalue(op);

    return me;
}

Object *int_bw_inplace_rshift( Object *me, Object *op ){
    (ob_int_ucast(me))->value >>= ob_ivalue(op);

    return me;
}

/** logic operators **/
Object *int_l_not( Object *me ){
    return (Object *)gc_new_integer( !(ob_int_ucast(me))->value );
}

Object *int_l_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value == ivalue );
}

Object *int_l_diff( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value != ivalue );
}

Object *int_l_less( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value < ivalue );
}

Object *int_l_greater( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value > ivalue );
}

Object *int_l_less_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value <= ivalue );
}

Object *int_l_greater_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value >= ivalue );
}

Object *int_l_or( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value || ivalue );
}

Object *int_l_and( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_integer( (ob_int_ucast(me))->value && ivalue );
}

/** collection operators **/
Object *int_cl_concat( Object *me, Object *op ){
    string mvalue = ob_svalue(me),
           svalue = ob_svalue(op);

    return (Object *)gc_new_string( (mvalue + svalue).c_str() );
}

Object *int_cl_at( Object *me, Object *op ){
    long ivalue = ob_ivalue(op),
         x      = (ob_int_ucast(me))->value;

    while (ivalue --) {
        x /= 10;
    }

    return (Object *)gc_new_integer(x % 10);
}

IMPLEMENT_TYPE(Integer) {
    /** type code **/
    otInteger,
	/** type name **/
    "integer",
	/** type basic size **/
    sizeof(long),

	/** generic function pointers **/
    int_typename, // type_name
	int_set_references, // set_references
	int_clone, // clone
	0, // free
	int_get_size, // get_size
	int_serialize, // serialize
	int_deserialize, // deserialize
	int_to_fd, // to_fd
	int_from_fd, // from_fd
	int_cmp, // cmp
	int_ivalue, // ivalue
	int_fvalue, // fvalue
	int_lvalue, // lvalue
	int_svalue, // svalue
	int_print, // print
	int_scanf, // scanf
	int_to_string, // to_string
	int_to_int, // to_int
	int_from_int, // from_int
	int_from_float, // from_float
	int_range, // range
	0, // regexp

	/** arithmetic operators **/
	int_assign, // assign
    int_factorial, // factorial
    int_increment, // increment
    int_decrement, // decrement
    int_minus, // minus
    int_add, // add
    int_sub, // sub
    int_mul, // mul
    int_div, // div
    int_mod, // mod
    int_inplace_add, // inplace_add
    int_inplace_sub, // inplace_sub
    int_inplace_mul, // inplace_mul
    int_inplace_div, // inplace_div
    int_inplace_mod, // inplace_mod

	/** bitwise operators **/
	int_bw_and, // bw_and
    int_bw_or, // bw_or
    int_bw_not, // bw_not
    int_bw_xor, // bw_xor
    int_bw_lshift, // bw_lshift
    int_bw_rshift, // bw_rshift
    int_bw_inplace_and, // bw_inplace_and
    int_bw_inplace_or, // bw_inplace_or
    int_bw_inplace_xor, // bw_inplace_xor
    int_bw_inplace_lshift, // bw_inplace_lshift
    int_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    int_l_not, // l_not
    int_l_same, // l_same
    int_l_diff, // l_diff
    int_l_less, // l_less
    int_l_greater, // l_greater
    int_l_less_or_same, // l_less_or_same
    int_l_greater_or_same, // l_greater_or_same
    int_l_or, // l_or
    int_l_and, // l_and

	/** collection operators **/
	int_cl_concat, // cl_concat
	0, // cl_inplace_concat
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	int_cl_at, // cl_at

	/** structure operators **/
	0, // define_attribute
	0, // attribute_access
	0, // set_attribute_access
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference
    0, // define_method
    0  // get_method
};

IMPLEMENT_TYPE(Alias) {
    /** type code **/
    otAlias,
	/** type name **/
    "alias",
	/** type basic size **/
    sizeof(long),

	/** generic function pointers **/
    alias_typename, // type_name
	int_set_references, // set_references
	alias_clone, // clone
	0, // free
	int_get_size, // get_size
	int_serialize, // serialize
	int_deserialize, // deserialize
	0, // to_fd
	0, // from_fd
	int_cmp, // cmp
	int_ivalue, // ivalue
	int_fvalue, // fvalue
	int_lvalue, // lvalue
	int_svalue, // svalue
	int_print, // print
	int_scanf, // scanf
	int_to_string, // to_string
	int_to_int, // to_int
	int_from_int, // from_int
	int_from_float, // from_float
	int_range, // range
	0, // regexp

	/** arithmetic operators **/
	int_assign, // assign
    int_factorial, // factorial
    int_increment, // increment
    int_decrement, // decrement
    int_minus, // minus
    int_add, // add
    int_sub, // sub
    int_mul, // mul
    int_div, // div
    int_mod, // mod
    int_inplace_add, // inplace_add
    int_inplace_sub, // inplace_sub
    int_inplace_mul, // inplace_mul
    int_inplace_div, // inplace_div
    int_inplace_mod, // inplace_mod

	/** bitwise operators **/
	int_bw_and, // bw_and
    int_bw_or, // bw_or
    int_bw_not, // bw_not
    int_bw_xor, // bw_xor
    int_bw_lshift, // bw_lshift
    int_bw_rshift, // bw_rshift
    int_bw_inplace_and, // bw_inplace_and
    int_bw_inplace_or, // bw_inplace_or
    int_bw_inplace_xor, // bw_inplace_xor
    int_bw_inplace_lshift, // bw_inplace_lshift
    int_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    int_l_not, // l_not
    int_l_same, // l_same
    int_l_diff, // l_diff
    int_l_less, // l_less
    int_l_greater, // l_greater
    int_l_less_or_same, // l_less_or_same
    int_l_greater_or_same, // l_greater_or_same
    int_l_or, // l_or
    int_l_and, // l_and

	/** collection operators **/
	int_cl_concat, // cl_concat
	0, // cl_inplace_concat
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	int_cl_at, // cl_at

    /** structure operators **/
	0, // define_attribute
	0, // attribute_access
	0, // set_attribute_access
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference;
    0, // define_method
    0  // get_method
};

IMPLEMENT_TYPE(Extern) {
    /** type code **/
    otExtern,
	/** type name **/
    "extern",
	/** type basic size **/
    sizeof(long),

	/** generic function pointers **/
    extern_typename, // type_name
	int_set_references, // set_references
	extern_clone, // clone
	0, // free
	int_get_size, // get_size
	int_serialize, // serialize
	int_deserialize, // deserialize
	0, // to_fd
	0, // from_fd
	int_cmp, // cmp
	int_ivalue, // ivalue
	int_fvalue, // fvalue
	int_lvalue, // lvalue
	int_svalue, // svalue
	int_print, // print
	int_scanf, // scanf
	int_to_string, // to_string
	int_to_int, // to_int
	int_from_int, // from_int
	int_from_float, // from_float
	int_range, // range
	0, // regexp

	/** arithmetic operators **/
	int_assign, // assign
    int_factorial, // factorial
    int_increment, // increment
    int_decrement, // decrement
    int_minus, // minus
    int_add, // add
    int_sub, // sub
    int_mul, // mul
    int_div, // div
    int_mod, // mod
    int_inplace_add, // inplace_add
    int_inplace_sub, // inplace_sub
    int_inplace_mul, // inplace_mul
    int_inplace_div, // inplace_div
    int_inplace_mod, // inplace_mod

	/** bitwise operators **/
	int_bw_and, // bw_and
    int_bw_or, // bw_or
    int_bw_not, // bw_not
    int_bw_xor, // bw_xor
    int_bw_lshift, // bw_lshift
    int_bw_rshift, // bw_rshift
    int_bw_inplace_and, // bw_inplace_and
    int_bw_inplace_or, // bw_inplace_or
    int_bw_inplace_xor, // bw_inplace_xor
    int_bw_inplace_lshift, // bw_inplace_lshift
    int_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    int_l_not, // l_not
    int_l_same, // l_same
    int_l_diff, // l_diff
    int_l_less, // l_less
    int_l_greater, // l_greater
    int_l_less_or_same, // l_less_or_same
    int_l_greater_or_same, // l_greater_or_same
    int_l_or, // l_or
    int_l_and, // l_and

	/** collection operators **/
	int_cl_concat, // cl_concat
	0, // cl_inplace_concat
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	int_cl_at, // cl_at
	0, // cl_set
	0, // cl_set_reference

    /** structure operators **/
	0, // define_attribute
	0, // attribute_access
	0, // set_attribute_access
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference
    0, // define_method
    0  // get_method
};
