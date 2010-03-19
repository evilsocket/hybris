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

#include "builtin.h"
#include "object.h"
#include "vmem.h"
#include "tree.h"
#include "common.h"

#define POOL_DEL(tid) pthread_mutex_lock( &ctx->th_mutex ); \
                        for( int pool_i = 0; pool_i < ctx->th_pool.size(); pool_i++ ){ \
                            if( ctx->th_pool[pool_i] == tid ){ \
                                ctx->th_pool.erase( ctx->th_pool.begin() + pool_i ); \
                                break; \
                            } \
                        } \
                      pthread_mutex_unlock( &ctx->th_mutex )

void    h_env_init    ( h_context_t *ctx, int argc, char *argv[] );
void    h_env_release ( h_context_t *ctx, int onerror = 0 );

string  hbuild_function_trace ( char *function, vmem_t *stack, int identifiers );
Node   *hresolve_call         ( h_context_t *ctx, vmem_t *stackframe, Node *call, char *name );
Object *htree_function_call   ( h_context_t *ctx, vmem_t *stackframe, Node *call, int threaded = 0 );
Object *htree_execute         ( h_context_t *ctx, vmem_t *stackframe, Node *node );

int     h_file_exists( char *filename );
int     h_changepath( h_context_t *ctx );

#endif

