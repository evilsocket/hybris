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
#include "common.h"

VirtualMemory::VirtualMemory() :
	HashMap<Object>(),

	break_state(false),
	next_state(false),
	return_state(false),
	exception_state(false),
	return_value(H_UNDEFINED),
	exception_value(H_UNDEFINED) {

}

VirtualMemory::~VirtualMemory(){
	/*
	 * See note on ~HashMap()
	 */
	/*
	 * Handle non managed exceptions.
	 */
	if( exception_state == true ){
		exception_state = false;
		assert( exception_value != NULL );
		if( exception_value->type->svalue ){
			hyb_throw( H_ET_GENERIC, "Unhandled exception : %s", ob_svalue(exception_value).c_str() );
		}
		else{
			hyb_throw( H_ET_GENERIC, "Unhandled '%s' exception", ob_typename(exception_value) );
		}
	}
}

Object *VirtualMemory::add( char *identifier, Object *object ){
    Object *_new = H_UNDEFINED,
           *_old = H_UNDEFINED;

    if( object != H_UNDEFINED ){
    	/*
    	 * Only constants and referenced objects shall be cloned.
    	 */
    	if( (object->attributes & H_OA_CONSTANT) == H_OA_CONSTANT || object->ref > 0 ){
    		_new = ob_clone(object);
    	}
    	/*
    	 * Non constant and no references.
    	 */
    	else{
    		_new = object;
    	}

    	ob_set_references( _new, +1 );
    }

    /* if object does not exist yet, insert as a new one */
    if( (_old = get( identifier )) == H_UNDEFINED ){
        return insert( identifier, _new );
    }
    /* else set the new value */
    else{
        replace( identifier, _old, _new );

        ob_free(_old);

        return _new;
    }
}

VirtualMemory *VirtualMemory::clone(){
    unsigned int i;

    VirtualMemory *clone = new VirtualMemory;

    for( i = 0; i < m_elements; ++i ){
        clone->add( (char *)label(i), at(i) );
    }
	clone->break_state     = break_state;
	clone->next_state      = next_state;
	clone->return_state    = return_state;
	clone->exception_state = exception_state;
	clone->return_value    = return_value;
	clone->exception_value = exception_value;

    return clone;
}
