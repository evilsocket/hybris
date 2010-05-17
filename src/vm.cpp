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

#ifndef MAX_STRING_SIZE
#	define MAX_STRING_SIZE 1024
#endif
#ifndef MAX_MESSAGE_SIZE
#	define MAX_MESSAGE_SIZE MAX_STRING_SIZE + 0xFF
#endif

void vm_signal_handler( int signo ){
    if( signo == SIGSEGV ){
        hyb_error( H_ET_GENERIC, "SIGSEGV Signal Catched" );
    }
}

void vm_str_split( string& str, string delimiters, vector<string>& tokens ){
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while( string::npos != pos || string::npos != lastPos ){
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

vm_t *vm_create(){
	vm_t *vm = new vm_t;

    memset( &vm->args, 0x00, sizeof(h_args_t) );
    vm->fp 	      = NULL;
    vm->releasing = false;

    return vm;
}

FILE *vm_fopen( vm_t *vm ){
	extern vector<string> __hyb_file_stack;
	extern vector<int>	  __hyb_line_stack;

    if( vm->args.source[0] != 0x00 ){
    	__hyb_file_stack.push_back( vm->args.source);
    	__hyb_line_stack.push_back(0);

        vm->fp = fopen( vm->args.source, "r" );
        vm_chdir( vm );
    }
    else{
    	__hyb_file_stack.push_back("<stdin>");

    	vm->fp = stdin;
    }

    return vm->fp;
}

void vm_fclose( vm_t *vm ){
    if( vm->args.source[0] != 0x00 && vm->fp ){
        fclose(vm->fp);
    }
}

int vm_chdir( vm_t *vm ){
	/* compute source path and chdir to it */
	char *ptr = strrchr( vm->args.source, '/' );
	if( ptr != NULL ){
		unsigned int pos = ptr - vm->args.source + 1;
		char path[0xFF]  = {0};
		strncpy( path, vm->args.source, pos );
		return ::chdir(path);
	}
	return 0;
}

void vm_init( vm_t *vm, int argc, char *argv[], char *envp[] ){
    int i;
    char name[0xFF] = {0};

    /* create code engine */
    vm->engine = engine_create( vm );
    /* save interpreter directory */
    getcwd( vm->args.rootpath, 0xFF );
    /* initialize pthread mutexes */
    vm->th_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
    vm->mm_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
    vm->mcache_mutex  = PTHREAD_MUTEX_INITIALIZER;
    vm->pcre_mutex	  = PTHREAD_MUTEX_INITIALIZER;

    /* set signal handler */
    signal( SIGSEGV, vm_signal_handler );

    if( vm->args.gc_threshold > 0 ){
    	gc_set_collect_threshold(vm->args.gc_threshold);
    }
    if( vm->args.mm_threshold > 0 ){
		gc_set_mm_threshold(vm->args.mm_threshold);
	}

    vm->vmem.owner = (argc > 1 ? argv[1] : "<stdin>");
    /*
     * The first frame is always the main one.
     */
    vm->frames.push_back(&vm->vmem);

    int h_argc = argc - 1;

    /* initialize command line arguments */
    HYBRIS_DEFINE_CONSTANT( vm, "argc", gc_new_integer(h_argc) );
    for( i = 1; i < argc; ++i ){
        sprintf( name, "%d", i - 1);
        HYBRIS_DEFINE_CONSTANT( vm, name, gc_new_string(argv[i]) );
    }
    /* initialize misc constants */
    HYBRIS_DEFINE_CONSTANT( vm, "null",  gc_new_reference(NULL) );

    HYBRIS_DEFINE_CONSTANT( vm, "__VERSION__",  gc_new_string(VERSION) );
    HYBRIS_DEFINE_CONSTANT( vm, "__LIB_PATH__", gc_new_string(LIB_PATH) );
    HYBRIS_DEFINE_CONSTANT( vm, "__INC_PATH__", gc_new_string(INC_PATH) );
    /*
     * Initiailze environment variables.
     */
    vm->env = envp;
}

void vm_release( vm_t *vm ){
	vm->releasing = true;

    vm_th_pool_lock( vm );
        if( vm->th_pool.size() > 0 ){
            fprintf( stdout, "[WARNING] Hard killing remaining running threads ... " );
            for( int pool_i = 0; pool_i < vm->th_pool.size(); ++pool_i ){
                pthread_kill( vm->th_pool[pool_i], SIGTERM );
            }
            vm->th_pool.clear();
            fprintf( stdout, "done .\n" );
        }
    vm_th_pool_unlock( vm );

    /*
     * Handle unhandled exceptions in the main memory frame.
     */
    if( vm->vmem.state.is(Exception) ){
    	vm->vmem.state.unset(Exception);
    	assert( vm->vmem.state.value != NULL );
    	if( vm->vmem.state.value->type->svalue ){
    		fprintf( stderr, "\033[22;31mERROR : Unhandled exception : %s\n\033[00m", ob_svalue(vm->vmem.state.value).c_str() );
    	}
    	else{
    		fprintf( stderr, "\033[22;31mERROR : Unhandled '%s' exception .\n\033[00m", ob_typename(vm->vmem.state.value) );
    	}
    }
    /*
     * gc_release must be called before anything else because it will
     * need vmem, vconst, vtypes and so on to call classes destructors.
     */
    gc_release();

    engine_free( vm->engine );

    unsigned int i, j,
                 ndyns( vm->modules.size() ),
                 nfuncs;

    for( i = 0; i < ndyns; ++i ){
        nfuncs = vm->modules[i]->functions.size();
        for( j = 0; j < nfuncs; ++j ){
			delete vm->modules[i]->functions[j];
        }
        dlclose( vm->modules[i]->handle );
        delete vm->modules[i];
    }

    vm->modules.clear();
    vm->mcache.clear();
    vm->vconst.release();
    vm->vmem.release();
    vm->vcode.release();
    vm->vtypes.release();

    vm->releasing = false;
}

void vm_load_namespace( vm_t *vm, string path ){
    DIR           *dir;
    struct dirent *ent;

    if( (dir = opendir(path.c_str())) == NULL ) {
        hyb_error( H_ET_GENERIC, "could not open directory '%s' for reading", path.c_str() );
    }

    while( (ent = readdir(dir)) != NULL ){
        /* recurse into directories */
        if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) && strcmp( ent->d_name, "." ) ){
            path = (path[path.size() - 1] == '/' ? path : path + '/');
            vm_load_namespace( vm, path + ent->d_name );
        }
        /* load .so dynamic module */
        else if( strstr( ent->d_name, ".so" ) ){
            string modname = string(ent->d_name);
            modname.replace( modname.find(".so"), 3, "" );
            vm_load_module( vm, path + '/' + ent->d_name, modname );
        }
    }

    closedir(dir);
}

void vm_load_module( vm_t *vm, string path, string name ){
    unsigned int i, a, j, k, max_argc = 0, sz(vm->modules.size());
    /* check that the module isn't already loaded */
    for( i = 0; i < sz; ++i ){
        if( vm->modules[i]->name == name ){
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
        initializer( vm );
    }

    /* exported functions vector */
    named_function_t *functions = (named_function_t *)dlsym( hmodule, "hybris_module_functions" );
    if(!functions){
        dlclose(hmodule);
        hyb_error( H_ET_WARNING, "could not find module '%s' functions pointer", path.c_str() );
        return;
    }

    module_t *hmod    = new module_t;
    vm_str_split( path, "/", hmod->tree );
    hmod->handle	  = hmodule;
    hmod->name        = name;
    hmod->initializer = initializer;
    i = 0;

    while( functions[i].function != NULL ){
        named_function_t *function = new named_function_t();

        function->identifier = functions[i].identifier;
        function->function   = functions[i].function;

        max_argc = 0;
        for( a = 0;; ++a ){
        	int argc = functions[i].argc[a];
        	function->argc[a] = argc;
        	if( argc >= 0 ){
        		if( argc > max_argc ){
        			max_argc = argc;
        		}
        	}
        	else{
        		break;
        	}
        }

        if( max_argc > 0 ){
			/*
			 * For each argument.
			 */
			for( j = 0; j < max_argc; ++j ){
				/*
				 * For each type.
				 */
				for( k = 0;; ++k ){
					H_OBJECT_TYPE type = functions[i].types[j][k];
					if( type != otEndMarker ){
						function->types[j][k] = type;
					}
					else{
						break;
					}
				}
			}
		}

        hmod->functions.push_back(function);
        ++i;
    }

    vm->modules.push_back(hmod);
}

void vm_load_module( vm_t *vm, char *module ){
    /* translate dotted module name to module path */
    string         name(module),
                   path(LIB_PATH),
                   group;
    vector<string> groups;
    unsigned int   i, sz, last;

    /* parse module path and name from dotted notation */
    vm_str_split( name, ".", groups );
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
                return vm_load_namespace( vm, path );
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

    vm_load_module( vm, path, name );
}

Object *vm_raise_exception( const char *fmt, ... ){
	extern vm_t *__hyb_vm;
    char message[MAX_MESSAGE_SIZE] = {0},
	     error[MAX_MESSAGE_SIZE] = {0};
	va_list ap;

	vm_mm_lock( __hyb_vm );

	va_start( ap, fmt );
		vsnprintf( message, MAX_MESSAGE_SIZE, fmt, ap );
	va_end(ap);

	Object *exception = (Object *)gc_new_string(message);

	/*
	 * Make sure the exception object will not be freed until someone
	 * catches it or the program ends.
	 */
	GC_SET_UNTOUCHABLE(exception);

	vm_frame(__hyb_vm)->state.set( Exception, exception );

	vm_mm_unlock( __hyb_vm );

	return H_DEFAULT_ERROR;
}

void vm_print_stack_trace( vm_t *vm, bool force /*= false*/ ){
	if( vm->args.stacktrace || force ){
		list<vframe_t *>::iterator i;
		unsigned int j, stop, pad, k, args, last;
		string name;
		vframe_t *frame;
		extern gc_t __gc;

		stop = (vm->frames.size() >= MAX_RECURSION_THRESHOLD ? 10 : vm->frames.size());

		fprintf( stderr, "\nCall Stack [memory usage %d bytes] :\n\n", __gc.usage );

		fprintf( stderr, "%s\n", vm->vmem.owner.c_str() );
		for( i = vm->frames.begin(), j = 1; i != vm->frames.end() && j < stop; i++, ++j ){
			frame = (*i);
			args  = frame->size();
			last  = args - 1;
			pad   = j;

			while(pad--){
				fprintf( stderr, "  " );
			}

			fprintf( stderr, "%s( )\n", frame->owner.c_str() );
		}

		if( vm->frames.size() >= MAX_RECURSION_THRESHOLD ){
			pad = j;
			while(pad--){
				fprintf( stderr, "  " );
			}

			fprintf( stderr, "... (nested functions not shown)\n", frame->owner.c_str() );
		}

		fprintf( stderr, "\n" );
	}
}
