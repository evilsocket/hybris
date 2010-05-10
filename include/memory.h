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
#ifndef _HVMEM_H_
#	define _HVMEM_H_

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

		/*
		 * Return an object instance if defined as 'identifier',
		 * otherwise return H_UNDEFINED (NULL).
		 */
        __force_inline Object *get( char *identifier ){
        	return find(identifier);
        }
        /*
         * Clone the object, define it as 'identifier' if it's not
         * defined yet, otherwise replace the old value with this one.
         *
         * NOTE 1 : If the old value is a reference type, it won't be replaced,
         * 		    instead the object it references will be replaced.
         *
         * NOTE 2 : If the old value is marked as constant, a warning will
         * 			be printed to let the user know he's overwrinting a
         * 			constant value.
         */
        Object *add( char *identifier, Object *object );
        /*
         * Unlikely ::add, this method will not clone the object, but just
         * define it and mark it as a constant value.
         */
        __force_inline Object *addConstant( char *identifier, Object *object ){
        	/*
        	 * Insert the object.
        	 */
        	Object *o = insert( identifier, object );
			/*
			 * Make sure it's marked as constant.
			 */
        	o->attributes |= H_OA_CONSTANT;
        	/*
        	 * Return the instance.
        	 */
        	return o;
        }
        /*
         * This method will push 'value' onto the stack, with an anonymous identifier.
         *
         * NOTE 1 : The object will not be cloned because this method is used with
         * 			builtin functions, so we do not care about reference overwriting
         * 			or issues like that.
         */
        __force_inline Object *push( Object *value ){
        	char label[0xFF] = {0};
        	sprintf( label, "HANONYMOUSIDENTIFIER%d", m_elements );
        	return insert( label, value );
        }
        /*
         * Create a clone of this memory segment.
         */
        MemorySegment *clone();
        /*
         * Release the structure that holds this memory segment.
         * Inner objects will not be freed until the next gc_collect
         * is called.
         */
        __force_inline void release(){
        	clear();
        }
};

/* post type definitions */
typedef MemorySegment vmem_t;
typedef MemorySegment vframe_t;

#endif
