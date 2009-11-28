/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "vmem.h"

Object *hybris_vm_add( vmem_t *mem, char *identifier, Object *object ){
    /* if object does not exist yet, create a new one */
    if( hybris_vm_get( mem, identifier ) == H_UNDEFINED ){
        Object *o = H_UNDEFINED;
        if( object != H_UNDEFINED ){
            o = new Object(object);
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
	int idx = mem->quick_search(identifier);
	if( idx != -1 ){
	    /* delete old value */
	    mem->at(idx)->release();
	    /* assign new value */
		return mem->set( (unsigned int)idx, new Object(object) );
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

#ifdef GC_SUPPORT
void hybris_vm_release( vmem_t *mem, vgarbage_t *garbage ){
    unsigned int size,
                 i;
    Object      *o;

    if( garbage != NULL ){
        size = garbage->size();
        for( i = 0; i < size; i++ ){
            o = garbage->at(i);
            if( o && o->xsize > 0 && hybris_vg_isgarbage( mem, o ) ){
                #ifdef MEM_DEBUG
                printf( "[MEM DEBUG] !!! releasing '%s' garbage at 0x%X (- %d bytes)\n", Object::type(o), o,  o->xsize );
                #endif
                delete o;
                /* update garbage size upon new item removal */
                size = garbage->size();
            }
        }
        garbage->clear();
    }

    size = mem->size();
    for( i = 0; i < size; i++ ){
        o = mem->at(i);
         #ifdef MEM_DEBUG
        printf( "[MEM DEBUG] !!! releasing '%s' value at 0x%X (- %d bytes)\n", Object::type(o), o,  o->xsize );
        #endif
        delete o;
    }

    mem->clear();
}
#else
void hybris_vm_release( vmem_t *mem ){
    unsigned int size,
                 i;
    Object      *o;

    size = mem->size();
    for( i = 0; i < size; i++ ){
        o = mem->at(i);
         #ifdef MEM_DEBUG
        printf( "[MEM DEBUG] !!! releasing '%s' value at 0x%X (- %d bytes)\n", Object::type(o), o,  o->xsize );
        #endif
        delete o;
    }

    mem->clear();
}
#endif

#ifdef GC_SUPPORT
int hybris_vg_isgarbage( vmem_t *mem, Object *o ){
    unsigned int size = mem->size(),
                 i;

    unsigned long ob_address = reinterpret_cast<unsigned long>(o),
                  vm_address;

    for( i = 0; i < size; i++ ){
        vm_address = reinterpret_cast<unsigned long>( mem->at(i) );
        if( ob_address == vm_address ){
            return 0;
        }
    }
    return 1;
}

void hybris_vg_add( vgarbage_t *garbage, Object *o ){
    garbage->push_back(o);
}

void hybris_vg_del( vgarbage_t *garbage, Object *o ){
    unsigned int size = garbage->size(),
                 i;

    unsigned long ob_address = reinterpret_cast<unsigned long>(o),
                  vm_address;

    for( i = 0; i < size; i++ ){
        vm_address = reinterpret_cast<unsigned long>( garbage->at(i) );
        if( ob_address == vm_address ){
            garbage->erase( garbage->begin() + i );
            return;
        }
    }
}
#endif

Node *hybris_vc_clone( Node *function ){
    Node *node = Tree::clone( function, node );
    return node;
}

Node *hybris_vc_add( vcode_t *code, Node *function ){
    printf( "vc_add[ 0x%X ]( %s )\n", code, function->_function );
    /* if object does not exist yet, create a new one */
    if( hybris_vc_get( code, function->_function ) == H_UNDEFINED ){
        return code->insert( function->_function, hybris_vc_clone(function) );
    }
    /* else set the new value */
    else{
        return hybris_vc_set( code, function );
    }

    return function;
}

Node *hybris_vc_set( vcode_t *code, Node *function ){
    printf( "vc_set[ 0x%X ]( %s )\n", code, function->_function );
    Node *ptr = code->find(function->_function);
    if( ptr != vcode_t::null ){
        return code->set( function->_function, function );
    }
    else{
        return H_UNDEFINED;
    }
}

Node *hybris_vc_get( vcode_t *code, char *function ){
    printf( "vc_get[ 0x%X ]( %s ) ", code, function );
    Node *tree = code->find(function);
    printf( "%s\n", (tree == vcode_t::null ? "NIENTE :(" : "TROVATA !") );
    return (tree == vcode_t::null ? H_UNDEFINED : tree);
}

void hybris_vc_release( vcode_t *code ){
    printf( "vc_release\n" );
    unsigned int i;

    for( i = 0; i < code->size(); i++ ){
        Tree::release( code->at(i) );
    }
    code->clear();
}
