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

/*
 * This structure holds the state of a memory frame.
 * Only the exception state is cloned up to higher frame
 * until someone catches it or the program ends.
 */
typedef struct _vframe_state {
	/*
	 * Set to true when a break statement is found or
	 * when a return statement is found inside a loop.
	 */
	bool 	_break;
	/*
	 * Set to true when a next statement is found.
	 */
	bool 	_next;
	/*
	 * Set to true when a return statement is found.
	 */
	bool 	_return;
	/*
	 * Set to true when an exception is thrown.
	 */
	bool    _exception;
	/*
	 * This will hold the exception or return data.
	 */
	Object *value;

	_vframe_state()
		: _break(false),
		  _next(false),
		  _return(false),
		  _exception(false),
		  value(NULL)
	{

	}

	__force_inline void assign( struct _vframe_state& s ){
		_break     = s._break;
		_next      = s._next;
		_return    = s._return;
		_exception = s._exception;
		value      = s.value;
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
		 * Virtual memory frame state.
		 */
		vframe_state_t state;

		MemorySegment();
        ~MemorySegment();

        __force_inline Object *get( char *identifier ){
        	return find(identifier);
        }

        Object *add( char *identifier, Object *object );

        __force_inline Object *addConstant( char *identifier, Object *object ){
        	Object *o = add(identifier,object);
        	o->attributes |= H_OA_CONSTANT;
        	return o;
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
