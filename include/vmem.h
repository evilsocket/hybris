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
#include "tree.h"
#include "object.h"
#include "map.h"

#define H_ADDRESS_OF(o)      reinterpret_cast<unsigned long>(o)

#define H_UNDEFINED          NULL
#define HANONYMOUSIDENTIFIER (char *)"HANONYMOUSIDENTIFIER"

/* data segment descriptor <identifier, object> */
typedef Map<Object>      vmem_t;
/* code segment descriptor <function, abstract tree> */
typedef Map<Node>        vcode_t;

Object *hybris_vm_add( vmem_t *mem, char *identifier, Object *object );
Object *hybris_vm_set( vmem_t *mem, char *identifier, Object *object );
Object *hybris_vm_get( vmem_t *mem, char *identifier );
vmem_t *hybris_vm_clone( vmem_t *mem );
void    hybris_vm_release( vmem_t *mem );

int     hybris_vg_isgarbage( vmem_t *mem, Object **o );
int     hybris_vg_isgarbage( vmem_t *frame, vmem_t *mem, Object **o );

Node   *hybris_vc_add( vcode_t *code, Node *function );
Node   *hybris_vc_set( vcode_t *code, Node *function );
Node   *hybris_vc_get( vcode_t *code, char  *function );
void    hybris_vc_release( vcode_t *code );

#endif
