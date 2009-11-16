/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _HVMEM_H_
#	define _HVMEM_H_

#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "object.h"
#include "map.h"

#define H_UNDEFINED          NULL
#define HANONYMOUSIDENTIFIER (char *)"HANONYMOUSIDENTIFIER"

/* data segment descriptor <identifier, object> */
typedef Map<Object> vmem_t;
/* code segment descriptor <function, abstract tree> */
typedef Map<Node>   vcode_t;

Object *hybris_vm_add( vmem_t *mem, char *identifier, Object *object );
Object *hybris_vm_set( vmem_t *mem, char *identifier, Object *object );
Object *hybris_vm_get( vmem_t *mem, char *identifier );
void    hybris_vm_release( vmem_t *mem );

Node   *hybris_vc_add( vcode_t *code, Node *function );
Node   *hybris_vc_set( vcode_t *code, Node *function );
Node   *hybris_vc_get( vcode_t *code, char  *function );
void    hybris_vc_release( vcode_t *code );

#endif
