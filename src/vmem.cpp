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
#include "vmem.h"

Object *hybris_vm_add( vmem_t *mem, char *identifier, Object *object ){
    Object *o   = H_UNDEFINED,
           *old = H_UNDEFINED;

    /* if object does not exist yet, create a new one */
    if( (old = hybris_vm_get( mem, identifier )) == H_UNDEFINED ){
        if( object != H_UNDEFINED ){
            o = new Object(object);
            #ifdef GC_SUPPORT
                o->setGarbageAttribute( ~H_OA_GARBAGE );
            #endif
            mem->insert( identifier, o );
        }
        else{
            mem->insert( identifier, H_UNDEFINED );
        }
        return o;
    }
    /* else set the new value */
    else{
        o = new Object(object);
        #ifdef GC_SUPPORT
            o->setGarbageAttribute( ~H_OA_GARBAGE );
        #endif
        mem->replace( identifier, old, o );

        old->release();

        return o;
    }
}

Object *hybris_vm_get( vmem_t *mem, char *identifier ){
    return mem->find(identifier);
}

vmem_t *hybris_vm_clone( vmem_t *mem ){
    unsigned int size = mem->size(),
                 i;

    vmem_t *clone = new vmem_t;

    for( i = 0; i < size; ++i ){
        Object *o = mem->at(i);
        clone->insert( (char *)mem->label(i), new Object(o) );
    }

    return clone;
}

void hybris_vm_release( vmem_t *mem ){
    unsigned int size,
                 i;
    Object      *o;

    size = mem->size();
    for( i = 0; i < size; ++i ){
        o = mem->at(i);
        if( o != H_UNDEFINED && o->xsize ){
            #ifdef MEM_DEBUG
            printf( "[MEM DEBUG] !!! releasing '%s' value at 0x%X (- %d bytes)\n", Object::type(o), o,  o->xsize );
            #endif
            delete o;
        }
    }

    mem->clear();
}

Node *hybris_vc_clone( Node *function ){
    Node *node = Tree::clone( function, node );
    return node;
}

Node *hybris_vc_add( vcode_t *code, Node *function ){
    /* if object does not exist yet, create a new one */
    if( hybris_vc_get( code, (char *)function->_function.c_str() ) == H_UNDEFINED ){
        return code->insert( (char *)function->_function.c_str(), hybris_vc_clone(function) );
    }
    /* else set the new value */
    else{
        return code->set( (char *)function->_function.c_str(), function );
    }

    return function;
}

Node *hybris_vc_get( vcode_t *code, char *function ){
    return code->find(function);
}

void hybris_vc_release( vcode_t *code ){
    unsigned int i;

    for( i = 0; i < code->size(); ++i ){
        Tree::release( code->at(i) );
    }
    code->clear();
}
