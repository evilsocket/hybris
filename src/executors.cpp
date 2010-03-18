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
#include "executors.h"

Object *hexc_identifier( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    int     idx;

    if( (o = hybris_vm_get( frame, (char *)node->_identifier.c_str() )) == H_UNDEFINED ){
        /* check if the identifier is a function name */
        if( (idx = ctx->HVC.index( (char *)node->_identifier.c_str() )) != -1 ){
            /* create alias */
            o = new Object((unsigned int)idx);
        }
        else{
            /* check for a global defined object if the frame is not the main one */
            if( reinterpret_cast<long>(frame) != reinterpret_cast<long>(&ctx->HVM) ){
                /* search in the main memory frame */
                if( (o = hybris_vm_get( &ctx->HVM, (char *)node->_identifier.c_str() )) == H_UNDEFINED ){
                    hybris_syntax_error( "'%s' undeclared identifier", node->_identifier.c_str() );
                }
            }
            else{
                hybris_syntax_error( "'%s' undeclared identifier", node->_identifier.c_str() );
            }
        }
    }
    return o;
}

Object *hexc_function( h_context_t *ctx, vmem_t *frame, Node *node ){
    /* check for double definition */
    if( hybris_vc_get( &ctx->HVC, (char *)node->_function.c_str() ) != H_UNDEFINED ){
        hybris_syntax_error( "function '%s' already defined", node->_function.c_str() );
    }
    else if( hfunction_search( ctx, (char *)node->_function.c_str() ) != H_UNDEFINED ){
        hybris_syntax_error( "function '%s' already defined as a language builtin", node->_function.c_str() );
    }
    /* add the function to the code segment */
    hybris_vc_add( &ctx->HVC, node );

    return H_UNDEFINED;
}

Object *hexc_dollar( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o    = H_UNDEFINED,
           *name = H_UNDEFINED;

    o    = htree_execute( ctx,  frame, node->child(0) );
    name = o->toString();

    if( (o = hybris_vm_get( frame, (char *)name->xstring.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", (char *)name->xstring.c_str() );
    }

    return o;
}

Object *hexc_pointer( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = htree_execute( ctx, frame, node->child(0) );
    res = new Object( (unsigned int)( reinterpret_cast<unsigned long>(o) ) );

    return res;
}

Object *hexc_object( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o   = H_UNDEFINED,
           *res = H_UNDEFINED;

    o   = htree_execute( ctx, frame, node->child(0) );
    res = o->getObject();

    return res;
}

Object *hexc_return( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = htree_execute( ctx,  frame, node->child(0) );
    return o;
}

Object *hexc_range( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *range = H_UNDEFINED,
           *from  = H_UNDEFINED,
           *to    = H_UNDEFINED;

    from  = htree_execute( ctx,  frame, node->child(0) );
    to    = htree_execute( ctx,  frame, node->child(1) );
    range = from->range( to );

    return range;
}

Object *hexc_subscript_add( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *object = H_UNDEFINED,
           *res    = H_UNDEFINED;

    array  = htree_execute( ctx, frame, node->child(0) );
    object = htree_execute( ctx, frame, node->child(1) );
    res    = array->push(object);

    return res;
}

Object *hexc_subscript_get( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *identifier = H_UNDEFINED,
           *array      = H_UNDEFINED,
           *index      = H_UNDEFINED,
           *res        = H_UNDEFINED;

    if( node->children() == 3 ){
        identifier    = htree_execute( ctx, frame, node->child(0) );
        array         = htree_execute( ctx, frame, node->child(1) );
        index         = htree_execute( ctx, frame, node->child(2) );
        (*identifier) = array->at( index );

        return identifier;
    }
    else{
        array = htree_execute( ctx, frame, node->child(0) );
        index = htree_execute( ctx, frame, node->child(1) );
        res   = array->at( index );

        return res;
    }
}

Object *hexc_subscript_set( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *index  = H_UNDEFINED,
           *object = H_UNDEFINED;

    array  = htree_execute( ctx, frame, node->child(0) );
    index  = htree_execute( ctx, frame, node->child(1) );
    object = htree_execute( ctx, frame, node->child(2) );

    array->at( index, object );

    return array;
}

Object *hexc_while( h_context_t *ctx, vmem_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    condition = node->child(0);
    body      = node->child(1);

    while( (boolean = htree_execute( ctx,  frame, condition ))->lvalue() ){
        result = htree_execute( ctx, frame, body );
    }

    return H_UNDEFINED;
}

Object *hexc_do( h_context_t *ctx, vmem_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    body      = node->child(0);
    condition = node->child(1);
    do{
        result = htree_execute( ctx, frame, body );
    }
    while( (boolean = htree_execute( ctx,  frame, condition ))->lvalue() );

    return H_UNDEFINED;
}

Object *hexc_for( h_context_t *ctx, vmem_t *frame, Node *node ){
    Node *condition,
         *increment,
         *body;

    Object *init    = H_UNDEFINED,
           *boolean = H_UNDEFINED,
           *inc     = H_UNDEFINED,
           *result  = H_UNDEFINED;

    init      = htree_execute( ctx,  frame, node->child(0) );
    condition = node->child(1);
    increment = node->child(2);
    body      = node->child(3);
    for( init;
         (boolean = htree_execute( ctx,  frame, condition ))->lvalue();
         (inc     = htree_execute( ctx,  frame, increment )) ){

        result = htree_execute( ctx, frame, body );
    }

    return H_UNDEFINED;
}

Object *hexc_foreach( h_context_t *ctx, vmem_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *identifier;

    identifier = (char *)node->child(0)->_identifier.c_str();
    map        = htree_execute( ctx, frame, node->child(1) );
    body       = node->child(2);
    size       = map->xarray.size();

    for( i = 0; i < size; i++ ){
        hybris_vm_add( frame, identifier, map->xarray[i] );
        result = htree_execute( ctx, frame, body );
    }

    return H_UNDEFINED;
}

Object *hexc_foreachm( h_context_t *ctx, vmem_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *key_identifier,
           *value_identifier;

    key_identifier   = (char *)node->child(0)->_identifier.c_str();
    value_identifier = (char *)node->child(1)->_identifier.c_str();
    map              = htree_execute( ctx, frame, node->child(2) );
    body             = node->child(3);
    size             = map->xmap.size();

    for( i = 0; i < size; i++ ){
        hybris_vm_add( frame, key_identifier,   map->xmap[i] );
        hybris_vm_add( frame, value_identifier, map->xarray[i] );
        result = htree_execute( ctx,  frame, body );
    }

    return H_UNDEFINED;
}

Object *hexc_if( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = htree_execute( ctx,  frame, node->child(0) );

    if( boolean->lvalue() ){
        result = htree_execute( ctx, frame, node->child(1) );
    }
    /* handle else case */
    else if( node->children() > 2 ){
        result = htree_execute( ctx, frame, node->child(2) );
    }

    return H_UNDEFINED;
}

Object *hexc_question( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = htree_execute( ctx,  frame, node->child(0) );

    if( boolean->lvalue() ){
        result = htree_execute( ctx, frame, node->child(1) );
    }
    else{
        result = htree_execute( ctx, frame, node->child(2) );
    }

    return result;
}

Object *hexc_eostmt( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *res_1 = H_UNDEFINED,
           *res_2 = H_UNDEFINED;

    res_1 = htree_execute( ctx, frame, node->child(0) );
    res_2 = htree_execute( ctx, frame, node->child(1) );

    return res_2;
}

Object *hexc_dot( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a      = H_UNDEFINED,
           *b      = H_UNDEFINED,
           *result = H_UNDEFINED;

    a      = htree_execute( ctx, frame, node->child(0) );
    b      = htree_execute( ctx, frame, node->child(1) );
    result = a->dot( b );

    return result;
}

Object *hexc_dote( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a      = H_UNDEFINED,
           *b      = H_UNDEFINED,
           *result = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b      = htree_execute( ctx, frame, node->child(1) );
    result = a->dotequal( b );

    return result;
}

Object *hexc_assign( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *identifier = H_UNDEFINED,
           *value      = H_UNDEFINED;


    value      = htree_execute( ctx,  frame, node->child(1) );
    identifier = hybris_vm_add( frame, (char *)node->child(0)->_identifier.c_str(), value );

    return identifier;
}

Object *hexc_uminus( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = htree_execute( ctx, frame, node->child(0) );
    result = -(*o);

    return result;
}

Object *hexc_regex( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *regexp = H_UNDEFINED,
           *result = H_UNDEFINED;

    o      = htree_execute( ctx,  frame, node->child(0) );
    regexp = htree_execute( ctx,  frame, node->child(1) );
    result = hrex_operator( o, regexp );

    return result;
}

Object *hexc_plus( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) + b;
}

Object *hexc_pluse( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) += b;

    return a;
}

Object *hexc_minus( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) - b;
}

Object *hexc_minuse( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) -= b;

    return a;
}

Object *hexc_mul( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) * b;
}

Object *hexc_mule( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) *= b;

    return a;
}

Object *hexc_div( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) / b;
}

Object *hexc_dive( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) /= b;

    return a;
}

Object *hexc_mod( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) % b;
}

Object *hexc_mode( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) %= b;

    return a;
}

Object *hexc_inc( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    if( (o = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
    }
    ++(*o);
    return o;
}

Object *hexc_dec( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    if( (o = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", (char *)node->child(0)->_identifier.c_str() );
    }
    --(*o);
    return o;
}

Object *hexc_xor( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) ^ b;
}

Object *hexc_xore( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) ^= b;

    return a;
}

Object *hexc_and( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) & b;
}

Object *hexc_ande( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) &= b;

    return a;
}

Object *hexc_or( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) | b;
}

Object *hexc_ore( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) |= b;

    return a;
}

Object *hexc_shiftl( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) << b;
}

Object *hexc_shiftle( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) <<= b;

    return a;
}

Object *hexc_shiftr( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0) );
    b = htree_execute( ctx, frame, node->child(1) );

    return (*a) >> b;
}

Object *hexc_shiftre( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    if( (a = hybris_vm_get( frame, (char *)node->child(0)->_identifier.c_str() )) == H_UNDEFINED ){
        hybris_syntax_error( "'%s' undeclared identifier", node->child(0)->_identifier.c_str() );
    }

    b = htree_execute( ctx, frame, node->child(1));

    (*a) >>= b;

    return a;
}

Object *hexc_fact( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = htree_execute( ctx,  frame, node->child(0) );

    return o->factorial();
}

Object *hexc_not( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = htree_execute( ctx,  frame, node->child(0) );

    return ~(*o);
}

Object *hexc_lnot( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = htree_execute( ctx,  frame, node->child(0) );

    return o->lnot();
}

Object *hexc_less( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) < b;
}

Object *hexc_greater( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) > b;
}

Object *hexc_ge( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) >= b;
}

Object *hexc_le( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) <= b;
}

Object *hexc_ne( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) != b;
}

Object *hexc_eq( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) == b;
}

Object *hexc_land( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) && b;
}

Object *hexc_lor( h_context_t *ctx, vmem_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = htree_execute( ctx, frame, node->child(0));
    b = htree_execute( ctx, frame, node->child(1));

    return (*a) || b;
}

