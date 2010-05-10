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

/** helpers **/
int map_find( Object *m, Object *key ){
    MapObject *mm = (MapObject *)m;
    size_t     i;

	for( i = 0; i < mm->items; ++i ){
		if( ob_cmp( mm->keys[i], key ) == 0 ){
			return i;
		}
	}
	return -1;
}

/** generic function pointers **/
const char *map_typename( Object *o ){
	return o->type->name;
}

Object *map_traverse( Object *me, int index ){
	MapObject *mme = (MapObject *)me;

	if( index < mme->keys.size() ){
		return mme->keys.at(index);
	}
	else if( index < mme->values.size() ){
		return mme->values.at(index);
	}
	else{
		return NULL;
	}
}

Object *map_clone( Object *me ){
    MapObjectIterator ki, vi;
    MapObject *mclone = gc_new_map(),
              *mme    = (MapObject *)me;
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
    MapObjectIterator ki, vi;
    MapObject *mme = (MapObject *)me;
    Object    *kitem,
              *vitem;

    for( ki = mme->keys.begin(), vi = mme->values.begin(); ki != mme->keys.end() && vi != mme->values.end(); ki++, vi++ ){
        kitem = *ki;
        vitem = *vi;

        if( kitem ){
            ob_free(kitem);
        }
        if( vitem ){
            ob_free(vitem);
        }
    }

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
        MapObject *mme  = (MapObject *)me,
                  *mcmp = (MapObject *)cmp;
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
    MapObjectIterator ki, vi;
    MapObject *mme = (MapObject *)me;
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
        hyb_error( H_ET_GENERIC, "could not pop an element from an empty map" );
    }

    Object *kitem = ((MapObject *)me)->keys[last_idx],
           *vitem = ((MapObject *)me)->values[last_idx];

    ((MapObject *)me)->keys.pop_back();
    ((MapObject *)me)->values.pop_back();

    ob_map_ucast(me)->items--;

    ob_free(kitem);

    return vitem;
}

Object *map_cl_remove( Object *me, Object *k ){
    int idx = map_find( me, k );
    if( idx != -1 ){
        Object *kitem = ((MapObject *)me)->keys[idx],
               *vitem = ((MapObject *)me)->values[idx];

		((MapObject *)me)->keys.erase( ((MapObject *)me)->keys.begin() + idx );
		((MapObject *)me)->values.erase( ((MapObject *)me)->values.begin() + idx );

		ob_map_ucast(me)->items--;

		ob_free(kitem);

        return vitem;
    }
    return me;
}

Object *map_cl_at( Object *me, Object *k ){
    int idx = map_find( me, k );
    if( idx != -1 ){
        return ((MapObject *)me)->values[idx];
    }
    else{
        hyb_error( H_ET_GENERIC, "no mapped values for label '%s'", ob_svalue(k).c_str() );
    }
    return me;
}

Object *map_cl_set_reference( Object *me, Object *k, Object *v ){
    int idx = map_find( me, k );
    if( idx != -1 ){
        Object *item = ((MapObject *)me)->values[idx];
        ob_free(item);

        ((MapObject *)me)->values[idx] = v;
    }
    else{
        ((MapObject *)me)->keys.push_back( k );
        ((MapObject *)me)->values.push_back( v );
        ob_map_ucast(me)->items++;
    }

    return me;
}

Object *map_cl_set( Object *me, Object *k, Object *v ){
    return map_cl_set_reference( me, ob_clone(k), ob_clone(v) );
}

IMPLEMENT_TYPE(Map) {
    /** type code **/
    otMap,
	/** type name **/
    "map",
	/** type basic size **/
    0,

	/** generic function pointers **/
    map_typename, // type_name
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
	0, // cl_concat
	0, // cl_inplace_concat
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
    0  // get_method
};

