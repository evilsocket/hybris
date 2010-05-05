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
#ifndef _Hvm_tEM_H_
#	define _Hvm_tEM_H_

#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#include "types.h"

/* helper macro to obtain the address of a pointer */
#define H_ADDRESS_OF(o)      reinterpret_cast<ulong>(o)
/* default null value for an Object pointer */
#define H_UNDEFINED          NULL


enum state_t {
	None      = 0, // 00000000
	Break     = 1, // 00000001
	Next      = 2, // 00000010
	Return    = 4, // 00000100
	Exception = 8  // 00001000
};

/*
 * This structure holds the state of a memory frame.
 * Only the exception state is cloned up to higher frame
 * until someone catches it or the program ends.
 */
typedef struct _vframe_state {
	/*
	 * The state bitmask.
	 */
	unsigned long mask;
	/*
	 * This will hold the exception or return data.
	 */
	Object *value;

	_vframe_state() : mask(None), value(NULL) {

	}

	__force_inline void set( state_t s ){
		mask |= s;
	}

	__force_inline void set( state_t s, Object *v ){
		mask |= s;
		value = v;
	}

	__force_inline void unset( state_t s ){
		mask &= ~s;
	}

	__force_inline bool is( state_t s ){
		return (mask & s) == s;
	}

	__force_inline void assign( struct _vframe_state& s ){
		mask  = s.mask;
		value = s.value;
	}

	__force_inline void reset(){
		mask  = None;
		value = NULL;
	}
}
vframe_state_t;

/*
 * This class represent a memory segment where constants
 * and variables are defined.
 */
class MemorySegment : public HashMap<Object> {
    public :
		/*
		 * Name of the function/method that owns this stack.
		 */
		string			owner;
		/*
		 * Virtual memory frame state.
		 */
		vframe_state_t  state;
		/*
		 * Mutex for thread shared segments.
		 */
		pthread_mutex_t mutex;

		MemorySegment();

        __force_inline Object *get( char *identifier ){
        	return find(identifier);
        }

        Object *add( char *identifier, Object *object );

        __force_inline Object *addConstant( char *identifier, Object *object ){
        	Object *o = add(identifier,object);
        	o->attributes |= H_OA_CONSTANT;
        	return o;
        }


        __force_inline Object *push( Object *value ){
        	char label[0xFF] = {0};
        	sprintf( label, "HANONYMOUSIDENTIFIER%d", m_elements );
        	return insert( label, value );
        }

        MemorySegment *clone();

        __force_inline void release(){
        	clear();
        }
};

/* post type definitions */
typedef MemorySegment vmem_t;
typedef MemorySegment vframe_t;

#endif
