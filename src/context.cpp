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
        hyb_throw( H_ET_GENERIC, "SIGSEGV Signal Catched" );
    }
}

string Context::mk_trace( char *function, vframe_t *frame ){
    string trace( function + string("(") );
    unsigned int i,
                 size( frame->size() ),
                 i_end( size - 1 );

    for( i = 0; i < size; ++i ){
        trace += " " + ob_svalue( frame->at(i) ) + (i < i_end ? "," : "");
    }
    trace += " )";

    return trace;
}

void Context::str_split( string& str, string delimiters, vector<string>& tokens ){
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while( string::npos != pos || string::npos != lastPos ){
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}


Context::Context(){
    memset( &args, 0x00, sizeof(h_args_t) );
    fp = NULL;
}

FILE *Context::openFile(){
    if( args.source[0] != 0x00 ){
        fp = fopen( args.source, "r" );
        this->chdir();
    }
    else{
        fp = stdin;
    }
    return fp;
}

void Context::closeFile(){
    if( args.source[0] != 0x00 ){
        fclose(fp);
    }
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
    HYBRIS_DEFINE_CONSTANT( this, "argc", MK_TMP_INT_OBJ(argc - 1) );
    for( i = 1; i < argc; ++i ){
        sprintf( name, "%d", i - 1 );
        HYBRIS_DEFINE_CONSTANT( this, name, MK_TMP_STRING_OBJ(argv[i]) );
    }
    /* initialize misc constants */
    HYBRIS_DEFINE_CONSTANT( this, "true",  MK_TMP_INT_OBJ(1) );
    HYBRIS_DEFINE_CONSTANT( this, "false", MK_TMP_INT_OBJ(0) );

    HYBRIS_DEFINE_CONSTANT( this, "__VERSION__",  MK_TMP_STRING_OBJ(VERSION) );
    HYBRIS_DEFINE_CONSTANT( this, "__LIB_PATH__", MK_TMP_STRING_OBJ(LIB_PATH) );
    HYBRIS_DEFINE_CONSTANT( this, "__INC_PATH__", MK_TMP_STRING_OBJ(INC_PATH) );
}

void Context::release(){
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

    delete engine;

    unsigned int i, j,
                 ndyns( modules.size() ),
                 nfuncs;

    for( i = 0; i < ndyns; ++i ){
        nfuncs = modules[i]->functions.size();
        for( j = 0; j < nfuncs; ++j ){
			delete modules[i]->functions[j];
        }
        delete modules[i];
    }

	modules.clear();
	modules_cache.clear();
    vmem.release();
    vcode.release();
    vtypes.release();
    gc_release();
}

void Context::loadNamespace( string path ){
    DIR           *dir;
    struct dirent *ent;

    if( (dir = opendir(path.c_str())) == NULL ) {
        hyb_throw( H_ET_GENERIC, "could not open directory '%s' for reading", path.c_str() );
    }

    while( (ent = readdir(dir)) != NULL ){
        /* recurse into directories */
        if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) && strcmp( ent->d_name, "." ) ){
            path = (path[path.size() - 1] == '/' ? path : path + '/');
            loadNamespace( path + ent->d_name );
        }
        /* load .so dynamic module */
        else if( strstr( ent->d_name, ".so" ) ){
            string modname = string(ent->d_name);
            modname.replace( modname.find(".so"), 3, "" );
            loadModule( path + '/' + ent->d_name, modname );
        }
    }

    closedir(dir);
}

void Context::loadModule( string path, string name ){
    /* check that the module isn't already loaded */
    unsigned int i, sz(modules.size());
    for( i = 0; i < sz; ++i ){
        if( modules[i]->name == name ){
            return;
        }
    }

    /* load the module */
    void *hmodule = dlopen( path.c_str(), RTLD_NOW );
    if( !hmodule ){
        char *error = dlerror();
        if( error == NULL ){
            hyb_throw( H_ET_WARNING, "module '%s' could not be loaded", path.c_str() );
        }
        else{
            hyb_throw( H_ET_WARNING, "%s", error );
        }
        return;
    }

    /* load initialization routine, usually used for constants definition, etc */
    initializer_t initializer = (initializer_t)dlsym( hmodule, "hybris_module_init" );
    if(initializer){
        initializer( this );
    }
    /* exported functions vector */
    named_function_t *functions = (named_function_t *)dlsym( hmodule, "hybris_module_functions" );
    if(!functions){
        dlclose(hmodule);
        hyb_throw( H_ET_WARNING, "could not find module '%s' functions pointer", path.c_str() );
        return;
    }

    module_t *hmod    = new module_t;
    str_split( path, "/", hmod->tree );
    hmod->name        = name;
    hmod->initializer = initializer;
    i = 0;

    while( functions[i].function != NULL ){
        named_function_t *function = new named_function_t( functions[i].identifier, functions[i].function );
        hmod->functions.push_back(function);
        ++i;
    }

    modules.push_back(hmod);
}

void Context::loadModule( char *module ){
    /* translate dotted module name to module path */
    string         name(module),
                   path(LIB_PATH),
                   group;
    vector<string> groups;
    unsigned int   i, sz, last;

    /* parse module path and name from dotted notation */
    str_split( name, ".", groups );
    sz   = groups.size();
    last = sz - 1;
    for( i = 0; i < sz; ++i ){
        group = groups[i];
        if( i == last ){
            /* load all modules in that group */
            if( group == "*" ){
                /* '*' not allowed as first namespace */
                if( i == 0 ){
                    hyb_throw( H_ET_SYNTAX, "Could not use '*' as main namespace" );
                }
                return loadNamespace( path );
            }
            else{
                name = group;
            }

            path += name + ".so";
        }
        else {
            path += group + "/";
        }
    }

    loadModule( path, name );
}

function_t Context::getFunction( char *identifier ){
    unsigned int i, j,
                 ndyns( modules.size() ),
                 nfuncs;

    /* first check if it's already in cache */
    named_function_t * cached_function = modules_cache.find(identifier);
    if( cached_function != H_UNDEFINED ){
		return cached_function->function;
	}

    /* search it in dynamic loaded modules */
    for( i = 0; i < ndyns; ++i ){
        /* for each function of the module */
        nfuncs = modules[i]->functions.size();
        for( j = 0; j < nfuncs; ++j ){
            if( modules[i]->functions[j]->identifier == identifier ){
                /* found it, add to the cache and return */
                cached_function = modules[i]->functions[j];
                modules_cache.insert( identifier, cached_function );
                return cached_function->function;
            }
        }
    }
    return H_UNDEFINED;
}
