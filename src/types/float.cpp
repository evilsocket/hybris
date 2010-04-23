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
 * adouble with Hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "types.h"
#include <math.h>

/** generic function pointers **/
const char *float_typename( Object *o ){
	return o->type->name;
}

void float_set_references( Object *me, int ref ){
    me->ref += ref;
}

Object *float_clone( Object *me ){
    return (Object *)gc_new_float( ob_float_ucast(me)->value );
}

size_t float_get_size( Object *me ){
	return sizeof(double);
}

byte *float_serialize( Object *o, size_t size ){
	size_t i, s   = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o) );
	byte  *buffer = new byte[s];

	memcpy( buffer, &(ob_float_ucast(o)->value), s );

	return buffer;
}

Object *float_deserialize( Object *o, byte *buffer, size_t size ){
	if( size ){
		o = ob_dcast( gc_new_float(0) );
		memcpy( &(ob_float_ucast(o)->value), buffer, size );
	}
	return o;
}

Object *float_to_fd( Object *o, int fd, size_t size ){
	size_t s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o));
	int    written;

	written = write( fd, &((ob_float_ucast(o))->value), s );

	return ob_dcast( gc_new_integer(written) );
}

Object *float_from_fd( Object *o, int fd, size_t size ){
	int rd = 0;
	if( size ){
		rd = read( fd, &((ob_float_ucast(o))->value), size );
	}
	else{
		return ob_from_fd( o, fd, sizeof(double) );
	}
	return ob_dcast( gc_new_integer(rd) );
}

int float_cmp( Object *me, Object *cmp ){
    double fvalue = ob_fvalue(cmp),
           mvalue = ob_float_ucast(me)->value;

    if( mvalue == fvalue ){
        return 0;
    }
    else if( mvalue > fvalue ){
        return 1;
    }
    else{
        return -1;
    }
}

long float_ivalue( Object *me ){
    return (long)ob_float_ucast(me)->value;
}

double float_fvalue( Object *me ){
    return ob_float_ucast(me)->value;
}

bool float_lvalue( Object *me ){
    return (bool)ob_float_ucast(me)->value;
}

string float_svalue( Object *me ){
    double fvalue = ob_float_ucast(me)->value;
    char svalue[0xFF] = {0};

    sprintf( svalue, "%lf", fvalue );

    return string(svalue);
}

void float_print( Object *me, int tabs ){
    for( int i = 0; i < tabs; ++i ){
        printf( "\t" );
    }
    printf( "%lf", ob_float_ucast(me)->value );
}

void float_scanf( Object *me ){
    scanf( "%lf", &ob_float_ucast(me)->value );
}

Object * float_to_string( Object *me ){
    DECLARE_TYPE(String);

    return String_Type.from_float(me);
}

Object * float_to_int( Object *me ){
    return (Object *)gc_new_integer( ob_float_ucast(me)->value );
}

Object * float_from_int( Object *me ){
    return (Object *)gc_new_float( (ob_int_ucast(me))->value );
}

Object * float_from_float( Object *me ){
    return me;
}

/** arithmetic operators **/
Object *float_assign( Object *me, Object *op ){
    if( ob_is_float(op) ){
        ob_float_ucast(me)->value = ob_float_ucast(op)->value;
    }
    else {
        Object *clone = ob_clone(op);

        ob_set_references( clone, +1 );

        me = ob_clone(op);
    }

    return me;
}

Object *float_factorial( Object *me ){
    double fvalue = ob_float_ucast(me)->value;
    int    ifact  = 1,
           i;

    for( i = 1; i <= (int)fvalue; ++i ){
        ifact *= i;
    }

    return (Object *)gc_new_float(ifact);
}

Object *float_increment( Object *me ){
    ob_float_ucast(me)->value++;

    return me;
}

Object *float_decrement( Object *me ){
    ob_float_ucast(me)->value--;

    return me;
}

Object *float_minus( Object *me ){
    return (Object *)gc_new_float( -ob_float_ucast(me)->value );
}

Object *float_add( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value + fvalue );
}

Object *float_sub( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value - fvalue );
}

Object *float_mul( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value * fvalue );
}

Object *float_div( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value / fvalue );
}

Object *float_mod( Object *me, Object *op ){
    double a = ob_float_ucast(me)->value,
           b = ob_fvalue(op);

    return (Object *)gc_new_float( fmod( a, b ) );
}

Object *float_inplace_add( Object *me, Object *op ){
    ob_float_ucast(me)->value += ob_fvalue(op);

    return me;
}

Object *float_inplace_sub( Object *me, Object *op ){
    ob_float_ucast(me)->value -= ob_fvalue(op);

    return me;
}

Object *float_inplace_mul( Object *me, Object *op ){
    ob_float_ucast(me)->value *= ob_fvalue(op);

    return me;
}

Object *float_inplace_div( Object *me, Object *op ){
    ob_float_ucast(me)->value /= ob_fvalue(op);

    return me;
}

Object *float_inplace_mod( Object *me, Object *op ){
    double a = ob_float_ucast(me)->value,
           b = ob_fvalue(op);

    ob_float_ucast(me)->value = fmod( a, b );

    return me;
}

/** bitwise operators **/
Object *float_bw_and( Object *me, Object *op ){
    return (Object *)gc_new_float( (int)ob_float_ucast(me)->value & ob_ivalue(op) );
}

Object *float_bw_or( Object *me, Object *op ){
    return (Object *)gc_new_float( (int)ob_float_ucast(me)->value | ob_ivalue(op) );
}

Object *float_bw_not( Object *me ){
    return (Object *)gc_new_float( ~(int)ob_float_ucast(me)->value );
}

Object *float_bw_xor( Object *me, Object *op ){
    return (Object *)gc_new_float( (int)ob_float_ucast(me)->value ^ ob_ivalue(op) );
}

Object *float_bw_lshift( Object *me, Object *op ){
    return (Object *)gc_new_float( (int)ob_float_ucast(me)->value << ob_ivalue(op) );
}

Object *float_bw_rshift( Object *me, Object *op ){
    return (Object *)gc_new_float( (int)ob_float_ucast(me)->value >> ob_ivalue(op) );
}

Object *float_bw_inplace_and( Object *me, Object *op ){
    long ivalue = float_ivalue(me);

    ivalue &= ob_ivalue(op);

    ob_float_ucast(me)->value = ivalue;

    return me;
}

Object *float_bw_inplace_or( Object *me, Object *op ){
    long ivalue = float_ivalue(me);

    ivalue |= ob_ivalue(op);

    ob_float_ucast(me)->value = ivalue;

    return me;
}

Object *float_bw_inplace_xor( Object *me, Object *op ){
    long ivalue = float_ivalue(me);

    ivalue ^= ob_ivalue(op);

    ob_float_ucast(me)->value = ivalue;

    return me;
}

Object *float_bw_inplace_lshift( Object *me, Object *op ){
    long ivalue = float_ivalue(me);

    ivalue <<= ob_ivalue(op);

    ob_float_ucast(me)->value = ivalue;

    return me;
}

Object *float_bw_inplace_rshift( Object *me, Object *op ){
    long ivalue = float_ivalue(me);

    ivalue >>= ob_ivalue(op);

    ob_float_ucast(me)->value = ivalue;

    return me;
}

/** logic operators **/
Object *float_l_not( Object *me ){
    return (Object *)gc_new_float( !ob_float_ucast(me)->value );
}

Object *float_l_same( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value == fvalue );
}

Object *float_l_diff( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value != fvalue );
}

Object *float_l_less( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value < fvalue );
}

Object *float_l_greater( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value > fvalue );
}

Object *float_l_less_or_same( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value <= fvalue );
}

Object *float_l_greater_or_same( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value >= fvalue );
}

Object *float_l_or( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value || fvalue );
}

Object *float_l_and( Object *me, Object *op ){
    double fvalue = ob_fvalue(op);

    return (Object *)gc_new_float( ob_float_ucast(me)->value && fvalue );
}

/** collection operators **/
Object *float_cl_concat( Object *me, Object *op ){
    string mvalue = ob_svalue(me),
           svalue = ob_svalue(op);

    return (Object *)gc_new_string( (mvalue + svalue).c_str() );
}

IMPLEMENT_TYPE(Float) {
    /** type code **/
    otFloat,
	/** type name **/
    "float",
	/** type basic size **/
    sizeof(double),

	/** generic function pointers **/
    float_typename, // type_name
	float_set_references, // set_references
	float_clone, // clone
	0, // free
	float_get_size, // get_size
	float_serialize, // serialize
	float_deserialize, // deserialize
	float_to_fd, // to_fd
	float_from_fd, // from_fd
	float_cmp, // cmp
	float_ivalue, // ivalue
	float_fvalue, // fvalue
	float_lvalue, // lvalue
	float_svalue, // svalue
	float_print, // print
	float_scanf, // scanf
	float_to_string, // to_string
	float_to_int, // to_int
	float_from_int, // from_int
	float_from_float, // from_float
	0, // range
	0, // regexp

	/** arithmetic operators **/
	float_assign, // assign
    float_factorial, // factorial
    float_increment, // increment
    float_decrement, // decrement
    float_minus, // minus
    float_add, // add
    float_sub, // sub
    float_mul, // mul
    float_div, // div
    float_mod, // mod
    float_inplace_add, // inplace_add
    float_inplace_sub, // inplace_sub
    float_inplace_mul, // inplace_mul
    float_inplace_div, // inplace_div
    float_inplace_mod, // inplace_mod

	/** bitwise operators **/
	float_bw_and, // bw_and
    float_bw_or, // bw_or
    float_bw_not, // bw_not
    float_bw_xor, // bw_xor
    float_bw_lshift, // bw_lshift
    float_bw_rshift, // bw_rshift
    float_bw_inplace_and, // bw_inplace_and
    float_bw_inplace_or, // bw_inplace_or
    float_bw_inplace_xor, // bw_inplace_xor
    float_bw_inplace_lshift, // bw_inplace_lshift
    float_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    float_l_not, // l_not
    float_l_same, // l_same
    float_l_diff, // l_diff
    float_l_less, // l_less
    float_l_greater, // l_greater
    float_l_less_or_same, // l_less_or_same
    float_l_greater_or_same, // l_greater_or_same
    float_l_or, // l_or
    float_l_and, // l_and

	/** collection operators **/
	float_cl_concat, // cl_concat
	0, // cl_inplace_concat
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	0, // cl_at
	0, // cl_set
	0, // cl_set_reference

    /** structure operators **/
	0, // attribute_access
	0, // set_attribute_access
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference
    0, // define_method
    0  // get_method
};

