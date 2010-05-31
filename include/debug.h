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
#ifndef _HDEBUG_H_
#   define _HDEBUG_H_

#include "common.h"

typedef struct bpoint {
	char   source[0xFF];
	char   line[0xFF];
	size_t lineno;
}
bpoint_t;

typedef struct {
	vm_t   *vm;
	llist_t bpoints;
}
dbg_t;

void dbg_init( dbg_t *dbg, vm_t *vm );
void dbg_add_bpoint( dbg_t *dbg, char *filename, size_t lineno );
void dbg_trigger( dbg_t *dbg, vframe_t *frame, Node *node );

#endif
