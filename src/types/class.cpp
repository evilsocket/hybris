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
#include "node.h"
#include "vmem.h"
#include "context.h"

/** generic function pointers **/
void class_set_references( Object *me, int ref ){
    ClassObjectValueIterator vi;
    ClassObject *cme = ob_class_ucast(me);

    me->ref += ref;

    for( vi = cme->a_values.begin(); vi != cme->a_values.end(); vi++ ){
        ob_set_references( *vi, ref );
    }
}

Object *class_clone( Object *me ){
    ClassObject *cclone = gc_new_class(),
                *cme    = ob_class_ucast(me);
    ClassObjectAccessIterator ai;
    ClassObjectNameIterator   ni;
    ClassObjectValueIterator  vi;
    ClassObjectMethodIterator mi;

    for( ni = cme->a_names.begin(), vi = cme->a_values.begin(), ai = cme->a_access.begin();
    	 ni != cme->a_names.end();
    	 ni++, vi++, ai++ ){
    	ob_set_attribute( (Object *)cclone, (char *)(*ni).c_str(), (*vi) );
    	cclone->a_access.push_back( (*ai) );
    }
    for( ni = cme->m_names.begin(), mi = cme->m_values.begin(); ni != cme->m_names.end(); ni++, mi++ ){
    	ob_define_method( (Object *)cclone, (char *)(*ni).c_str(), (*mi) );
    }

    cclone->items = cme->items;
    cclone->name  = cme->name;

    return (Object *)cclone;
}

size_t class_get_size( Object *me ){
	return ob_class_ucast(me)->items;
}

void class_free( Object *me ){
	ClassObjectNameIterator   ni;
    ClassObjectValueIterator  vi;
    ClassObjectMethodIterator mi;
    ClassObject *cme = ob_class_ucast(me);
    Object      *vitem;

    /*
     * Check if the class has a destructors and call it.
     */
	for( ni = cme->m_names.begin(), mi = cme->m_values.begin(); ni != cme->m_names.end(); ni++, mi++ ){
		if( *ni == "__expire" ){
			Node *dtor = *mi;
			vframe_t stack;
			extern Context __context;

			stack.insert( "me", me );

			__context.trace( "__expire", &stack );

			__context.engine->exec( &stack, dtor->callBody() );

			__context.detrace();
		}
	}

    for( vi = cme->a_values.begin(); vi != cme->a_values.end(); vi++ ){
        vitem = *vi;
        if( vitem ){
            ob_free(vitem);
        }
    }

    cme->a_access.clear();
    cme->a_names.clear();
    cme->a_values.clear();
    cme->m_names.clear();
    cme->m_values.clear();
    cme->items = 0;
}

long class_ivalue( Object *me ){
    return static_cast<long>( ob_class_ucast(me)->items );
}

double class_fvalue( Object *me ){
    return static_cast<double>( ob_class_ucast(me)->items );
}

bool class_lvalue( Object *me ){
    return static_cast<bool>( ob_class_ucast(me)->items );
}

string class_svalue( Object *me ){
    return string( "<class>" );
}

void class_print( Object *me, int tabs ){
	ClassObject *cme = ob_class_ucast(me);
	ClassObjectNameIterator   ni;
	ClassObjectValueIterator  vi;
	ClassObjectMethodIterator mi;
	int j;

	for( j = 0; j < tabs; ++j ){
		printf( "\t" );
	}
	printf( "class {\n" );
	for( ni = cme->a_names.begin(), vi = cme->a_values.begin(); ni != cme->a_names.end(); ni++, vi++ ){
		for( j = 0; j < tabs + 1; ++j ) printf( "\t" );
		printf( "%s %s -> ", (*vi)->type->name, (*ni).c_str() );
		ob_print( *vi, tabs + 1 );
		printf( "\n" );
	}
	printf( "\n" );
	for( ni = cme->m_names.begin(), mi = cme->m_values.begin(); ni != cme->m_names.end(); ni++, mi++ ){
		for( j = 0; j < tabs + 1; ++j ) printf( "\t" );
		printf( "method %s { ... }\n", (*ni).c_str() );
	}
	for( j = 0; j < tabs; ++j ) printf( "\t" );
	printf( "}\n" );
}

/** arithmetic operators **/
Object *class_assign( Object *me, Object *op ){
    class_free(me);

    Object *clone = ob_clone(op);

    ob_set_references( clone, +1 );

    return (me = clone);
}

/** structure operators **/
void class_add_attribute( Object *me, char *name ){
    ClassObject *cme = ob_class_ucast(me);

    cme->a_names.push_back( name );
    cme->a_values.push_back( (Object *)gc_new_integer(0) );
    cme->items = cme->a_names.size() + cme->m_names.size();
}

Object *class_get_attribute( Object *me, char *name ){
    ClassObject *cme = ob_class_ucast(me);

    int i, sz( cme->a_names.size() );
    for( i = 0; i < sz; ++i ){
        if( cme->a_names[i] == string(name) ){
            return cme->a_values[i];
        }
    }
    return NULL;
}

void class_set_attribute_reference( Object *me, char *name, Object *value ){
    ClassObject *cme = ob_class_ucast(me);
    int i, sz( cme->a_names.size() );

    for( i = 0; i < sz; ++i ){
        if( cme->a_names[i] == string(name) ){
            cme->a_values[i] = ob_assign( cme->a_values[i], value );
            return;
        }
    }

    cme->a_names.push_back( name );
    cme->a_values.push_back( value );
    cme->items = cme->a_names.size() + cme->m_names.size();
}

void class_set_attribute( Object *me, char *name, Object *value ){
    return ob_set_attribute_reference( me, name, ob_clone(value) );
}

void class_define_method( Object *me, char *name, Node *code ){
	ClassObject *cme = ob_class_ucast(me);

	cme->m_names.push_back( name );
	cme->m_values.push_back( code->clone() );
	cme->items = cme->a_names.size() + cme->m_names.size();
}

Node *class_get_method( Object *me, char *name, int argc ){
	ClassObject *cme = ob_class_ucast(me);
	Node *best_match = NULL;
	int   best_match_argc, match_argc;

	int i, sz( cme->m_names.size() );
	for( i = 0; i < sz; ++i ){
		if( cme->m_names[i] == string(name) ){
			if( argc < 0 ){
				return cme->m_values[i];
			}
			else{
				if( best_match == NULL ){
					/*
					 * The last child of a method is its body itself, so we compare
					 * call children with method->children() - 1 to ignore the body.
					 */
					best_match 		= cme->m_values[i];
					best_match_argc = best_match->children() - 1;
				}
				else{
					match_argc = cme->m_values[i]->children() - 1;
					if( match_argc != best_match_argc && match_argc == argc ){
						return cme->m_values[i];
					}
				}
			}
		}
	}
	return best_match;
}

IMPLEMENT_TYPE(Class) {
    /** type code **/
    otClass,
	/** type name **/
    "class",
	/** type basic size **/
    0,

	/** generic function pointers **/
	class_set_references, // set_references
	class_clone, // clone
	class_free, // free
	class_get_size, // get_size
	0, // serialize
	0, // deserialize
	0, // to_fd
	0, // from_fd
	0, // cmp
	class_ivalue, // ivalue
	class_fvalue, // fvalue
	class_lvalue, // lvalue
	class_svalue, // svalue
	class_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	0, // from_int
	0, // from_float
	0, // range
	0, // regexp

	/** arithmetic operators **/
	class_assign, // assign
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
    class_add_attribute, // add_attribute
    class_get_attribute, // get_attribute
    class_set_attribute, // set_attribute
    class_set_attribute_reference,  // set_attribute_reference
    class_define_method, // define_method
    class_get_method  // get_method
};

