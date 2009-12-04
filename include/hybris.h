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
#ifndef _HHYBRIS_H_
#   define _HHYBRIS_H_

#include "object.h"
#include "vmem.h"
#include "tree.h"
#include "common.h"
#include "builtin.h"

#define HMAGIC "XCS\x10\x12\x19\x85"

extern vector<pthread_t> h_thread_pool;
extern pthread_mutex_t   h_thread_pool_mutex;


#define POOL_DEL(tid) pthread_mutex_lock( &h_thread_pool_mutex ); \
                        for( int pool_i = 0; pool_i < h_thread_pool.size(); pool_i++ ){ \
                            if( h_thread_pool[pool_i] == tid ){ \
                                h_thread_pool.erase( h_thread_pool.begin() + pool_i ); \
                                break; \
                            } \
                        } \
                      pthread_mutex_unlock( &h_thread_pool_mutex )

extern vector<string>   HSTACKTRACE;
extern vmem_t           HVM;
#ifdef GC_SUPPORT
extern vgarbage_t       HVG;
#endif
extern vcode_t          HVC;
extern hybris_globals_t HGLOBALS;


Object *htree_function_call( vmem_t *stackframe, Node *call, int threaded = 0 );
Object *htree_execute( vmem_t *stackframe, Node *node );

/*
void    htree_compile( Node *node, FILE *output );
Node   *htree_load( FILE *input );
*/

int  h_file_exists( char *filename );
void h_env_init( int argc, char *argv[] );
int  h_changepath();
void h_env_release( int onerror = 0 );

#endif

