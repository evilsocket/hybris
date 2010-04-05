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
#include "object.h"
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

/* builtin function pointer prototype */
typedef Object * (*function_t)( Context *, vmem_t * );

/* helper macro to declare a builtin function */
#define HYBRIS_BUILTIN(name) Object *name( Context *ctx, vmem_t *data )
/* helper macro to define a builtin function */
#define HYBRIS_DEFINE_BUILTIN( ctx, name, func ) ctx->builtins.insert( name, new builtin_t( name, func ) )
/* helper macro to define a builtin constant */
#define HYBRIS_DEFINE_CONSTANT( ctx, name, value ) ctx->constants.push_back( new builtin_constant_t( name, new Object( value  ) ) )

/* builtins definition list item structure */
typedef struct _builtin {
    string     identifier;
    function_t function;

    _builtin( string id, function_t f ) : identifier(id), function(f) {

    }
}
builtin_t;

/* builtins' constants definition structure */
typedef struct _builtin_constant {
    string identifier;
    Object *value;

    _builtin_constant( string id, Object *v ) : identifier(id), value(v) {

    }
}
builtin_constant_t;

/* module structure definition */
typedef struct _module_t {
    string              name;
    void (*initializer)( Context * );
    vector<builtin_t *> functions;
}
module_t;

/* module initializer function pointer prototype */
typedef void (*initializer_t)( Context * );

typedef vector<builtin_constant_t *> h_constants_t;
typedef Map<builtin_t>               h_builtins_t;
typedef vector<module_t *>           h_modules_t;

class Context {
    private :

        static void signal_handler( int signo );

        string mk_trace( char *function, vframe_t *frame );

    public  :

        #ifdef MT_SUPPORT
            /* running threads pool vector */
            vector<pthread_t> th_pool;
            /* threads mutex */
            pthread_mutex_t   th_mutex;
            /* lock the pthread mutex */
            inline void lock(){
                pthread_mutex_lock( &th_mutex );
            }
            /* release the pthread mutex */
            inline void unlock(){
                pthread_mutex_unlock( &th_mutex );
            }
            /* add a thread to the threads pool */
            inline void pool( pthread_t tid = 0 ){
                tid = (tid == 0 ? pthread_self() : tid);
                lock();
                th_pool.push_back(tid);
                unlock();
            }
            /* remove a thread from the threads pool */
            inline void depool( pthread_t tid = 0 ){
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
            inline void lock(){}
            inline void unlock(){}
            inline void pool( pthread_t tid = 0 ){}
            inline void depool( pthread_t tid = 0 ){}
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
        /* default constants */
        h_constants_t  constants;
        /* builtin functions */
        h_builtins_t   builtins;
        /* dynamically loaded modules */
        h_modules_t    modules;
        /* code execution engine */
        Engine        *engine;

        Context();

        FILE *openFile();
        void closeFile();

        int chdir();

        inline void trace( char *function, vframe_t *frame ){
            lock();
                stack_trace.push_back( mk_trace( function, frame  ) );
            unlock();
        }

        inline void detrace(){
            lock();
                stack_trace.pop_back();
            unlock();
        }

        inline void timer(){
            if( args.tm_timer ){
                /* first call */
                if( args.tm_start == 0 ){
                    args.tm_start = hyb_uticks();
                }
                /* last call */
                else{
                    args.tm_end = hyb_uticks();
                    printf( "\033[01;33m[TIME] Elapsed %s .\n\033[00m", hyb_timediff( args.tm_end - args.tm_start ) );
                }
            }
        }

        void init( int argc, char *argv[] );
        void release( int error = 0 );
        void load( char *module );
        function_t getFunction( char *identifier );
};

/* type.cc */
HYBRIS_BUILTIN(hisint);
HYBRIS_BUILTIN(hisfloat);
HYBRIS_BUILTIN(hischar);
HYBRIS_BUILTIN(hisstring);
HYBRIS_BUILTIN(hisarray);
HYBRIS_BUILTIN(hismap);
HYBRIS_BUILTIN(hisalias);
HYBRIS_BUILTIN(htypeof);
HYBRIS_BUILTIN(hsizeof);
HYBRIS_BUILTIN(htoint);
HYBRIS_BUILTIN(htostring);
HYBRIS_BUILTIN(htoxml);
HYBRIS_BUILTIN(hfromxml);
/* math.cc */
HYBRIS_BUILTIN(hacos);
HYBRIS_BUILTIN(hasin);
HYBRIS_BUILTIN(hatan);
HYBRIS_BUILTIN(hatan2);
HYBRIS_BUILTIN(hceil);
HYBRIS_BUILTIN(hcos);
HYBRIS_BUILTIN(hcosh);
HYBRIS_BUILTIN(hexp);
HYBRIS_BUILTIN(hfabs);
HYBRIS_BUILTIN(hfloor);
HYBRIS_BUILTIN(hfmod);
HYBRIS_BUILTIN(hlog);
HYBRIS_BUILTIN(hlog10);
HYBRIS_BUILTIN(hpow);
HYBRIS_BUILTIN(hsin);
HYBRIS_BUILTIN(hsinh);
HYBRIS_BUILTIN(hsqrt);
HYBRIS_BUILTIN(htan);
HYBRIS_BUILTIN(htanh);
/* binary.cc */
HYBRIS_BUILTIN(hbinary);
HYBRIS_BUILTIN(hpack);
/* array.cc */
HYBRIS_BUILTIN(harray);
HYBRIS_BUILTIN(helements);
HYBRIS_BUILTIN(hpop);
HYBRIS_BUILTIN(hremove);
HYBRIS_BUILTIN(hcontains);
/* map.cc */
HYBRIS_BUILTIN(hmap);
HYBRIS_BUILTIN(hmapelements);
HYBRIS_BUILTIN(hmappop);
HYBRIS_BUILTIN(hunmap);
HYBRIS_BUILTIN(hismapped);
HYBRIS_BUILTIN(hhaskey);
/* string.cc */
HYBRIS_BUILTIN(hstrlen);
HYBRIS_BUILTIN(hstrfind);
HYBRIS_BUILTIN(hsubstr);
HYBRIS_BUILTIN(hstrreplace);
HYBRIS_BUILTIN(hstrsplit);
/* matrix.cc */
HYBRIS_BUILTIN(hmatrix);
HYBRIS_BUILTIN(hcolumns);
HYBRIS_BUILTIN(hrows);

#ifdef PCRE_SUPPORT
/* pcre.cc */
HYBRIS_BUILTIN(hrex_match);
HYBRIS_BUILTIN(hrex_matches);
HYBRIS_BUILTIN(hrex_replace);
#endif
/* conio.cc */
HYBRIS_BUILTIN(hprint);
HYBRIS_BUILTIN(hprintln);
HYBRIS_BUILTIN(hinput);
/* process.cc */
HYBRIS_BUILTIN(hexec);
HYBRIS_BUILTIN(hfork);
HYBRIS_BUILTIN(hgetpid);
HYBRIS_BUILTIN(hwait);
HYBRIS_BUILTIN(hpopen);
HYBRIS_BUILTIN(hpclose);
HYBRIS_BUILTIN(hexit);
/* reflection.cc */
HYBRIS_BUILTIN(hvar_names);
HYBRIS_BUILTIN(hvar_values);
HYBRIS_BUILTIN(huser_functions);
HYBRIS_BUILTIN(hcore_functions);
HYBRIS_BUILTIN(hdyn_functions);
HYBRIS_BUILTIN(hcall);
/* dll.cc */
HYBRIS_BUILTIN(hdllopen);
HYBRIS_BUILTIN(hdlllink);
HYBRIS_BUILTIN(hdllcall);
HYBRIS_BUILTIN(hdllclose);
/* time.cc */
HYBRIS_BUILTIN(hticks);
HYBRIS_BUILTIN(husleep);
HYBRIS_BUILTIN(hsleep);
HYBRIS_BUILTIN(htime);
HYBRIS_BUILTIN(hstrtime);
HYBRIS_BUILTIN(hstrdate);
/* fileio.cc */
HYBRIS_BUILTIN(hfopen);
HYBRIS_BUILTIN(hfseek);
HYBRIS_BUILTIN(hftell);
HYBRIS_BUILTIN(hfsize);
HYBRIS_BUILTIN(hfread);
HYBRIS_BUILTIN(hfgets);
HYBRIS_BUILTIN(hfwrite);
HYBRIS_BUILTIN(hfclose);
HYBRIS_BUILTIN(hfile);
HYBRIS_BUILTIN(hreaddir);
/* netio.cc */
HYBRIS_BUILTIN(hsettimeout);
HYBRIS_BUILTIN(hconnect);
HYBRIS_BUILTIN(hserver);
HYBRIS_BUILTIN(haccept);
HYBRIS_BUILTIN(hrecv);
HYBRIS_BUILTIN(hsend);
HYBRIS_BUILTIN(hclose);

#ifdef HTTP_SUPPORT
/* http.cc */
HYBRIS_BUILTIN(hhttp);
HYBRIS_BUILTIN(hhttp_get);
HYBRIS_BUILTIN(hhttp_post);
#endif

#ifdef XML_SUPPORT
/* xml.cc */
HYBRIS_BUILTIN(hxml_load);
HYBRIS_BUILTIN(hxml_parse);
#endif

/* encode.cc */
HYBRIS_BUILTIN(hurlencode);
HYBRIS_BUILTIN(hurldecode);
HYBRIS_BUILTIN(hbase64encode);
HYBRIS_BUILTIN(hbase64decode);

#ifdef MT_SUPPORT
/* pthreads.cc */
HYBRIS_BUILTIN(hpthread_create);
HYBRIS_BUILTIN(hpthread_exit);
HYBRIS_BUILTIN(hpthread_join);
#endif

#endif
