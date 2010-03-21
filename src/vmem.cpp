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
    /* if object does not exist yet, create a new one */
    if( hybris_vm_get( mem, identifier ) == H_UNDEFINED ){
        Object *o = H_UNDEFINED;
        if( object != H_UNDEFINED ){
            o = new Object(object);
            o->setGarbageAttribute( ~H_OA_GARBAGE );
            mem->insert( identifier, o );
        }
        else{
            mem->insert( identifier, H_UNDEFINED );
        }
        return o;
    }
    /* else set the new value */
    else{
        return hybris_vm_set( mem, identifier, object );
    }

    return object;
}

Object *hybris_vm_set( vmem_t *mem, char *identifier, Object *object ){
    Object *new_object = H_UNDEFINED,
           *old_object = H_UNDEFINED;

	int idx = mem->quick_search(identifier);
	if( idx != -1 ){
	    /* release old value */
	    old_object = mem->at(idx);
	    old_object->release();
	    /* assign new value */
	    new_object = new Object(object);
	    new_object->setGarbageAttribute( ~H_OA_GARBAGE );

		return mem->set( (unsigned int)idx, new_object );
	}
	else{
		return hybris_vm_add( mem, identifier, object );
	}
}

Object *hybris_vm_get( vmem_t *mem, char *identifier ){
    Object *o = mem->find(identifier);
    return (o == vmem_t::null ? H_UNDEFINED : o);
}

vmem_t *hybris_vm_clone( vmem_t *mem ){
    unsigned int size = mem->size(),
                 i;

    vmem_t *clone = new vmem_t;

    for( i = 0; i < size; i++ ){
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
    for( i = 0; i < size; i++ ){
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
        return hybris_vc_set( code, function );
    }

    return function;
}

Node *hybris_vc_set( vcode_t *code, Node *function ){
    Node *ptr = code->find( (char *)function->_function.c_str() );
    if( ptr != vcode_t::null ){
        return code->set( (char *)function->_function.c_str(), function );
    }
    else{
        return H_UNDEFINED;
    }
}

Node *hybris_vc_get( vcode_t *code, char *function ){
    Node *tree = code->find(function);
    return (tree == vcode_t::null ? H_UNDEFINED : tree);
}

void hybris_vc_release( vcode_t *code ){
    unsigned int i;

    for( i = 0; i < code->size(); i++ ){
        Tree::release( code->at(i) );
    }
    code->clear();
}
