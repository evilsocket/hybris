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

MemorySegment::MemorySegment() : HashMap<Object>(), mutex(PTHREAD_MUTEX_INITIALIZER) {

}

Object *MemorySegment::add( char *identifier, Object *object ){
    Object *next = H_UNDEFINED,
           *prev = H_UNDEFINED;

    /*
     * First of all, create a clone of the object instance.
     */
    next = ob_clone(object);

    pthread_mutex_lock( &mutex );

    prev = insert( identifier, next );
    /*
     * An object with that key was already there ?
     */
    if( prev != next ){
    	/*
    	 * If the old value is just a reference to something else (otReference type),
    	 * assign a new value to the object it refers to.
    	 */
    	if( ob_is_reference(prev) ){
			ob_assign( prev, next );
			/*
			 * TODO: THIS IS NASTY! We should find a better way to do this!
			 *
			 *  Replace it back.
			 */
			insert( identifier, prev );

			return prev;
    	}
    	/*
    	 * Plain object, ob_free the old value.
    	 */
    	else{
    		ob_free(prev);
    	}
    }

    pthread_mutex_unlock( &mutex );

    return next;
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
