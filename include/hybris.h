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
#ifndef _HHYBRIS_H_
#   define _HHYBRIS_H_

#include "vm.h"
#include "memory.h"
#include "code.h"
#include "node.h"
#include "common.h"

/*
 * Default ok and error return values.
 *
 * Declared inside the object to optimize allocations
 * of the same object.
 */
extern IntegerObject __default_return_value;
extern IntegerObject __default_error_value;

#define H_DEFAULT_RETURN (Object *)&__default_return_value
#define H_DEFAULT_ERROR  (Object *)&__default_error_value

#endif

