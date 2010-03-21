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
#ifndef _HEXECUTORS_H_
#   define _HEXECUTORS_H_

#include "hybris.h"

#ifdef GC_SUPPORT
/* determine whenever an object is garbage or a constant */
#   define H_IS_GARBAGE(o)  ((o->attributes & H_OA_GARBAGE)  == H_OA_GARBAGE)
#   define H_IS_CONSTANT(o) ((o->attributes & H_OA_CONSTANT) == H_OA_CONSTANT)
/* deallocate garbage and non constant objects */
#   define H_FREE_GARBAGE(o) if( o != H_UNDEFINED && H_IS_GARBAGE(o) && !H_IS_CONSTANT(o) ){ delete o; o = H_UNDEFINED; }
#else
#   define H_FREE_GARBAGE(o) // o
#endif

Object *exec_identifier    ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_function      ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_dollar        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_pointer       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_object        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_return        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_range         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_subscript_add ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_subscript_get ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_subscript_set ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_while         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_do            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_for           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_foreach       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_foreachm      ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_if            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_question      ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_eostmt        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_dot           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_dote          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_assign        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_uminus        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_regex         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_plus          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_pluse         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_minus         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_minuse        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_mul           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_mule          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_div           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_dive          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_mod           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_mode          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_inc           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_dec           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_xor           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_xore          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_and           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_ande          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_or            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_ore           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_shiftl        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_shiftle       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_shiftr        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_shiftre       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_fact          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_not           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_lnot          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_less          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_greater       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_ge            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_le            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_ne            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_eq            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_land          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *exec_lor           ( h_context_t *ctx, vmem_t *frame, Node *node );

#endif

