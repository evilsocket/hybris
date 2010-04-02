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

VirtualMemory::VirtualMemory() : Map<Object>() {
    #ifdef MEM_DEBUG
    printf( "[MEM DEBUG] !!! Virtual memory table initialized .\n" );
    #endif
}


VirtualMemory::~VirtualMemory(){
    // release();
}

Object *VirtualMemory::get( char *identifier ){
    return find(identifier);
}

Object *VirtualMemory::add( char *identifier, Object *object ){
    Object *o   = H_UNDEFINED,
           *old = H_UNDEFINED;

    /* if object does not exist yet, create a new one */
    if( (old = get( identifier )) == H_UNDEFINED ){
        if( object != H_UNDEFINED ){
            o = new Object(object);
            #ifdef GC_SUPPORT
                o->setGarbageAttribute( ~H_OA_GARBAGE );
            #endif
            insert( identifier, o );
        }
        else{
            insert( identifier, H_UNDEFINED );
        }
        return o;
    }
    /* else set the new value */
    else{
        o = new Object(object);
        #ifdef GC_SUPPORT
            o->setGarbageAttribute( ~H_OA_GARBAGE );
        #endif
        replace( identifier, old, o );

        old->release();

        return o;
    }
}

VirtualMemory *VirtualMemory::clone(){
    unsigned int i;

    VirtualMemory *clone = new VirtualMemory;

    for( i = 0; i < m_elements; ++i ){
        clone->insert( (char *)label(i), new Object( at(i) ) );
    }

    return clone;
}

void VirtualMemory::release(){
    unsigned int i;
    Object      *o;

    for( i = 0; i < m_elements; ++i ){
        o = at(i);
        if( o != H_UNDEFINED && o->size ){
            #ifdef MEM_DEBUG
                printf( "[MEM DEBUG] !!! releasing '%s' value at 0x%X (- %d bytes)\n", Object::type_name(o), o,  o->size );
            #endif
            delete o;
        }
    }

    clear();
}
