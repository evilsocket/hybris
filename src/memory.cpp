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
    Object *_new = H_UNDEFINED,
           *_old = H_UNDEFINED,
           *_ret = H_UNDEFINED;

    /*
     * First of all, create a clone of the object instance.
     */
    _new = ob_clone(object);

    pthread_mutex_lock( &mutex );
    /* if object does not exist yet, insert as a new one */
    if( (_old = get( identifier )) == H_UNDEFINED ){
    	_ret = insert( identifier, _new );
    }
    /* else set the new value */
    else{
		/*
		 * Overwriting a constant object, print a warning.
		 */
    	if( (_old->attributes & H_OA_CONSTANT) == H_OA_CONSTANT ){
			hyb_error( H_ET_WARNING, "Overwriting constant value '%s'", identifier );
		}
    	
    	/*
    	 * If the old value is just a reference to something else (otReference type),
    	 * don't replace it in memory, but assign a new value to the object it
    	 * references.
    	 */
    	if( ob_is_reference(_old) ){
    		_ret = ob_assign( _old, _new );
    	}
    	/*
    	 * Plain object, do a normal memory replacement and ob_free the old value.
    	 */
    	else{
			replace( identifier, _old, _new );

			ob_free(_old);

			_ret = _new;
    	}
    }
    pthread_mutex_unlock( &mutex );

    return _ret;
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
