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
#include "vm.h"

void VM::signal_handler( int signo ){
    if( signo == SIGSEGV ){
        hyb_error( H_ET_GENERIC, "SIGSEGV Signal Catched" );
    }
}

void VM::str_split( string& str, string delimiters, vector<string>& tokens ){
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while( string::npos != pos || string::npos != lastPos ){
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}


VM::VM(){
    memset( &args, 0x00, sizeof(h_args_t) );
    fp 	   = NULL;
    vframe = &vmem;
}

FILE *VM::openFile(){
	extern vector<string> __hyb_file_stack;
	extern vector<int>	  __hyb_line_stack;

    if( args.source[0] != 0x00 ){
    	__hyb_file_stack.push_back(args.source);
    	__hyb_line_stack.push_back(0);

        fp = fopen( args.source, "r" );
        this->chdir();
    }
    else{
    	__hyb_file_stack.push_back("<stdin>");

        fp = stdin;
    }
    return fp;
}

void VM::closeFile(){
    if( args.source[0] != 0x00 ){
        fclose(fp);
    }
}

int VM::chdir(){
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

void VM::init( int argc, char *argv[] ){
    int i;
    char name[0xFF] = {0};

    /* create code engine */
    engine = new Engine( this );
    /* save interpreter directory */
    getcwd( args.rootpath, 0xFF );
    /* initialize pthread mutex */
    th_mutex = PTHREAD_MUTEX_INITIALIZER;
    /* set signal handler */
    signal( SIGSEGV, VM::signal_handler );

    if( args.gc_threshold > 0 ){
    	gc_set_threshold(args.gc_threshold);
    }

    vmem.owner = (argc > 1 ? argv[1] : "<stdin>");

    /* initialize command line arguments */
    HYBRIS_DEFINE_CONSTANT( this, "argc", gc_new_integer(argc - 1) );
    for( i = 1; i < argc; ++i ){
        sprintf( name, "%d", i - 1 );
        HYBRIS_DEFINE_CONSTANT( this, name, gc_new_string(argv[i]) );
    }
    /* initialize misc constants */
    HYBRIS_DEFINE_CONSTANT( this, "true",  gc_new_integer(1) );
    HYBRIS_DEFINE_CONSTANT( this, "false", gc_new_integer(0) );
    HYBRIS_DEFINE_CONSTANT( this, "null",  gc_new_integer(0) );

    HYBRIS_DEFINE_CONSTANT( this, "__VERSION__",  gc_new_string(VERSION) );
    HYBRIS_DEFINE_CONSTANT( this, "__LIB_PATH__", gc_new_string(LIB_PATH) );
    HYBRIS_DEFINE_CONSTANT( this, "__INC_PATH__", gc_new_string(INC_PATH) );
}

void VM::release(){
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

    /*
     * Handle unhandled exceptions in the main memory frame.
     */
    if( vmem.state.is(Exception) ){
    	vmem.state.unset(Exception);
    	assert( vmem.state.value != NULL );
    	if( vmem.state.value->type->svalue ){
    		fprintf( stderr, "\033[22;31mERROR : Unhandled exception : %s\n\033[00m", ob_svalue(vmem.state.value).c_str() );
    	}
    	else{
    		fprintf( stderr, "\033[22;31mERROR : Unhandled '%s' exception .\n\033[00m", ob_typename(vmem.state.value) );
    	}
    }
    /*
     * gc_release must be called before anything else because it will
     * need vmem, vtypes and so on to call classes destructors.
     */
    gc_release();

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
	mcache.clear();
    vmem.release();
    vcode.release();
    vtypes.release();
}

void VM::loadNamespace( string path ){
    DIR           *dir;
    struct dirent *ent;

    if( (dir = opendir(path.c_str())) == NULL ) {
        hyb_error( H_ET_GENERIC, "could not open directory '%s' for reading", path.c_str() );
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

void VM::loadModule( string path, string name ){
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
            hyb_error( H_ET_WARNING, "module '%s' could not be loaded", path.c_str() );
        }
        else{
            hyb_error( H_ET_WARNING, "%s", error );
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
        hyb_error( H_ET_WARNING, "could not find module '%s' functions pointer", path.c_str() );
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

void VM::printStackTrace( bool force /*= false*/ ){
	if( args.stacktrace || force ){
		list<vframe_t *>::iterator i;
		unsigned int j, pad, k, args, last;
		vframe_t *frame;
		extern gc_t __gc;

		fprintf( stderr, "\nCall Stack [memory usage %d bytes] :\n\n", __gc.usage );

		fprintf( stderr, "%s\n", vmem.owner.c_str() );
		for( i = frames.begin(), j = 1; i != frames.end(); i++, ++j ){
			frame = (*i);
			args  = frame->size();
			last  = args - 1;
			pad   = j;

			while(pad--){
				fprintf( stderr, "  " );
			}

			fprintf( stderr, "%s(%s", frame->owner.c_str(), (args ? " " : "") );
			for( k = 0; k < args; ++k ){
				fprintf( stderr, "%s = %s%s", frame->label(k),
											  ob_svalue( frame->at(k) ).c_str(),
											  (k < last ? ", " : " ") );
			}
			fprintf( stderr, ")\n" );
		}
		fprintf( stderr, "\n" );
	}
}

void VM::loadModule( char *module ){
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
                    hyb_error( H_ET_SYNTAX, "Could not use '*' as main namespace" );
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

