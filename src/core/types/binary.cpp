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
Object *binary_traverse( Object *me, int index ){
	return (index >= ((Binary *)me)->value.size() ? NULL : ((Binary *)me)->value.at(index));
}

Object *binary_clone( Object *me ){
    BinaryIterator i;
    Binary *bclone = gc_new_binary(),
                 *bme    = (Binary *)me;

    vv_foreach( i, bme->value ){
        ob_cl_push_reference( (Object *)bclone, ob_clone( *i ) );
    }

    return (Object *)bclone;
}

void binary_free( Object *me ){
    Binary *bme = (Binary *)me;

    bme->items = 0;
    bme->value.clear();
}

size_t binary_get_size( Object *me ){
	return ob_binary_ucast(me)->items;
}

byte *binary_serialize( Object *o, size_t size ){
	size_t i, s   = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o) );
	byte  *buffer = new byte[s];
	Binary *bme = (Binary *)o;

	for( i = 0; i < s; ++i ){
		buffer[i] = (byte)ob_ivalue( bme->value[i] );
	}

	return buffer;
}

Object *binary_deserialize( Object *o, byte *buffer, size_t size ){
	size_t i;

	if( size ){
		o = ob_dcast( gc_new_binary() );
		for( i = 0; i < size; ++i ){
			ob_cl_push( o, ob_dcast( gc_new_integer(buffer[i]) ) );
		}
	}

	return o;
}

int binary_cmp( Object *me, Object *cmp ){
    if( !ob_is_binary(cmp) ){
        return 1;
    }
    else {
        Binary *bme  = (Binary *)me,
                     *bcmp = (Binary *)cmp;
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
    return static_cast<long>( ob_binary_ucast(me)->items );
}

double binary_fvalue( Object *me ){
    return static_cast<double>( ob_binary_ucast(me)->items );
}

bool binary_lvalue( Object *me ){
    return static_cast<bool>( ob_binary_ucast(me)->items );
}

string binary_svalue( Object *me ){
	return string("<binary>");
}

void binary_print( Object *me, int tabs ){
    BinaryIterator i;
    Binary *bme = (Binary *)me;
    Object       *item;
    int           j;

    for( j = 0; j < tabs; ++j ){
        fprintf( stdout, "\t" );
    }
    fprintf( stdout, "binary {\n" );
    vv_foreach( i, bme->value ){
        item = *i;
        fprintf( stdout, "%.2X", ((Char *)item)->value );
    }
    for( j = 0; j < tabs; ++j ) fprintf( stdout, "\t" );
    fprintf( stdout, "\n}\n" );
}

/** arithmetic operators **/
Object *binary_assign( Object *me, Object *op ){
    binary_free(me);

    Object *clone = ob_clone(op);

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

    if( ob_is_char(o) == false && ob_is_int(o) == false && ob_is_float(o) == false ){
        hyb_error( H_ET_SYNTAX, "binary type allows only char, int or float types in its subscript operator" );
    }

    ((Binary *)me)->value.push_back( o );
    ob_binary_ucast(me)->items++;

    return me;
}

Object *binary_cl_at( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_binary_ucast(me)->items ){
    	return vm_raise_exception( "index out of bounds" );
    }

    return ((Binary *)me)->value[idx];
}

Object *binary_cl_set( Object *me, Object *i, Object *v ){
    return ob_cl_set_reference( me, i, ob_clone(v) );
}

Object *binary_cl_set_reference( Object *me, Object *i, Object *v ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_binary_ucast(me)->items ){
    	return vm_raise_exception( "index out of bounds" );
    }

    DECLARE_TYPE(Char);
    DECLARE_TYPE(Integer);
    DECLARE_TYPE(Float);

    if( ob_is_char(v) == false && ob_is_int(v) == false && ob_is_float(v) == false ){
        hyb_error( H_ET_SYNTAX, "binary type allows only char, int or float types in its subscript operator" );
    }

    Object *old = ((Binary *)me)->value[idx];

    ob_free(old);

    ((Binary *)me)->value[idx] = v;

    return me;
}

IMPLEMENT_TYPE(Binary) {
    /** type code **/
    otBinary,
	/** type name **/
    "binary",
	/** type basic size **/
    OB_COLLECTION_SIZE,
    /** type builtin methods **/
    { OB_BUILIN_METHODS_END_MARKER },

	/** generic function pointers **/
    0, // type_name
    binary_traverse, // traverse
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
	binary_cl_push, // cl_push
	binary_cl_push_reference, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	binary_cl_at, // cl_at
	binary_cl_set, // cl_set
	binary_cl_set_reference, // cl_set_reference

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

