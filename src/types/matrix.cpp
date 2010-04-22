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
const char *matrix_typename( Object *o ){
	return o->type->name;
}

void matrix_set_references( Object *me, int ref ){
    size_t x, y;
    MatrixObject *mme = ob_matrix_ucast(me);

    me->ref += ref;

    for( x = 0; x < mme->rows; ++x ){
        for( y = 0; y < mme->columns; ++y ){
            ob_set_references( mme->matrix[x][y], ref );
        }
    }
}

Object *matrix_clone( Object *me ){
    size_t x, y;
    vector<Object *> dummy;
    MatrixObject *mclone = gc_new_matrix(0,0,dummy),
                 *mme    = ob_matrix_ucast(me);
    Object       *clone;

    mclone->rows    = mme->rows;
    mclone->columns = mme->columns;
    mclone->items   = mme->items;
    mclone->matrix  = new Object ** [mme->rows];

    for( x = 0; x < mclone->rows; ++x ){
        mclone->matrix[x] = new Object * [mclone->columns];
    }

    for( x = 0; x < mclone->rows; ++x ){
        for( y = 0; y < mclone->columns; ++y ){
            clone                = ob_clone( mme->matrix[x][y] );
            mclone->matrix[x][y] = clone;
        }
    }

    return (Object *)mclone;
}

void matrix_free( Object *me ){
    MatrixObject *mme = ob_matrix_ucast(me);
    Object       *item;
    size_t        x, y;

    for( x = 0; x < mme->rows; ++x ){
        for( y = 0; y < mme->columns; ++y ){
            item = mme->matrix[x][y];
            ob_free(item);
        }
        delete [] mme->matrix[x];
    }
    delete [] mme->matrix;
    mme->rows       = 0;
    mme->columns    = 0;
    mme->items      = 0;
}

size_t matrix_get_size( Object *me ){
	return ob_matrix_ucast(me)->items;
}

int matrix_cmp( Object *me, Object *cmp ){
    if( !ob_is_matrix(cmp) ){
        return 1;
    }
    else {
        MatrixObject *mme  = ob_matrix_ucast(me),
                     *mcmp = ob_matrix_ucast(cmp);
        size_t        mme_size( mme->items ),
                      mcmp_size( mcmp->items ),
                      mme_rows( mme->rows ),
                      mcmp_rows( mcmp->rows ),
                      mme_cols( mme->columns ),
                      mcmp_cols( mcmp->columns );

        if( mme_size > mcmp_size || mme_rows > mcmp_rows || mme_cols > mcmp_cols ){
            return 1;
        }
        else if( mme_size < mcmp_size || mme_rows < mcmp_rows || mme_rows < mcmp_cols ){
            return -1;
        }
         /*
         * Same type and same size, let's check the elements.
         */
        else{
            size_t x, y;
            int    diff;

            for( x = 0; x < mme_rows; ++x ){
                for( y = 0; y < mme_cols; ++y ){
                    diff = ob_cmp( mme->matrix[x][y], mcmp->matrix[x][y] );
                    if( diff != 0 ){
                        return diff;
                    }
                }
            }
            return 0;
        }
    }
}

long matrix_ivalue( Object *me ){
    return static_cast<long>( ob_matrix_ucast(me)->items );
}

double matrix_fvalue( Object *me ){
    return static_cast<double>( ob_matrix_ucast(me)->items );
}

bool matrix_lvalue( Object *me ){
    return static_cast<bool>( ob_matrix_ucast(me)->items );
}

string matrix_svalue( Object *me ){
	return string("<matrix>");
}

void matrix_print( Object *me, int tabs ){
    MatrixObject *mme = ob_matrix_ucast(me);
    size_t        x, y, j;

    for( j = 0; j < tabs; ++j ){ printf( "\t" ); }
    printf( "matrix [%dx%d] {\n", mme->rows, mme->columns );
    for( x = 0; x < mme->rows; ++x ){
        for( y = 0; y < mme->columns; ++y ){
            ob_print( mme->matrix[x][y], tabs + 1 );
        }
        printf( "\n" );
    }
    for( j = 0; j < tabs; ++j ) printf( "\t" );
    printf( "}\n" );
}

/** arithmetic operators **/
Object *matrix_assign( Object *me, Object *op ){
    matrix_free(me);

    Object *clone = ob_clone(op);

    ob_set_references( clone, +1 );

    return me = clone;
}

Object *matrix_add( Object *me, Object *o ){
    MatrixObject *mme = ob_matrix_ucast(ob_clone(me));
    size_t x, y;

    if( ob_is_matrix(o) ){
        MatrixObject *mo = ob_matrix_ucast(o);
        if( mme->rows != mo->rows || mme->columns != mo->columns ){
            hyb_throw( H_ET_SYNTAX, "matrices have to be the same size" );
        }
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_add( mme->matrix[x][y], mo->matrix[x][y] );
            }
        }
    }
    else{
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_add( mme->matrix[x][y], o );
            }
        }
    }

    return (Object *)mme;
}

Object *matrix_sub( Object *me, Object *o ){
    MatrixObject *mme = ob_matrix_ucast(me);
    size_t x, y;

    if( ob_is_matrix(o) ){
        MatrixObject *mo = ob_matrix_ucast(o);
        if( mme->rows != mo->rows || mme->columns != mo->columns ){
            hyb_throw( H_ET_SYNTAX, "matrices have to be the same size" );
        }

        mme = ob_matrix_ucast( ob_clone(me) );
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_sub( mme->matrix[x][y], mo->matrix[x][y] );
            }
        }
    }
    else{
        mme = ob_matrix_ucast( ob_clone(me) );
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_sub( mme->matrix[x][y], o );
            }
        }
    }

    return (Object *)mme;
}

Object *matrix_mul( Object *me, Object *o ){
    MatrixObject *mme = ob_matrix_ucast(me),
                 *nm;
    unsigned int x, y, z;

    if( ob_is_matrix(o) ){
        MatrixObject *mo = ob_matrix_ucast(o);
        if( mme->columns != mo->rows ){
            hyb_throw( H_ET_SYNTAX, "first matrix columns have to be the same size of second matrix rows" );
        }

        nm = ob_matrix_ucast( gc_track( ob_dcast(new MatrixObject( mme->columns, mo->rows )), sizeof(MatrixObject) ) );
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mo->columns; ++y ){
                for( z = 0; z < mme->columns; ++z ){
                    nm->matrix[x][y] = ob_mul( mme->matrix[x][z], mo->matrix[z][y] );
                }
            }
        }
    }
    else{
        nm = ob_matrix_ucast( ob_clone(me) );
        for( x = 0; x < nm->rows; ++x ){
            for( y = 0; y < nm->columns; ++y ){
                ob_inplace_mul( nm->matrix[x][y], o );
            }
        }
    }

    return (Object *)nm;
}

Object *matrix_div( Object *me, Object *o ){
    MatrixObject *mme = ob_matrix_ucast( ob_clone(me) );
    size_t x, y;

    if( ob_is_matrix(o) ){
        MatrixObject *mo = ob_matrix_ucast(o);
        if( mme->rows != mo->rows || mme->columns != mo->columns ){
            hyb_throw( H_ET_SYNTAX, "matrices have to be the same size" );
        }
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_div( mme->matrix[x][y], mo->matrix[x][y] );
            }
        }
    }
    else{
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_div( mme->matrix[x][y], o );
            }
        }
    }

    return (Object *)mme;
}

Object *matrix_inplace_add( Object *me, Object *o ){
    MatrixObject *mme = ob_matrix_ucast(me);
    size_t x, y;

    if( ob_is_matrix(o) ){
        MatrixObject *mo = ob_matrix_ucast(o);
        if( mme->rows != mo->rows || mme->columns != mo->columns ){
            hyb_throw( H_ET_SYNTAX, "matrices have to be the same size" );
        }
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_add( mme->matrix[x][y], mo->matrix[x][y] );
            }
        }
    }
    else{
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_add( mme->matrix[x][y], o );
            }
        }
    }

    return (Object *)mme;
}

Object *matrix_inplace_sub( Object *me, Object *o ){
    MatrixObject *mme = ob_matrix_ucast(me);
    size_t x, y;

    if( ob_is_matrix(o) ){
        MatrixObject *mo = ob_matrix_ucast(o);
        if( mme->rows != mo->rows || mme->columns != mo->columns ){
            hyb_throw( H_ET_SYNTAX, "matrices have to be the same size" );
        }
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_sub( mme->matrix[x][y], mo->matrix[x][y] );
            }
        }
    }
    else{
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_sub( mme->matrix[x][y], o );
            }
        }
    }

    return (Object *)mme;
}

Object *matrix_inplace_mul( Object *me, Object *op ){
    MatrixObject *nm = (MatrixObject *)matrix_mul( me, op );

    ob_free(me);

    return (Object *)(me = (Object *)nm);
}

Object *matrix_inplace_div( Object *me, Object *o ){
    MatrixObject *mme = ob_matrix_ucast(me);
    size_t x, y;

    if( ob_is_matrix(o) ){
        MatrixObject *mo = ob_matrix_ucast(o);
        if( mme->rows != mo->rows || mme->columns != mo->columns ){
            hyb_throw( H_ET_SYNTAX, "matrices have to be the same size" );
        }
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_div( mme->matrix[x][y], mo->matrix[x][y] );
            }
        }
    }
    else{
        for( x = 0; x < mme->rows; ++x ){
            for( y = 0; y < mme->columns; ++y ){
                ob_inplace_div( mme->matrix[x][y], o );
            }
        }
    }

    return (Object *)mme;
}

/** collection operators **/
Object *matrix_cl_at( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);
    MatrixObject *mme = ob_matrix_ucast(me);

    #ifdef BOUNDS_CHECK
    if( idx >= mme->columns ){
        hyb_throw( H_ET_GENERIC, "index out of bounds" );
    }
    #endif

    VectorObject *vector = gc_new_vector();
    size_t        x = idx, y;

    for( y = 0; y < mme->rows; ++y ){
        ob_cl_push_reference( (Object *)vector, mme->matrix[x][y] );
    }

    return (Object *)vector;
}

IMPLEMENT_TYPE(Matrix) {
    /** type code **/
    otMatrix,
	/** type name **/
    "matrix",
	/** type basic size **/
    0,

	/** generic function pointers **/
    matrix_typename, // type_name
	matrix_set_references, // set_references
	matrix_clone, // clone
	matrix_free, // free
	matrix_get_size, // get_size
	0, // serialize
	0, // deserialize
	0, // to_fd
	0, // from_fd
	matrix_cmp, // cmp
	matrix_ivalue, // ivalue
	matrix_fvalue, // fvalue
	matrix_lvalue, // lvalue
	matrix_svalue, // svalue
	matrix_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	0, // from_int
	0, // from_float
	0, // range
	0, // regexp

	/** arithmetic operators **/
	matrix_assign, // assign
    0, // factorial
    0, // increment
    0, // decrement
    0, // minus
    matrix_add, // add
    matrix_sub, // sub
    matrix_mul, // mul
    matrix_div, // div
    0, // mod
    matrix_inplace_add, // inplace_add
    matrix_inplace_sub, // inplace_sub
    matrix_inplace_mul, // inplace_mul
    matrix_inplace_div, // inplace_div
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
	matrix_cl_at, // cl_at
	0, // cl_set
	0, // cl_set_reference

	/** structure operators **/
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference
    0, // define_method
    0  // get_method
};

