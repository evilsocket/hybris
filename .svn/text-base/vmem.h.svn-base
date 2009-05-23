#ifndef _HVMEM_H_
#	define _HVMEM_H_

#include <stdlib.h>
#include <string.h>
#include "object.hpp"
#include "tree.hpp"
#include "map.hpp"

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
