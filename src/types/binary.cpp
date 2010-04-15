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
#include <math.h>
#include <stdio.h>

/** generic function pointers **/
void binary_set_references( Object *me, int ref ){
    BinaryObjectIterator i;
    BinaryObject *bme = (BinaryObject *)me;

    me->ref += ref;

    for( i = bme->value.begin(); i != bme->value.end(); i++ ){
        ob_set_references( *i, ref );
    }
}

Object *binary_clone( Object *me ){
    BinaryObjectIterator i;
    BinaryObject *bclone = MK_BINARY_OBJ(),
                 *bme    = (BinaryObject *)me;

    for( i = bme->value.begin(); i != bme->value.end(); i++ ){
        ob_cl_push_reference( (Object *)bclone, ob_clone( *i ) );
    }

    return (Object *)bclone;
}

void binary_free( Object *me ){
    BinaryObjectIterator i;
    BinaryObject *bme = (BinaryObject *)me;
    Object       *item;

    for( i = bme->value.begin(); i != bme->value.end(); i++ ){
        item = *i;
        if( item && ob_free(item) == true ){
            bme->items--;
        }
    }
    bme->value.clear();
}

size_t binary_get_size( Object *me ){
	return BINARY_UPCAST(me)->items;
}

byte *binary_serialize( Object *o, size_t size ){
	size_t i, s   = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o) );
	byte  *buffer = new byte[s];
	BinaryObject *bme = (BinaryObject *)o;

	for( i = 0; i < s; ++i ){
		buffer[i] = (byte)ob_ivalue( bme->value[i] );
	}

	return buffer;
}

Object *binary_deserialize( Object *o, byte *buffer, size_t size ){
	size_t i;

	if( size ){
		o = OB_DOWNCAST( MK_BINARY_OBJ() );
		for( i = 0; i < size; ++i ){
			ob_cl_push( o, OB_DOWNCAST( MK_INT_OBJ(buffer[i]) ) );
		}
	}

	return o;
}

int binary_cmp( Object *me, Object *cmp ){
    if( !IS_BINARY_TYPE(cmp) ){
        return 1;
    }
    else {
        BinaryObject *bme  = (BinaryObject *)me,
                     *bcmp = (BinaryObject *)cmp;
        size_t        bme_size( bme->value.size() ),
                      bcmp_size( bcmp->value.size() );

        if( bme_size > bcmp_size ){
            return 1;
        }
        if( bme_size < bcmp_size ){
            return -1;
        }
        /*
         * Same type and same size, let's check the elements.
         */
        else{
            size_t i;
            int    diff;

            for( i = 0; i < bme_size; ++i ){
                diff = ob_cmp( bme->value[i], bcmp->value[i] );
                if( diff != 0 ){
                    return diff;
                }
            }
            return 0;
        }
    }
}

long binary_ivalue( Object *me ){
    return static_cast<long>( BINARY_UPCAST(me)->items );
}

double binary_fvalue( Object *me ){
    return static_cast<double>( BINARY_UPCAST(me)->items );
}

bool binary_lvalue( Object *me ){
    return static_cast<bool>( BINARY_UPCAST(me)->items );
}

string binary_svalue( Object *me ){
	return string("<binary>");
}

void binary_print( Object *me, int tabs ){
    BinaryObjectIterator i;
    BinaryObject *bme = (BinaryObject *)me;
    Object       *item;
    int           j;

    for( j = 0; j < tabs; ++j ){
        printf( "\t" );
    }
    printf( "binary {\n" );
    for( i = bme->value.begin(); i != bme->value.end(); i++ ){
        item = *i;
        printf( "%.2X", ((CharObject *)item)->value );
    }
    for( j = 0; j < tabs; ++j ) printf( "\t" );
    printf( "\n}\n" );
}

/** arithmetic operators **/
Object *binary_assign( Object *me, Object *op ){
    /*
     * Decrement my items reference count (not mine).
     */
    binary_free(me);

    Object *clone = ob_clone(op);

    ob_set_references( clone, +1 );

    return clone;
}

/** collection operators **/
Object *binary_cl_push( Object *me, Object *o ){
    return ob_cl_push_reference( me, ob_clone(o) );
}

Object *binary_cl_push_reference( Object *me, Object *o ){
    DECLARE_TYPE(Char);
    DECLARE_TYPE(Integer);
    DECLARE_TYPE(Float);

    if( ob_is_type_in( o, &Char_Type, &Integer_Type, &Float_Type, NULL ) == false ){
        hyb_throw( H_ET_SYNTAX, "binary type allows only char, int or float types in its subscript operator" );
    }

    ((BinaryObject *)me)->value.push_back( o );
    BINARY_UPCAST(me)->items++;

    return me;
}

Object *binary_cl_at( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);
    #ifdef BOUNDS_CHECK
    if( idx >= BINARY_UPCAST(me)->items ){
        hyb_throw( H_ET_GENERIC, "index out of bounds" );
    }
    #endif

    return ((BinaryObject *)me)->value[idx];
}

Object *binary_cl_set( Object *me, Object *i, Object *v ){
    return ob_cl_set_reference( me, i, ob_clone(v) );
}

Object *binary_cl_set_reference( Object *me, Object *i, Object *v ){
    size_t idx = ob_ivalue(i);
    #ifdef BOUNDS_CHECK
    if( idx >= BINARY_UPCAST(me)->items ){
        hyb_throw( H_ET_GENERIC, "index out of bounds" );
    }
    #endif

    DECLARE_TYPE(Char);
    DECLARE_TYPE(Integer);
    DECLARE_TYPE(Float);

    if( ob_is_type_in( v, &Char_Type, &Integer_Type, &Float_Type, NULL ) == false ){
        hyb_throw( H_ET_SYNTAX, "binary type allows only char, int or float types in its subscript operator" );
    }

    Object *old = ((BinaryObject *)me)->value[idx];

    ob_free(old);

    ((BinaryObject *)me)->value[idx] = v;

    return me;
}

IMPLEMENT_TYPE(Binary) {
    /** type code **/
    otBinary,
	/** type name **/
    "binary",
	/** type basic size **/
    0,

	/** generic function pointers **/
	binary_set_references, // set_references
	binary_clone, // clone
	binary_free, // free
	binary_get_size, // get_size
	binary_serialize, // serialize
	binary_deserialize, // deserialize
	0, // to_fd
	0, // from_fd
	binary_cmp, // cmp
	binary_ivalue, // ivalue
	binary_fvalue, // fvalue
	binary_lvalue, // lvalue
	binary_svalue, // svalue
	binary_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	0, // from_int
	0, // from_float
	0, // range
	0, // regexp

	/** arithmetic operators **/
	binary_assign, // assign
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
	binary_cl_push, // cl_push
	binary_cl_push_reference, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	binary_cl_at, // cl_at
	binary_cl_set, // cl_set
	binary_cl_set_reference, // cl_set_reference

	/** structure operators **/
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0  // set_attribute_reference;
};
