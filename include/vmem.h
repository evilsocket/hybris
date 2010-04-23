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
 * This class represent a memory segment where constants
 * and variables are defined.
 */
class VirtualMemory : public HashMap<Object> {
    public :
		/*
		 * Set to true when a break statement is found or
		 * when a return statement is found inside a loop.
		 */
		bool 	break_state;
		/*
		 * Set to true when a next statement is found.
		 */
		bool 	next_state;
		/*
		 * Set to true when a return statement is found.
		 */
		bool 	return_state;
		/*
		 * Set to true when an exception is thrown.
		 */
		bool    exception_state;
		/*
		 * If return_state == true, this will hold the value
		 * to return.
		 */
		Object *return_value;
		/*
		 * If exception_state == true, this will hold the exception
		 * data.
		 */
		Object *exception_value;

        VirtualMemory();
        ~VirtualMemory();

        __force_inline Object *get( char *identifier ){
        	return find(identifier);
        }

        Object *add( char *identifier, Object *object );

        __force_inline Object *addConstant( char *identifier, Object *object ){
        	Object *o = add(identifier,object);
        	o->attributes |= H_OA_CONSTANT;
        	return o;
        }

        VirtualMemory *clone();

        __force_inline void release(){
        	clear();
        }
};

/* post type definitions */
typedef VirtualMemory vmem_t;
typedef VirtualMemory vframe_t;

#endif
