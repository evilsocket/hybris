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
#include "context.h"

void Context::signal_handler( int signo ){
    if( signo == SIGSEGV ){
        extern Context __context;
        __context.args.stacktrace = 1;
        hybris_generic_error( "SIGSEGV Signal Catched" );
    }
}

string Context::mk_trace( char *function, vframe_t *frame ){
    string trace( function + string("(") );
    unsigned int i,
                 size( frame->size() ),
                 i_end( size - 1 );

    for( i = 0; i < size; ++i ){
        trace += " " + frame->at(i)->svalue() + (i < i_end ? "," : "");
    }
    trace += " )";

    return trace;
}

Context::Context(){
    memset( &args, 0x00, sizeof(h_args_t) );
}

int Context::chdir(){
	/* compute source path and chdir to it */
	char *ptr = strrchr( args.source, '/' );
	if( ptr != NULL ){
		unsigned int pos = ptr - args.source + 1;
		char path[0xFF]  = {0};
		strncpy( path, args.source, pos );
		return ::chdir(path);
	}
	return 0;
}

void Context::init( int argc, char *argv[] ){
    int i;
    char name[0xFF] = {0};


    /* create code engine */
    engine = new Engine( this );
    /* save interpreter directory */
    getcwd( args.rootpath, 0xFF );
    /* initialize pthread mutex */
    #ifdef MT_SUPPORT
    th_mutex = PTHREAD_MUTEX_INITIALIZER;
    #endif
    /* set signal handler */
    signal( SIGSEGV, Context::signal_handler );

    /* initialize command line arguments */
    Object *o = new Object( static_cast<long>(argc - 1) );
    vmem.add( (char *)"argc", o );
    delete o;
    for( i = 1; i < argc; ++i ){
        sprintf( name, "%d", i - 1 );
        o = new Object(argv[i]);
        vmem.add( name, o );
        delete o;
    }

    /* initialize builtins' constants */
    /* misc */
    HYBRIS_DEFINE_CONSTANT( this, "true",  static_cast<long>(1) );
    HYBRIS_DEFINE_CONSTANT( this, "false", static_cast<long>(0) );
    /* fileio.cc::fseek */
    HYBRIS_DEFINE_CONSTANT( this, "SEEK_SET", static_cast<long>(SEEK_SET) );
    HYBRIS_DEFINE_CONSTANT( this, "SEEK_CUR", static_cast<long>(SEEK_CUR) );
    HYBRIS_DEFINE_CONSTANT( this, "SEEK_END", static_cast<long>(SEEK_END) );
    /* fileio.cc::readdir */
    HYBRIS_DEFINE_CONSTANT( this, "DT_BLK",   static_cast<long>(DT_BLK)  );
    HYBRIS_DEFINE_CONSTANT( this, "DT_CHR",   static_cast<long>(DT_CHR)  );
    HYBRIS_DEFINE_CONSTANT( this, "DT_DIR",   static_cast<long>(DT_DIR)  );
    HYBRIS_DEFINE_CONSTANT( this, "DT_FIFO",  static_cast<long>(DT_FIFO) );
    HYBRIS_DEFINE_CONSTANT( this, "DT_LNK",   static_cast<long>(DT_LNK)  );
    HYBRIS_DEFINE_CONSTANT( this, "DT_REG",   static_cast<long>(DT_REG)  );
    HYBRIS_DEFINE_CONSTANT( this, "DT_SOCK",  static_cast<long>(DT_SOCK) );
    /* http.cc::http */
    HYBRIS_DEFINE_CONSTANT( this, "GET",  static_cast<long>(0) );
    HYBRIS_DEFINE_CONSTANT( this, "POST", static_cast<long>(1) );
    for( i = 0; i < constants.size(); ++i ){
        vmem.add( (char *)constants[i]->identifier.c_str(), constants[i]->value );
    }

    /* initialize builtin functions */
    HYBRIS_DEFINE_BUILTIN( this, "isint", hisint );
	HYBRIS_DEFINE_BUILTIN( this, "isfloat", hisfloat );
	HYBRIS_DEFINE_BUILTIN( this, "ischar", hischar );
	HYBRIS_DEFINE_BUILTIN( this, "isstring", hisstring );
	HYBRIS_DEFINE_BUILTIN( this, "isarray", hisarray );
	HYBRIS_DEFINE_BUILTIN( this, "ismap", hismap );
	HYBRIS_DEFINE_BUILTIN( this, "isalias", hisalias );
	HYBRIS_DEFINE_BUILTIN( this, "typeof", htypeof );
	HYBRIS_DEFINE_BUILTIN( this, "sizeof", hsizeof );
	HYBRIS_DEFINE_BUILTIN( this, "toint", htoint );
	HYBRIS_DEFINE_BUILTIN( this, "tostring", htostring );
	HYBRIS_DEFINE_BUILTIN( this, "toxml", htoxml );
	HYBRIS_DEFINE_BUILTIN( this, "fromxml", hfromxml );
	HYBRIS_DEFINE_BUILTIN( this, "acos", hacos );
    HYBRIS_DEFINE_BUILTIN( this, "asin", hasin );
    HYBRIS_DEFINE_BUILTIN( this, "atan", hatan );
    HYBRIS_DEFINE_BUILTIN( this, "atan2", hatan2 );
    HYBRIS_DEFINE_BUILTIN( this, "ceil", hceil );
    HYBRIS_DEFINE_BUILTIN( this, "cos", hcos );
    HYBRIS_DEFINE_BUILTIN( this, "cosh", hcosh );
    HYBRIS_DEFINE_BUILTIN( this, "exp", hexp );
    HYBRIS_DEFINE_BUILTIN( this, "fabs", hfabs );
    HYBRIS_DEFINE_BUILTIN( this, "floor", hfloor );
    HYBRIS_DEFINE_BUILTIN( this, "fmod", hfmod );
    HYBRIS_DEFINE_BUILTIN( this, "log", hlog );
    HYBRIS_DEFINE_BUILTIN( this, "log10", hlog10 );
    HYBRIS_DEFINE_BUILTIN( this, "pow", hpow );
    HYBRIS_DEFINE_BUILTIN( this, "sin", hsin );
    HYBRIS_DEFINE_BUILTIN( this, "sinh", hsinh );
    HYBRIS_DEFINE_BUILTIN( this, "sqrt", hsqrt );
    HYBRIS_DEFINE_BUILTIN( this, "tan", htan );
    HYBRIS_DEFINE_BUILTIN( this, "tanh", htanh );
	HYBRIS_DEFINE_BUILTIN( this, "array", harray );
	HYBRIS_DEFINE_BUILTIN( this, "elements", helements );
	HYBRIS_DEFINE_BUILTIN( this, "pop", hpop );
	HYBRIS_DEFINE_BUILTIN( this, "remove", hremove );
	HYBRIS_DEFINE_BUILTIN( this, "contains", hcontains );
	HYBRIS_DEFINE_BUILTIN( this, "struct", hmap );
	HYBRIS_DEFINE_BUILTIN( this, "map", hmap );
	HYBRIS_DEFINE_BUILTIN( this, "mapelements", hmapelements );
	HYBRIS_DEFINE_BUILTIN( this, "mappop", hmappop );
	HYBRIS_DEFINE_BUILTIN( this, "unmap", hunmap );
	HYBRIS_DEFINE_BUILTIN( this, "ismapped", hismapped );
	HYBRIS_DEFINE_BUILTIN( this, "haskey", hhaskey );
	HYBRIS_DEFINE_BUILTIN( this, "strlen", hstrlen );
	HYBRIS_DEFINE_BUILTIN( this, "strfind", hstrfind );
	HYBRIS_DEFINE_BUILTIN( this, "substr", hsubstr );
	HYBRIS_DEFINE_BUILTIN( this, "strreplace", hstrreplace );
	HYBRIS_DEFINE_BUILTIN( this, "strsplit", hstrsplit );
    HYBRIS_DEFINE_BUILTIN( this, "matrix",  hmatrix );
    HYBRIS_DEFINE_BUILTIN( this, "columns", hcolumns );
    HYBRIS_DEFINE_BUILTIN( this, "rows",    hrows );
	#ifdef PCRE_SUPPORT
	HYBRIS_DEFINE_BUILTIN( this, "rex_match", hrex_match );
	HYBRIS_DEFINE_BUILTIN( this, "rex_matches", hrex_matches );
	HYBRIS_DEFINE_BUILTIN( this, "rex_replace", hrex_replace );
	#endif
	HYBRIS_DEFINE_BUILTIN( this, "print", hprint );
	HYBRIS_DEFINE_BUILTIN( this, "println", hprintln );
	HYBRIS_DEFINE_BUILTIN( this, "input", hinput );
	HYBRIS_DEFINE_BUILTIN( this, "exec", hexec );
	HYBRIS_DEFINE_BUILTIN( this, "fork", hfork );
	HYBRIS_DEFINE_BUILTIN( this, "getpid", hgetpid );
	HYBRIS_DEFINE_BUILTIN( this, "wait", hwait );
	HYBRIS_DEFINE_BUILTIN( this, "popen", hpopen );
	HYBRIS_DEFINE_BUILTIN( this, "pclose", hpclose );
    HYBRIS_DEFINE_BUILTIN( this, "exit", hexit );
	HYBRIS_DEFINE_BUILTIN( this, "var_names", hvar_names );
	HYBRIS_DEFINE_BUILTIN( this, "var_values", hvar_values );
	HYBRIS_DEFINE_BUILTIN( this, "user_functions", huser_functions );
	HYBRIS_DEFINE_BUILTIN( this, "core_functions", hcore_functions );
	HYBRIS_DEFINE_BUILTIN( this, "dyn_functions",  hdyn_functions );
	HYBRIS_DEFINE_BUILTIN( this, "call", hcall );
	HYBRIS_DEFINE_BUILTIN( this, "dllopen", hdllopen );
    HYBRIS_DEFINE_BUILTIN( this, "dlllink", hdlllink );
    HYBRIS_DEFINE_BUILTIN( this, "dllcall", hdllcall );
    HYBRIS_DEFINE_BUILTIN( this, "dllclose", hdllclose );
	HYBRIS_DEFINE_BUILTIN( this, "ticks", hticks );
	HYBRIS_DEFINE_BUILTIN( this, "usleep", husleep );
	HYBRIS_DEFINE_BUILTIN( this, "sleep", hsleep );
	HYBRIS_DEFINE_BUILTIN( this, "time", htime );
	HYBRIS_DEFINE_BUILTIN( this, "strtime", hstrtime );
	HYBRIS_DEFINE_BUILTIN( this, "strdate", hstrdate );
	HYBRIS_DEFINE_BUILTIN( this, "fopen", hfopen );
	HYBRIS_DEFINE_BUILTIN( this, "fseek", hfseek );
	HYBRIS_DEFINE_BUILTIN( this, "ftell", hftell );
	HYBRIS_DEFINE_BUILTIN( this, "fsize", hfsize );
	HYBRIS_DEFINE_BUILTIN( this, "fread", hfread );
	HYBRIS_DEFINE_BUILTIN( this, "fgets", hfgets );
	HYBRIS_DEFINE_BUILTIN( this, "fwrite", hfwrite );
	HYBRIS_DEFINE_BUILTIN( this, "fclose", hfclose );
	HYBRIS_DEFINE_BUILTIN( this, "file", hfile );
    HYBRIS_DEFINE_BUILTIN( this, "readdir", hreaddir );
	HYBRIS_DEFINE_BUILTIN( this, "settimeout", hsettimeout );
	HYBRIS_DEFINE_BUILTIN( this, "connect", hconnect );
	HYBRIS_DEFINE_BUILTIN( this, "server", hserver );
	HYBRIS_DEFINE_BUILTIN( this, "accept", haccept );
	HYBRIS_DEFINE_BUILTIN( this, "recv", hrecv );
	HYBRIS_DEFINE_BUILTIN( this, "send", hsend );
	HYBRIS_DEFINE_BUILTIN( this, "close", hclose );
	#ifdef HTTP_SUPPORT
	HYBRIS_DEFINE_BUILTIN( this, "http",     hhttp );
	HYBRIS_DEFINE_BUILTIN( this, "http_get", hhttp_get );
	HYBRIS_DEFINE_BUILTIN( this, "http_post", hhttp_post );
    #endif
	#ifdef XML_SUPPORT
	HYBRIS_DEFINE_BUILTIN( this, "xml_load", hxml_load );
	HYBRIS_DEFINE_BUILTIN( this, "xml_parse", hxml_parse );
    #endif
    HYBRIS_DEFINE_BUILTIN( this, "urlencode", hurlencode );
	HYBRIS_DEFINE_BUILTIN( this, "urldecode", hurldecode );
    HYBRIS_DEFINE_BUILTIN( this, "base64encode", hbase64encode );
	HYBRIS_DEFINE_BUILTIN( this, "base64decode", hbase64decode );
    #ifdef MT_SUPPORT
	HYBRIS_DEFINE_BUILTIN( this, "pthread_create", hpthread_create );
    HYBRIS_DEFINE_BUILTIN( this, "pthread_exit",   hpthread_exit );
    HYBRIS_DEFINE_BUILTIN( this, "pthread_join",   hpthread_join );
    #endif
}

void Context::release( int error /*= 0*/ ){
    #ifdef MT_SUPPORT
    lock();
        if( th_pool.size() > 0 ){
            printf( "[WARNING] Hard killing remaining running threads ... " );
            for( int pool_i = 0; pool_i < th_pool.size(); ++pool_i ){
                pthread_kill( th_pool[pool_i], SIGTERM );
            }
            th_pool.clear();
            printf( "done .\n" );
        }
    unlock();
    #endif

    for( int i = 0; i < builtins.size(); ++i ){
        delete builtins.at(i);
    }
    builtins.clear();

    delete engine;

    vmem.release();
    vcode.release();
    vtypes.release();
}

void Context::load( char *module ){
    void *hmodule = dlopen( module, RTLD_NOW );
    if( !hmodule ){
        char *error = dlerror();
        if( error == NULL ){
            hybris_generic_warning( "module '%s' could not be loaded", module );
        }
        else{
            hybris_generic_warning( "%s", error );
        }
        return;
    }
    /* load module name */
    char *modname = (char *)dlsym( hmodule, "hybris_module_name" );
    if( !modname ){
        dlclose(hmodule);
        hybris_generic_warning( "could not find module name symbol in '%s'", module );
        return;
    }
    /* load initialization routine, usually used for constants definition, etc */
    initializer_t initializer = (initializer_t)dlsym( hmodule, "hybris_module_init" );
    if(initializer){
        initializer( this );
    }
    /* exported functions vector */
    builtin_t *functions = (builtin_t *)dlsym( hmodule, "hybris_module_functions" );
    if(!functions){
        dlclose(hmodule);
        hybris_generic_warning( "could not find module '%s' functions pointer", module );
        return;
    }

    module_t *hmod    = new module_t;
    hmod->name        = modname;
    hmod->initializer = initializer;
    unsigned long i   = 0;

    while( functions[i].function != NULL ){
        builtin_t *function = new builtin_t( functions[i].identifier, functions[i].function );
        hmod->functions.push_back(function);
        ++i;
    }

    modules.push_back(hmod);
}

function_t Context::getFunction( char *identifier ){
    unsigned int i, j,
                 ndyns( modules.size() ),
                 nfuncs;

    /* first search the function in builtins symbols */
    builtin_t *b = builtins.find(identifier);
    if( b != H_UNDEFINED ){
        return b->function;
    }

    /* then search it in dynamic loaded modules */
    for( i = 0; i < ndyns; ++i ){
        /* for each function of the module */
        nfuncs = modules[i]->functions.size();
        for( j = 0; j < nfuncs; ++j ){
            if( modules[i]->functions[j]->identifier == identifier ){
                return modules[i]->functions[j]->function;
            }
        }
    }
    return H_UNDEFINED;
}
