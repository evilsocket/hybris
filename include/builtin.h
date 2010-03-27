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
#ifndef _HBUILTIN_H_
#   define _HBUILTIN_H_

#include <vector>
#include <string>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include "object.h"
#include "vmem.h"
#include "executor.h"

using std::string;
using std::vector;

typedef vector<string> vstr_t;

/* pre declaration of struct _h_context */
struct _h_context;
/* pre declaration of class Executor */
class Executor;

/* builtin function pointer prototype */
typedef Object * (*function_t)( struct _h_context *, vmem_t * );

/* helper macro to declare a builtin function */
#define HYBRIS_BUILTIN(name) Object *name( h_context_t *ctx, vmem_t *data )
/* helper macro to define a builtin function */
#define HYBRIS_DEFINE_BUILTIN( ctx, name, func ) ctx->builtins.insert( name, new builtin_t( name, func ) )
/* helper macro to define a builtin constant */
#define HYBRIS_DEFINE_CONSTANT( ctx, name, value ) ctx->constants.push_back( new builtin_constant_t( name, new Object( value  ) ) )

/* builtins definition list item structure */
typedef struct _builtin {
    string     identifier;
    function_t function;

    _builtin( string _id, function_t _fun ){
        identifier = _id;
        function   = _fun;
    }
}
builtin_t;

/* builtins' constants definition structure */
typedef struct _builtin_constant {
    string identifier;
    Object *value;

    _builtin_constant( string _id, Object *_v ){
        identifier = _id;
        value      = _v;
    }
}
builtin_constant_t;

/* module structure definition */
typedef struct _module_t {
    string              name;
    void (*initializer)( struct _h_context * );
    vector<builtin_t *> functions;
}
module_t;

/* module initializer function pointer prototype */
typedef void (*initializer_t)( struct _h_context * );

typedef vector<builtin_constant_t *> h_constants_t;
typedef Map<builtin_t>               h_builtins_t;
typedef vector<module_t *>           h_modules_t;

/* hybris execution contest structure */
typedef struct _h_context {
    /* function call trace vector */
    vector<string> stack_trace;
    /* data segment */
    vmem_t         vmem;
    /* code segment */
    vcode_t        vcode;
    /* execution arguments */
    h_args_t       args;
    /* default constants */
    h_constants_t  constants;
    /* builtin functions */
    h_builtins_t   builtins;
    /* dynamically loaded modules */
    h_modules_t    modules;
    #ifdef MT_SUPPORT
    /* running threads pool vector */
    vector<pthread_t> th_pool;
    /* threads mutex */
    pthread_mutex_t   th_mutex;
    #endif

    Executor      *executor;
}
h_context_t;

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

Object *hrex_operator( Object *o, Object *regexp );
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
#ifndef _LP64
HYBRIS_BUILTIN(hcall);
#endif
/* dll.cc */
#ifndef _LP64
HYBRIS_BUILTIN(hdllopen);
HYBRIS_BUILTIN(hdlllink);
HYBRIS_BUILTIN(hdllcall);
HYBRIS_BUILTIN(hdllclose);
#endif
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

void       hmodule_load     ( h_context_t *ctx, char *module );
function_t hfunction_search ( h_context_t *ctx, char *identifier );

#endif
