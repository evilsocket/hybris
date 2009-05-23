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
	    delete mem->at(idx);
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

void hybris_vm_release( vmem_t *mem ){
    unsigned int i;
    for( i = 0; i < mem->size(); i++ ){
        delete mem->at(i);
    }

    mem->clear();
}

Node *hybris_vc_clone( Node *function ){
    Node *node = Tree::clone( function, node );
    return node;
}

Node *hybris_vc_add( vcode_t *code, Node *function ){
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
    Node *ptr = code->find(function->_function);

    if( ptr != vcode_t::null ){
        return code->set( function->_function, function );
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
