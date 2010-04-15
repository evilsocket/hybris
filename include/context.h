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
#ifndef _HCONTEXT_H_
#   define _HCONTEXT_H_

#include <vector>
#include <string>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include "types.h"
#include "vmem.h"
#include "vcode.h"
#include "engine.h"

using std::string;
using std::vector;

typedef vector<string> vstr_t;

/* pre declaration of class Context */
class Context;
/* pre declaration of class Engine */
class Engine;

/* initializer function pointer prototype */
typedef void     (*initializer_t)( Context * );
/* function pointer prototype */
typedef Object * (*function_t)( Context *, vmem_t * );

/* macro to declare a hybris function */
#define HYBRIS_DEFINE_FUNCTION(name) Object *name( Context *ctx, vmem_t *data )
/* macro to define a constant value */
#define HYBRIS_DEFINE_CONSTANT( ctx, name, value ) ctx->vmem.add( (char *)name, (Object *)value )
/* macro to define a new structure type given its name and its attribute names */
#define HYBRIS_DEFINE_STRUCTURE( ctx, name, n, attrs ) ctx->defineType( name, n, attrs )

/* macro to easily access hybris functions parameters */
#define ob_argv(i)    (data->at(i))
/* macro to easily access hybris functions parameters number */
#define ob_argc()     (data->size())
/* typed versions of ob_argv macro */
#define int_argv(i)    ob_int_val( data->at(i) )
#define alias_argv(i)  ob_alias_val( data->at(i) )
#define extern_argv(i) ob_extern_val( data->at(i) )
#define float_argv(i)  ob_float_val( data->at(i) )
#define char_argv(i)   ob_char_val( data->at(i) )
#define string_argv(i) ob_string_val( data->at(i) )
#define binary_argv(i) ob_binary_val( data->at(i) )
#define vector_argv(i) ob_vector_val( data->at(i) )
#define map_argv(i)    ob_map_val( data->at(i) )
#define matrix_argv(i) ob_matrix_val( data->at(i) )
#define struct_argv(i) ob_struct_val( data->at(i) )

/* macros to assert an object type */
#define ob_type_assert(o,t)      if( !(o->type->code == t) ){ \
                                     hyb_throw( H_ET_SYNTAX, "'%s' is not a valid variable type", o->type->name ); \
                                  }
#define ob_types_assert(o,t1,t2) if( !(o->type->code == t1) && !(o->type->code == t2) ){ \
                                     hyb_throw( H_ET_SYNTAX, "'%s' is not a valid variable type", o->type->name ); \
                                  }

#define HYB_TIMER_START 1
#define HYB_TIMER_STOP  0

typedef struct _named_function_t {
    string     identifier;
    function_t function;

    _named_function_t( string i, function_t f ) :
        identifier(i),
        function(f){

    }
}
named_function_t;

typedef vector<named_function_t *> named_functions_t;

/* module structure definition */
typedef struct _module_t {
    vstr_t            tree;
    string            name;
    initializer_t     initializer;
    named_functions_t functions;
}
module_t;

/* module initializer function pointer prototype */
typedef void (*initializer_t)( Context * );

typedef vector<module_t *>           h_modules_t;

class Context {
    private :

        static void signal_handler( int signo );

        string mk_trace( char *function, vframe_t *frame );

        void   str_split( string& str, string delimiters, vector<string>& tokens );

        void   loadModule( string path, string name );
        void   loadNamespace( string path );

    public  :

        #ifdef MT_SUPPORT
            /* running threads pool vector */
            vector<pthread_t> th_pool;
            /* threads mutex */
            pthread_mutex_t   th_mutex;
            /* lock the pthread mutex */
            __force_inline void lock(){
                pthread_mutex_lock( &th_mutex );
            }
            /* release the pthread mutex */
            __force_inline void unlock(){
                pthread_mutex_unlock( &th_mutex );
            }
            /* add a thread to the threads pool */
            __force_inline void pool( pthread_t tid = 0 ){
                tid = (tid == 0 ? pthread_self() : tid);
                lock();
                th_pool.push_back(tid);
                unlock();
            }
            /* remove a thread from the threads pool */
            __force_inline void depool( pthread_t tid = 0 ){
                tid = (tid == 0 ? pthread_self() : tid);

                lock();
                for( int pool_i = 0; pool_i < th_pool.size(); ++pool_i ){
                    if( th_pool[pool_i] == tid ){
                        th_pool.erase( th_pool.begin() + pool_i );
                        break;
                    }
                }
                unlock();
            }
        #else
            __force_inline void lock(){}
            __force_inline void unlock(){}
            __force_inline void pool( pthread_t tid = 0 ){}
            __force_inline void depool( pthread_t tid = 0 ){}
        #endif
        /* source file handle */
        FILE          *fp;
        /* function call trace vector */
        vector<string> stack_trace;
        /* data segment */
        vmem_t         vmem;
        /* code segment */
        vcode_t        vcode;
        /* type definitions segment (structs and classes) */
        vmem_t         vtypes;
        /* execution arguments */
        h_args_t       args;
        /* functions lookup hashtable cache */
        HashMap<named_function_t> modules_cache;
        /* dynamically loaded modules */
        h_modules_t    modules;
        /* code execution engine */
        Engine        *engine;

        Context();

        FILE *openFile();
        void closeFile();

        int chdir();

        __force_inline void trace( char *function, vframe_t *frame ){
            lock();
                stack_trace.push_back( mk_trace( function, frame  ) );
            unlock();
        }

        __force_inline void detrace(){
            lock();
                stack_trace.pop_back();
            unlock();
        }

        __force_inline void timer( int start = 0 ){
            if( args.tm_timer ){
                /* first call */
                if( args.tm_start == 0 && start ){
                    args.tm_start = hyb_uticks();
                }
                /* last call */
                else if( !start ){
                    args.tm_end = hyb_uticks();
                    char buffer[0xFF] = {0};
                    hyb_timediff( args.tm_end - args.tm_start, buffer );
                    printf( "\033[01;33m[TIME] Elapsed %s .\n\033[00m", buffer );
                }
            }
        }

        void init( int argc, char *argv[] );
        void release();
        void loadModule( char *module );
        function_t getFunction( char *identifier );

        __force_inline Object * defineType( char *name, int nattrs, char *attributes[] ){
            StructureObject *type = gc_new_struct();
            unsigned int     i;

            for( i = 0; i < nattrs; ++i ){
                ob_add_attribute( (Object *)type, attributes[i] );
            }

           /*
            * Prevent the structure definition from being deleted by the gc.
            */
            type->attributes |= H_OA_CONSTANT;

            return vtypes.insert( name, (Object *)type );
        }

        __force_inline void defineType( char *name, Object *type ){
           /*
            * Prevent the structure definition from being deleted by the gc.
            */
            type->attributes |= H_OA_CONSTANT;

            vtypes.insert( name, type );
        }

        __force_inline Object * getType( char *name ){
            return vtypes.find(name);
        }
};

#endif
