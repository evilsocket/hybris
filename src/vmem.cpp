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

VirtualMemory::VirtualMemory() : HashMap<Object>() {

}

VirtualMemory::~VirtualMemory(){
	/*
	 * See note on ~HashMap()
	 */
	// release();
}

Object *VirtualMemory::add( char *identifier, Object *object ){
    Object *o   = H_UNDEFINED,
           *old = H_UNDEFINED;

    /* if object does not exist yet, create a new one */
    if( (old = get( identifier )) == H_UNDEFINED ){
    	if( object != H_UNDEFINED ){
            o = ob_clone(object);

            ob_set_references( o, +1 );
        }
        return insert( identifier, o );
    }
    /* else set the new value */
    else{
        o = ob_clone(object);

        ob_set_references( o, +1 );

        replace( identifier, old, o );

        ob_free(old);

        return o;
    }
}

VirtualMemory *VirtualMemory::clone(){
    unsigned int i;

    VirtualMemory *clone = new VirtualMemory;

    for( i = 0; i < m_elements; ++i ){
        clone->add( (char *)label(i), at(i) );
    }

    return clone;
}
