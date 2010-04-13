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
#include "common.h"
#include "gc.h"

extern void hyb_throw( H_ERROR_TYPE type, const char *format, ... );

gc_t __gc;

void gc_free( gc_item_t *item,  gc_pool_t::iterator &index ){
    #ifdef MEM_DEBUG
        printf( "[MEM DEBUG] Releasing %p [%s] [%d bytes] [%d references].\n", item->pobj, item->pobj->type->name, item->pobj->ref );
    #endif

    __gc.items--;
    __gc.usage -= item->size;

    delete item->pobj;
    delete item;

    /**
     * NOTE: To remove the item from the pool, we should use :
     *
     *      __gc.pool.erase(index)
     *
     * but the std::vector::erase method is way much slower than this, therefore
     * let's put an "empty block" in the pool and upon the next iteration we will
     * simply skip it.
     */
    (*index) = NULL;
}

struct _Object *gc_track( struct _Object *o, size_t size ){
    if( o == NULL ){
        /*
         * We assume that 'o' was previously allocated with one of the MK_..._OBJ
         * macros, therefore, if its pointer is null, most of it there was a memory
         * allocation problem.
         */
        hyb_throw( H_ET_GENERIC, "out of memory" );
    }

    __gc.items++;
    __gc.usage += size;
    __gc.pool.push_back( new gc_item_t(o,size) );

    return o;
}

void gc_collect(){
    /**
     * Execute garbage collection loop only if used memory has reaced the
     * threshold.
     */
    if( __gc.usage >= __gc.threshold ){
        gc_pool_t::iterator i;
        for( i = __gc.pool.begin(); i != __gc.pool.end(); i++ ){
            /*
             * Skip empty blocks.
             */
            gc_item_t *item = (*i);
            if( item != NULL ){
                /*
                 * Skip constant objects because they are part of the execution tree nodes.
                 */
                struct _Object *o = item->pobj;
                if( (o->attributes & H_OA_CONSTANT) != H_OA_CONSTANT ){
                    /*
                     * Skip objects that are still referenced somewhere.
                     */
                    if( o->ref <= 0 ){
                        /**
                         * Finally execute garbage collection
                         */
                        gc_free( item, i );
                    }
                    #ifdef MEM_DEBUG
                    else{
                        printf( "[MEM DEBUG] Skipping %p [%s] [%d bytes] [%d references].\n", item->pobj, item->pobj->type->name, item->pobj->ref );
                    }
                    #endif
                }
                #ifdef MEM_DEBUG
                else{
                    printf( "[MEM DEBUG] Skipping constant %p [%s] [%d bytes] [%d references].\n", item->pobj, item->pobj->type->name, item->pobj->ref );
                }
                #endif
            }
        }
    }
}

void gc_release(){
    gc_pool_t::iterator i;
    for( i = __gc.pool.begin(); i != __gc.pool.end(); i++ ){
        if( (*i) != NULL  ){
            gc_free( *i, i );
        }
    }
}
