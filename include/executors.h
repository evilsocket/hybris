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

Object *hexc_identifier    ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_function      ( h_context_t *ctx, vmem_t *frame, Node *node );

// operators
Object *hexc_dollar        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_pointer       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_object        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_return        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_range         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_subscript_add ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_subscript_get ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_subscript_set ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_while         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_do            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_for           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_foreach       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_foreachm      ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_if            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_question      ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_eostmt        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_dot           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_dote          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_assign        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_uminus        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_regex         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_plus          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_pluse         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_minus         ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_minuse        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_mul           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_mule          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_div           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_dive          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_mod           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_mode          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_inc           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_dec           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_xor           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_xore          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_and           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_ande          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_or            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_ore           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_shiftl        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_shiftle       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_shiftr        ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_shiftre       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_fact          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_not           ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_lnot          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_less          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_greater       ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_ge            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_le            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_ne            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_eq            ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_land          ( h_context_t *ctx, vmem_t *frame, Node *node );
Object *hexc_lor           ( h_context_t *ctx, vmem_t *frame, Node *node );

#endif

