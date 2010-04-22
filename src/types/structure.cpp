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

/** generic function pointers **/
const char *struct_typename( Object *o ){
	return o->type->name;
}

void struct_set_references( Object *me, int ref ){
    StructureObjectValueIterator vi;
    StructureObject *sme = ob_struct_ucast(me);

    me->ref += ref;

    for( vi = sme->values.begin(); vi != sme->values.end(); vi++ ){
        ob_set_references( *vi, ref );
    }
}

Object *struct_clone( Object *me ){
    StructureObject *sclone = gc_new_struct(),
                    *sme    = ob_struct_ucast(me);
    StructureObjectNameIterator  ni;
    StructureObjectValueIterator vi;

    for( ni = sme->names.begin(), vi = sme->values.begin(); ni != sme->names.end(); ni++, vi++ ){
        sclone->type->set_attribute( (Object *)sclone, (char *)(*ni).c_str(), *vi );
    }

    sclone->items = sme->items;

    return (Object *)sclone;
}

size_t struct_get_size( Object *me ){
	return ob_struct_ucast(me)->items;
}

void struct_free( Object *me ){
    StructureObjectValueIterator vi;
    StructureObject *sme = ob_struct_ucast(me);
    Object          *vitem;

    for( vi = sme->values.begin(); vi != sme->values.end(); vi++ ){
        vitem = *vi;
        if( vitem ){
            ob_free(vitem);
        }
    }

    sme->names.clear();
    sme->values.clear();
    sme->items = 0;
}

int struct_cmp( Object *me, Object *cmp ){
    if( !ob_is_struct(cmp) ){
        return 1;
    }
    else {
        StructureObject *sme  = ob_struct_ucast(me),
                  *scmp = ob_struct_ucast(cmp);
        size_t     sme_nsize( sme->names.size() ),
                   mcmp_nsize( scmp->names.size() ),
                   sme_vsize( sme->values.size() ),
                   mcmp_vsize( scmp->values.size() );

        if( sme_nsize > mcmp_nsize || sme_vsize > mcmp_vsize ){
            return 1;
        }
        else if( sme_nsize < mcmp_nsize || sme_vsize < mcmp_vsize ){
            return -1;
        }
        /*
         * Same type and same size, let's check the elements.
         */
        else{
            size_t i;
            int    diff;

            for( i = 0; i < sme_nsize; ++i ){
                if( sme->names[i] != scmp->names[i] ){
                    return 1;
                }
                diff = ob_cmp( sme->values[i], scmp->values[i] );
                if( diff != 0 ){
                    return diff;
                }
            }
            return 0;
        }
    }
}

long struct_ivalue( Object *me ){
    return static_cast<long>( ob_struct_ucast(me)->items );
}

double struct_fvalue( Object *me ){
    return static_cast<double>( ob_struct_ucast(me)->items );
}

bool struct_lvalue( Object *me ){
    return static_cast<bool>( ob_struct_ucast(me)->items );
}

string struct_svalue( Object *me ){
    return string( "<struct>" );
}

void struct_print( Object *me, int tabs ){
    StructureObject *sme = ob_struct_ucast(me);
    int        i, j;

    printf( "struct {\n" );
    for( i = 0; i < sme->items; ++i ){
        for( j = 0; j <= tabs; ++j ) printf( "\t" );
        printf( "%s : ", sme->names[i].c_str() );
        ob_print( sme->values[i], tabs + 1 );
        printf( "\n" );
    }
    for( i = 0; i < tabs; ++i ) printf( "\t" );
    printf( "}\n" );
}

/** arithmetic operators **/
Object *struct_assign( Object *me, Object *op ){
    struct_free(me);

    Object *clone = ob_clone(op);

    ob_set_references( clone, +1 );

    return (me = clone);
}

/** structure operators **/
void struct_add_attribute( Object *me, char *name ){
    StructureObject *sme = ob_struct_ucast(me);

    sme->names.push_back( name );
    sme->values.push_back( (Object *)gc_new_integer(0) );
    sme->items = sme->names.size();
}

Object *struct_get_attribute( Object *me, char *name ){
    StructureObject *sme = ob_struct_ucast(me);

    int i, sz( sme->items );
    for( i = 0; i < sz; ++i ){
        if( sme->names[i] == string(name) ){
            return sme->values[i];
        }
    }
    return NULL;

}

void struct_set_attribute_reference( Object *me, char *name, Object *value ){
    StructureObject *sme = ob_struct_ucast(me);
    Object          *o;
    int i, sz( sme->items );

    for( i = 0; i < sz; ++i ){
        if( sme->names[i] == string(name) ){
            sme->values[i] = ob_assign( sme->values[i], value );
            return;
        }
    }

    sme->names.push_back( name );
    sme->values.push_back( value );
    sme->items = sme->names.size();
}

void struct_set_attribute( Object *me, char *name, Object *value ){
    return ob_set_attribute_reference( me, name, ob_clone(value) );
}

IMPLEMENT_TYPE(Structure) {
    /** type code **/
    otStructure,
	/** type name **/
    "struct",
	/** type basic size **/
    0,

	/** generic function pointers **/
    struct_typename, // type_name
	struct_set_references, // set_references
	struct_clone, // clone
	struct_free, // free
	struct_get_size, // get_size
	0, // serialize
	0, // deserialize
	0, // to_fd
	0, // from_fd
	struct_cmp, // cmp
	struct_ivalue, // ivalue
	struct_fvalue, // fvalue
	struct_lvalue, // lvalue
	struct_svalue, // svalue
	struct_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	0, // from_int
	0, // from_float
	0, // range
	0, // regexp

	/** arithmetic operators **/
	struct_assign, // assign
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
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	0, // cl_at
	0, // cl_set
	0, // cl_set_reference

	/** structure operators **/
    struct_add_attribute, // add_attribute
    struct_get_attribute, // get_attribute
    struct_set_attribute, // set_attribute
    struct_set_attribute_reference, // set_attribute_reference
    0, // define_method
    0  // get_method
};

