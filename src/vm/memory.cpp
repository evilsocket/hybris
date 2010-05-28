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
#include "memory.h"
#include "common.h"

MemorySegment::MemorySegment() : ITree<Object>(), mutex(PTHREAD_MUTEX_INITIALIZER) {

}

MemorySegment::~MemorySegment(){
	vector<pair_t *>::iterator i;
	register size_t value;
	vv_foreach( vector<pair_t *>, i, m_map ){
		value = (*i)->value->ref;
		(*i)->value->ref = (value <= 0 ? value : --value);
	}
}

Object *MemorySegment::add( char *identifier, Object *object ){
    Object *next = H_UNDEFINED,
           *prev = H_UNDEFINED,
           *retn = H_UNDEFINED;

    /*
	* Create a clone of the object instance if it's already
	* referenced somewhere else (or it's a constant), otherwhise just use it as it is.
	*/
	if( object->ref != 0 || (object->attributes & H_OA_CONSTANT) == H_OA_CONSTANT ){
		next = ob_clone(object);
	}
	else{
		next = object;
	}

    pthread_mutex_lock( &mutex );

    /* if object does not exist yet, insert as a new one */
    if( (prev = get( identifier )) == H_UNDEFINED ){
    	retn = MemorySegment::insert( identifier, next );
    }
    /* else set the new value */
    else{
		/*
		 * If the old value is just a reference to something else (otReference type),
		 * don't replace it in memory, but assign a new value to the object it
		 * references.
		 */
		 if( ob_is_reference(prev) && ob_ref_ucast(prev)->value != NULL ){
			 retn = ob_assign( prev, next );
		 }
		 /*
		  * Plain object, do a normal memory replacement and ob_free the old value.
		  */
		 else{
			 next->ref++;
			 prev->ref--;

			 retn = replace( identifier, prev, next );
		 }
    }

    pthread_mutex_unlock( &mutex );

    return retn;
}


MemorySegment *MemorySegment::clone(){
    unsigned int i;

    MemorySegment *clone = new MemorySegment;

    for( i = 0; i < m_elements; ++i ){
        clone->add( (char *)label(i), at(i) );
    }

	clone->state.assign(state);

    return clone;
}
