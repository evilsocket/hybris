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
const char *char_typename( Object *o ){
	return o->type->name;
}

void char_set_references( Object *me, int ref ){
    me->ref += ref;
}

Object *char_clone( Object *me ){
    return (Object *)gc_new_char( ob_char_ucast(me)->value );
}

size_t char_get_size( Object *me ){
	return sizeof(char);
}

byte *char_serialize( Object *o, size_t size ){
	byte *buffer = new byte;

	*buffer = (byte)ob_char_ucast(o)->value;

	return buffer;
}

Object *char_deserialize( Object *o, byte *buffer, size_t size ){
	if( size ){
		o = ob_dcast( gc_new_char(*buffer) );
	}

	return o;
}

Object *char_to_fd( Object *o, int fd, size_t size ){
	size_t s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o));
	int    written;

	written = write( fd, &((ob_char_ucast(o))->value), s );

	return ob_dcast( gc_new_integer(written) );
}

Object *char_from_fd( Object *o, int fd, size_t size ){
	int rd = 0;
	if( size ){
		rd = read( fd, &((ob_char_ucast(o))->value), size );
	}
	else{
		return ob_from_fd( o, fd, sizeof(char) );
	}
	return ob_dcast( gc_new_integer(rd) );
}

int char_cmp( Object *me, Object *cmp ){
    long ivalue = ob_ivalue(cmp),
         mvalue = ob_char_ucast(me)->value;

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

long char_ivalue( Object *me ){
    return (long)ob_char_ucast(me)->value;
}

double char_fvalue( Object *me ){
    return (double)ob_char_ucast(me)->value;
}

bool char_lvalue( Object *me ){
    return (bool)ob_char_ucast(me)->value;
}

string char_svalue( Object *me ){
    char svalue[0xFF] = {0};

    sprintf( svalue, "%c", ob_char_ucast(me)->value );

    return string(svalue);
}

void char_print( Object *me, int tabs ){
    for( int i = 0; i < tabs; ++i ){
        printf( "\t" );
    }
    printf( "%c", ob_char_ucast(me)->value );
}

void char_scanf( Object *me ){
    scanf( "%c", &ob_char_ucast(me)->value );
}

Object * char_to_string( Object *me ){
    DECLARE_TYPE(String);

    IntegerObject tmp( ob_char_ucast(me)->value );

    return String_Type.from_int((Object *)&tmp);
}

Object * char_to_int( Object *me ){
    return (Object *)gc_new_integer( ob_char_ucast(me)->value );
}

Object * char_from_int( Object *i ){
    return (Object *)gc_new_char( (ob_int_ucast(i))->value );
}

Object * char_from_float( Object *f ){
    return (Object *)gc_new_char( ob_float_ucast(f)->value );
}

Object *char_range( Object *a, Object *b ){
	char start, end;
	int  i;
	Object *range = ob_dcast( gc_new_vector() );

	if( ob_cmp( a, b ) == -1 ){
		start = ob_char_ucast(a)->value;
		end   = ob_char_ucast(b)->value;
	}
	else{
		start = ob_char_ucast(b)->value;
		end   = ob_char_ucast(a)->value;
	}

	for( i = start; i <= end; ++i ){
		ob_cl_push_reference( range, ob_dcast( gc_new_char(i) ) );
	}

	return range;
}

/** arithmetic operators **/
Object *char_assign( Object *me, Object *op ){
    if( ob_is_char(op) ){
        ob_char_ucast(me)->value = ob_char_ucast(op)->value;
    }
    else {
        Object *clone = ob_clone(op);
        ob_set_references( clone, +1 );

        me = clone;
    }

    return me;
}

Object *char_factorial( Object *me ){
    long ivalue = char_ivalue(me),
         ifact  = 1,
         i;

    for( i = 1; i <= ivalue; ++i ){
        ifact *= i;
    }

    return (Object *)gc_new_char(ifact);
}

Object *char_increment( Object *me ){
    ob_char_ucast(me)->value++;

    return me;
}

Object *char_decrement( Object *me ){
    ob_char_ucast(me)->value--;

    return me;
}

Object *char_minus( Object *me ){
    return (Object *)gc_new_char( -ob_char_ucast(me)->value );
}

Object *char_add( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value + ivalue );
}

Object *char_sub( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value - ivalue );
}

Object *char_mul( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value * ivalue );
}

Object *char_div( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value / ivalue );
}

Object *char_mod( Object *me, Object *op ){
    long a = ob_char_ucast(me)->value,
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

    return (Object *)gc_new_char(mod);
}

Object *char_inplace_add( Object *me, Object *op ){
    ob_char_ucast(me)->value += ob_ivalue(op);

    return me;
}

Object *char_inplace_sub( Object *me, Object *op ){
    ob_char_ucast(me)->value -= ob_ivalue(op);

    return me;
}

Object *char_inplace_mul( Object *me, Object *op ){
    ob_char_ucast(me)->value *= ob_ivalue(op);

    return me;
}

Object *char_inplace_div( Object *me, Object *op ){
    ob_char_ucast(me)->value /= ob_ivalue(op);

    return me;
}

Object *char_inplace_mod( Object *me, Object *op ){
    long a = ob_char_ucast(me)->value,
         b = ob_ivalue(op);

	/* b is 0 or 1 */
    if( b == 0 || b == 1 ){
        ob_char_ucast(me)->value = 0;
    }
    /* b is a power of 2 */
    else if( (b & (b - 1)) == 0 ){
        ob_char_ucast(me)->value = a & (b - 1);
    }
    else{
        ob_char_ucast(me)->value = a % b;
    }

    return me;
}

/** bitwise operators **/
Object *char_bw_and( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value & ivalue );
}

Object *char_bw_or( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value | ivalue );
}

Object *char_bw_not( Object *me ){
    return (Object *)gc_new_char( ~ob_char_ucast(me)->value );
}

Object *char_bw_xor( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value ^ ivalue );
}

Object *char_bw_lshift( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value << ivalue );
}

Object *char_bw_rshift( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value >> ivalue );
}

Object *char_bw_inplace_and( Object *me, Object *op ){
    ob_char_ucast(me)->value &= ob_ivalue(op);

    return me;
}

Object *char_bw_inplace_or( Object *me, Object *op ){
    ob_char_ucast(me)->value &= ob_ivalue(op);

    return me;
}

Object *char_bw_inplace_xor( Object *me, Object *op ){
    ob_char_ucast(me)->value ^= ob_ivalue(op);

    return me;
}

Object *char_bw_inplace_lshift( Object *me, Object *op ){
    ob_char_ucast(me)->value <<= ob_ivalue(op);

    return me;
}

Object *char_bw_inplace_rshift( Object *me, Object *op ){
    ob_char_ucast(me)->value >>= ob_ivalue(op);

    return me;
}

/** logic operators **/
Object *char_l_not( Object *me ){
    return (Object *)gc_new_char( !ob_char_ucast(me)->value );
}

Object *char_l_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value == ivalue );
}

Object *char_l_diff( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value != ivalue );
}

Object *char_l_less( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value < ivalue );
}

Object *char_l_greater( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value > ivalue );
}

Object *char_l_less_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value <= ivalue );
}

Object *char_l_greater_or_same( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value >= ivalue );
}

Object *char_l_or( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value || ivalue );
}

Object *char_l_and( Object *me, Object *op ){
    long ivalue = ob_ivalue(op);

    return (Object *)gc_new_char( ob_char_ucast(me)->value && ivalue );
}

/** collection operators **/
Object *char_cl_concat( Object *me, Object *op ){
    string mvalue = ob_svalue(me),
           svalue = ob_svalue(op);

    return (Object *)gc_new_string( (mvalue + svalue).c_str() );
}

IMPLEMENT_TYPE(Char) {
    /** type code **/
    otChar,
	/** type name **/
    "char",
	/** type basic size **/
    sizeof(char),

	/** generic function pointers **/
    char_typename, // type_name
	char_set_references, // set_references
	char_clone, // clone
	0, // free
	char_get_size, // get_size
	char_serialize, // serialize
	char_deserialize, // deserialize
	char_to_fd, // to_fd
	char_from_fd, // from_fd
	char_cmp, // cmp
	char_ivalue, // ivalue
	char_fvalue, // fvalue
	char_lvalue, // lvalue
	char_svalue, // svalue
	char_print, // print
	char_scanf, // scanf
	char_to_string, // to_string
	char_to_int, // to_int
	char_from_int, // from_int
	char_from_float, // from_float
	char_range, // range
	0, // regexp

	/** arithmetic operators **/
	char_assign, // assign
    char_factorial, // factorial
    char_increment, // increment
    char_decrement, // decrement
    char_minus, // minus
    char_add, // add
    char_sub, // sub
    char_mul, // mul
    char_div, // div
    char_mod, // mod
    char_inplace_add, // inplace_add
    char_inplace_sub, // inplace_sub
    char_inplace_mul, // inplace_mul
    char_inplace_div, // inplace_div
    char_inplace_mod, // inplace_mod

	/** bitwise operators **/
	char_bw_and, // bw_and
    char_bw_or, // bw_or
    char_bw_not, // bw_not
    char_bw_xor, // bw_xor
    char_bw_lshift, // bw_lshift
    char_bw_rshift, // bw_rshift
    char_bw_inplace_and, // bw_inplace_and
    char_bw_inplace_or, // bw_inplace_or
    char_bw_inplace_xor, // bw_inplace_xor
    char_bw_inplace_lshift, // bw_inplace_lshift
    char_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    char_l_not, // l_not
    char_l_same, // l_same
    char_l_diff, // l_diff
    char_l_less, // l_less
    char_l_greater, // l_greater
    char_l_less_or_same, // l_less_or_same
    char_l_greater_or_same, // l_greater_or_same
    char_l_or, // l_or
    char_l_and, // l_and

	/** collection operators **/
	char_cl_concat, // cl_concat
	0, // cl_inplace_concat
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
	0, // set_attribute_access
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference
    0, // define_method
    0  // get_method
};

