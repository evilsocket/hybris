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
#include "parser.h"
#include "hybris.h"

#ifndef MAX_STRING_SIZE
#	define MAX_STRING_SIZE 1024
#endif
#ifndef MAX_MESSAGE_SIZE
#	define MAX_MESSAGE_SIZE MAX_STRING_SIZE + 0xFF
#endif

void vm_signal_handler( int signo ){
    if( signo == SIGSEGV ){
    	/*
    	 * This will cause the stack trace to be printed.
    	 */
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

    memset( &vm->args, 0x00, sizeof(vm_args_t) );
    /*
     * Input file handle.
     */
    vm->fp 	      = NULL;
    /*
     * Releasing flag.
     */
    vm->releasing = false;
    /*
	* Set the initial vm state.
	*/
   vm->state = vmNone;

    return vm;
}

FILE *vm_fopen( vm_t *vm ){
	extern vector<string> __hyb_file_stack;
	extern vector<int>	  __hyb_line_stack;

    if( *vm->args.source ){
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
    if( *vm->args.source && vm->fp ){
        fclose(vm->fp);
    }
}

int vm_chdir( vm_t *vm ){
	/*
	 * Compute source path and chdir to it.
	 */
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

    /*
     * Initialize main vm thread id.
     */
    vm->main_tid = pthread_self();
    /*
     * Save interpreter directory
     */
    getcwd( vm->args.rootpath, 0xFF );
    /*
     * Initialize vm mutexes
     */
    vm->mm_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
    vm->mcache_mutex  = PTHREAD_MUTEX_INITIALIZER;
    vm->pcre_mutex	  = PTHREAD_MUTEX_INITIALIZER;
    /*
     * Set segmentation fault signal handler
     */
    signal( SIGSEGV, vm_signal_handler );

    /*
     * Eventually set garbage collector user defined thresholds.
     */
    if( vm->args.gc_threshold > 0 ){
    	gc_set_collect_threshold(vm->args.gc_threshold);
    }
    if( vm->args.mm_threshold > 0 ){
		gc_set_mm_threshold(vm->args.mm_threshold);
	}

    vm->vmem.owner = "<main>";
    /*
     * The first frame is always the main one.
     */
    ll_init( &vm->frames );
    ll_append( &vm->frames, &vm->vmem );

    int h_argc = argc - 1;

    /*
     * Initialize command line arguments
     */
    HYBRIS_DEFINE_CONSTANT( vm, "argc", gc_new_integer(h_argc) );
    for( i = 1; i < argc; ++i ){
        sprintf( name, "%d", i - 1);
        HYBRIS_DEFINE_CONSTANT( vm, name, gc_new_string(argv[i]) );
    }
    /*
     * Initialize misc constants
     */
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

    vm_mm_lock( vm );
        if( vm->th_frames.size() ){
            fprintf( stdout, "[WARNING] Hard killing remaining running threads ... " );
            vm_thread_scope_t::iterator ti;
            vv_foreach( vm_thread_scope_t, ti, vm->th_frames ){
            	 pthread_kill( (pthread_t)ti->first, SIGTERM );
            	 delete ti->second;
            }
            vm->th_frames.clear();
            fprintf( stdout, "done .\n" );
        }
    vm_mm_unlock( vm );

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

	ll_item_t	  *m_item,
				  *f_item;
	vm_module_t   *module;

	for( m_item = vm->modules.head; m_item; m_item = m_item->next ){
		module = ll_data( vm_module_t *, m_item );
		for( f_item = module->functions.head; f_item; f_item = f_item->next ){
			delete ll_data( vm_function_t *, f_item );
		}
		ll_clear( &module->functions );
	}

	ll_clear(&vm->modules);

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
    int i(0), a, j, k, max_argc = 0;
    ll_item_t 	*item;
    vm_module_t *module;

    /* check that the module isn't already loaded */
    for( item = vm->modules.head; item; item = item->next ){
    	module = ll_data( vm_module_t *, item );
        if( module->name == name ){
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
    vm_function_t *functions = (vm_function_t *)dlsym( hmodule, "hybris_module_functions" );
    if(!functions){
        dlclose(hmodule);
        hyb_error( H_ET_WARNING, "could not find module '%s' functions pointer", path.c_str() );
        return;
    }

    module = new vm_module_t( name, path, hmodule, initializer );

    while( functions[i].function != NULL ){
        vm_function_t *function = new vm_function_t();

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

        ll_append( &module->functions, function );

        ++i;
    }

    ll_append( &vm->modules, module );
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
    char message[MAX_MESSAGE_SIZE] = {0};
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
	gc_set_alive(exception);

	vm_frame(__hyb_vm)->state.set( Exception, exception );

	vm_mm_unlock( __hyb_vm );

	return H_DEFAULT_ERROR;
}

void vm_print_stack_trace( vm_t *vm, bool force /*= false*/ ){
	if( vm->args.stacktrace || force ){
		ll_item_t *item;
		unsigned int j, stop, pad, args, last;
		string name;
		vframe_t *frame = NULL;
		size_t scopesize = vm_scope_size(vm);

		stop = (scopesize >= VM_MAX_RECURSION ? 10 : scopesize);

		fprintf( stderr, "\nCall Stack [memory usage %d bytes] :\n\n", gc_mm_usage() );

		for( item = vm->frames.head, j = 1; item && j < stop; item = item->next, ++j ){
			frame = ll_data( vframe_t *, item );
			args  = frame->size();
			last  = args - 1;
			pad   = j;

			while(pad--){
				fprintf( stderr, "  " );
			}

			if( frame->owner == "<main>" ){
				fprintf( stderr, "<main>\n" );
			}
			else{
				fprintf( stderr, "%s()\n", frame->owner.c_str() );
			}
		}

		if( scopesize >= VM_MAX_RECURSION ){
			pad = j;
			while(pad--){
				fprintf( stderr, "  " );
			}

			fprintf( stderr, "... (nested functions not shown)\n", frame->owner.c_str() );
		}

		fprintf( stderr, "\n" );
	}
}

void vm_parse_frame_argv( vframe_t *argv, char *format, ... ){
	size_t argc( argv->size() ),
		   i;
	char   *ptr;
	va_list va;

/*
 * Just to not repeat the same things around :P
 */
#define HANDLE_C_TYPE( FMT, TYPE, CONVERSION ) case FMT : { \
													TYPE *ptr = va_arg( va, TYPE * ); \
													*ptr = (TYPE)CONVERSION ; \
											   } \
											   break

#define HANDLE_H_TYPE( FMT, TYPE ) case FMT : { \
									   TYPE **ptr = va_arg( va, TYPE ** ); \
									   *ptr = (TYPE *)o; \
								   } \
								   break

	va_start( va, format );
	/*
	 * With the condition 'i < argc && *ptr', we take the minimun value between
	 * the number of arguments and the length of the format string.
	 * Therefore if we have less arguments than we expected (optional args
	 * for instance), they will not be considered and will be left to
	 * default values.
	 * Viceversa, if we have less format characters than arguments (user
	 * passed too many args to a function), only the right amount of values
	 * will be fetched from the frame and formatted.
	 */
	for( i = 0, ptr = format; i < argc && *ptr; ++i, ++ptr ){
		Object *o = argv->at(i);

		switch( *ptr ){
			/*
			 * C-Types.
			 */
			HANDLE_C_TYPE( 'b', bool,   ob_lvalue(o) );
			HANDLE_C_TYPE( 'i', int,    ob_ivalue(o) );
			HANDLE_C_TYPE( 'l', long,   ob_ivalue(o) );
			HANDLE_C_TYPE( 'd', double, ob_fvalue(o) );
			HANDLE_C_TYPE( 'c', char,   ob_ivalue(o) );
			HANDLE_C_TYPE( 'p', char *, ob_svalue(o).c_str() );
			HANDLE_C_TYPE( 's', string, ob_svalue(o) );
			/*
			 * Hybris types.
			 */
			HANDLE_H_TYPE( 'O', Object );
			HANDLE_H_TYPE( 'E', Extern );
			HANDLE_H_TYPE( 'A', Alias );
			HANDLE_H_TYPE( 'H', Handle );
			HANDLE_H_TYPE( 'V', Vector );
			HANDLE_H_TYPE( 'B', Binary );
			HANDLE_H_TYPE( 'M', Map );
			HANDLE_H_TYPE( 'R', Reference );
			HANDLE_H_TYPE( 'S', Structure );
			HANDLE_H_TYPE( 'C', Class );

			default :
				/*
				 * THIS SHOULD NEVER HAPPEN!
				 */
				assert(!!!"Invalid format given!");
		}
	}
	va_end(va);

#undef HANDLE_C_TYPE
#undef HANDLE_H_TYPE
}

/*
 * Here starts the vm execution functions definition.
 */
INLINE void vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner,  Object *cobj, int argc, Node *prototype, Node *argv ){
	int 	   i, n_ids(prototype->children());
	ll_item_t *iitem, *aitem;
	Object 	  *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );
	/*
	 * Static methods can not use 'me' instance.
	 */
	if( prototype->value.m_static == false ){
		stack.insert( "me", cobj );
	}
	/*
	 * Evaluate each object and insert it into the stack
	 */
	for( i = 0, iitem = prototype->m_children.head, aitem = argv->m_children.head; i < argc; ++i, aitem = aitem->next ){
		value = vm_exec( vm, root, ll_node( aitem ) );

		if( root->state.is(Exception) ){
			vm_dismiss_stack( vm );
			return;
		}
		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ll_node( iitem )->value.m_identifier.c_str(), value );
			iitem = iitem->next;
		}
	}
}

INLINE void vm_prepare_stack( vm_t *vm, vframe_t &stack, string owner, Object *cobj, Node *ids, int argc, ... ){
	va_list    ap;
	int 	   i, n_ids(ids->children());
	ll_item_t *iitem;
	Object 	  *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	stack.owner = owner;

	stack.insert( "me", cobj );
	/*
	 * Evaluate each object and insert it into the stack
	 */
	va_start( ap, argc );
	for( i = 0, iitem = ids->m_children.head; i < argc; ++i ){
		value = va_arg( ap, Object * );
		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ll_node( iitem )->value.m_identifier.c_str(), value );

			iitem = iitem->next;
		}
	}
	va_end(ap);

	vm_add_frame( vm, &stack );
}

INLINE void vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, vector<string> ids, vmem_t *argv ){
	int 	   i, n_ids( ids.size() ), argc;
	Object 	  *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;

	argc = argv->size();
	for( i = 0; i < argc; ++i ){
		value = argv->at(i);
		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ids[i].c_str(), value );
		}
	}

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );
}

INLINE void vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, vector<string> ids, Node *argv ){
	int 	   i, n_ids( ids.size() ), argc;
	ll_item_t *iitem;
	Object 	  *value;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );

	argc = argv->children();
	ll_foreach_to( &argv->m_children, iitem, i, argc ){
		value = vm_exec( vm, root, ll_node( iitem ) );

		if( root->state.is(Exception) ){
			vm_dismiss_stack( vm );
			return;
		}

		if( i >= n_ids ){
			stack.push( value );
		}
		else{
			stack.insert( (char *)ids[i].c_str(), value );
		}
	}
}

INLINE void vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, Extern *fn_pointer, Node *argv ){
	int 	  i, argc;
	Object 	  *value;
	ll_item_t *iitem;

	/*
	 * Check for heavy recursions and/or nested calss.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );
	stack.push( (Object *)fn_pointer );
	argc = argv->children();
	ll_foreach_to( &argv->m_children, iitem, i, argc ){
		value = vm_exec( vm, root, ll_node( iitem ) );
		if( root->state.is(Exception) ){
			vm_dismiss_stack( vm );
			return;
		}
		stack.push( value );
	}
}

INLINE void vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, Node *argv ){
	int 	  i, argc;
	Object 	  *value;
	ll_item_t *iitem;

	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}
	/*
	 * Set the stack owner
	 */
	stack.owner = owner;
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );
	argc = argv->children();
	ll_foreach_to( &argv->m_children, iitem, i, argc ){
		value = vm_exec( vm, root, ll_node( iitem ) );
		if( root->state.is(Exception) ){
			vm_dismiss_stack( vm );
			return;
		}
		stack.push( value );
	}
}

INLINE void vm_prepare_stack( vm_t *vm, vframe_t *root, vm_function_t *function, vframe_t &stack, string owner, Node *argv ){
	int 	i, argc, f_argc, t;
	ll_item_t *iitem;
	Object *value;
	H_OBJECT_TYPE type;
	/*
	 * Check for heavy recursions and/or nested calls.
	 */
	if( vm_scope_size(vm) >= VM_MAX_RECURSION ){
		hyb_error( H_ET_GENERIC, "Reached max number of nested calls" );
	}

	/*
	 * First of all, check that the arguments number is the right one.
	 */
	argc = argv->children();
	for( i = 0 ;; ++i ){
		f_argc = function->argc[i];
		if( f_argc < 0 ){
			break;
		}
		else if( f_argc <= argc ){
			break;
		}
	}
	/*
	 * If f_argc is -1 and i is 0, the first argc descriptor is H_ANY_ARGC so
	 * the function pointer accept any number of arguments, otherwise, if i != 0
	 * and f_argc is -1, we reached the end marker without a match, so the argc
	 * is wrong.
	 */
	if( f_argc == -1 && i != 0 ){
		hyb_error( H_ET_SYNTAX, "Function '%s' requires %s%d argument%s, %d given",
							    function->identifier.c_str(),
							    function->argc[1] >= 0 ? "at least " : "",
							    function->argc[0],
							    function->argc[0] > 1  ? "s" : "",
							    argc );
	}

	stack.owner = owner;
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );
	/*
	 * Ok, argc is the right one (or one of the right ones), now evaluate each
	 * object and check the type.
	 */
	ll_foreach_to( &argv->m_children, iitem, i, argc ){
		value = vm_exec( vm, root, ll_node( iitem ) );

		if( root->state.is(Exception) ){
			vm_dismiss_stack( vm );
			return;
	    }

		if( f_argc != -1 && i < f_argc ){
			for( t = 0 ;; ++t ){
				type = function->types[i][t];
				if( type <= otVoid ){
					break;
				}
				else if( value->type->code == type ){
					break;
				}
			}
			/*
			 * Same as before, check if H_ANY_TYPE given or report error.
			 */
			if( type <= otVoid && t != 0 ){
				std::stringstream error;

				error << "Invalid " << ob_typename(value)
					  << " type for argument " << i + 1
					  << " of '"
					  << function->identifier.c_str()
					  << "' function, required type"
					  << (function->types[i][1] > 0 ? "s are " : " is ");

				for( t = 0 ;; ++t ){
					type = function->types[i][t];
					if( type <= otVoid ){
						break;
					}
					bool prev_last = ( function->types[i][t + 2] <= otVoid );
					bool last      = ( function->types[i][t + 1] <= otVoid );
					error << ob_type_to_string(type) << ( last ? "" : (prev_last ? " or " : ", ") );
				}

				hyb_error( H_ET_SYNTAX, error.str().c_str() );
			}
		}

		stack.push( value );
	}
}

INLINE void vm_dismiss_stack( vm_t *vm ){
	vm_pop_frame( vm );
}

INLINE Node * vm_find_function( vm_t *vm, vframe_t *frame, Node *call ){
    char *callname = (char *)call->value.m_call.c_str();

    /* search first in the code segment */
	Node *function = H_UNDEFINED;
	if( (function = vm->vcode.get(callname)) != H_UNDEFINED ){
		return function;
	}
	/* then search for a function alias */
	Alias *alias = (Alias *)frame->get( callname );
	if( alias != H_UNDEFINED && ob_is_alias(alias) ){
		return (Node *)alias->value;
	}
	/* try to evaluate the call as an alias itself */
	if( call->value.m_alias_call != NULL ){
		alias = (Alias *)vm_exec( vm, frame, call->value.m_alias_call );
		if( ob_is_alias(alias) ){
			return (Node *)alias->value;
		}
	}
	/* function is not defined */
	return H_UNDEFINED;
}

Object *vm_exec( vm_t *vm, vframe_t *frame, Node *node ){
    /*
	 * An exception has been thrown, wait for a try-catch statement or,
	 * when the frame will be deleted (vframe_t class destructor), exit
	 * with a non handled exception.
	 */
	if( frame->state.is(Exception) ){
		return frame->state.value;
	}
    /*
	 * A return statement was succesfully executed, skip everything
	 * now on until the frame will be destroyed and return appropriate
	 * value.
	 */
	else if( frame->state.is(Return) ){
		return frame->state.value;
	}
    /*
     * A next statement was found, so skip nodes execution
     * until one of the loop handlers will reset the flag.
     */
    else if( frame->state.is(Next) ){
    	return H_DEFAULT_RETURN;
    }
	/*
	 * Null node, probably a function or method without statements.
	 */
    else if( node == H_UNDEFINED ){
    	return H_DEFAULT_RETURN;
    }

	/*
	 * Set current line number.
	 */
	vm_set_lineno( vm, node->lineno() );

    switch( node->type() ){
        /* identifier */
        case H_NT_IDENTIFIER :
            return vm_exec_identifier( vm, frame, node );
        /* attribute */
        case H_NT_ATTRIBUTE  :
            return vm_exec_attribute_request( vm, frame, node );
		/* attribute */
		case H_NT_METHOD_CALL :
			return vm_exec_method_call( vm, frame, node );
        /* constant value */
        case H_NT_CONSTANT   :
            return vm_exec_constant( vm, frame, node );
        /* function definition */
        case H_NT_FUNCTION   :
            return vm_exec_function_declaration( vm, frame, node );
        /* structure or class creation */
        case H_NT_NEW :
            return vm_exec_new_operator( vm, frame, node );
        /* function call */
        case H_NT_CALL       :
            return vm_exec_function_call( vm, frame, node );
        /* struct type declaration */
        case H_NT_STRUCT :
            return vm_exec_structure_declaration( vm, frame, node );
        /* class type declaration */
        case H_NT_CLASS :
			return vm_exec_class_declaration( vm, frame, node );

		/* statements */
        case H_NT_STATEMENT :
        	/*
        	 * Call the garbage collection routine every new statement.
        	 * If the routine would be called on expressions too, there would be a high
        	 * risk of loosing tmp values such as evaluations, ecc.
        	 */
        	gc_collect( vm );

            switch( node->value.m_statement ){
				/* statement unless expression */
				case T_UNLESS :
					return vm_exec_unless( vm, frame, node );
                /* if( condition ) */
                case T_IF     :
                    return vm_exec_if( vm, frame, node );
                /* while( condition ){ body } */
                case T_WHILE  :
                    return vm_exec_while( vm, frame, node );
                /* do{ body }while( condition ); */
                case T_DO  :
                    return vm_exec_do( vm, frame, node );
                /* for( initialization; condition; variance ){ body } */
                case T_FOR    :
                    return vm_exec_for( vm, frame, node );
                /* foreach( item of array ) */
                case T_FOREACH :
                    return vm_exec_foreach( vm, frame, node );
                /* foreach( label -> item of map ) */
                case T_FOREACHM :
                    return vm_exec_foreach_mapping( vm, frame, node );
				/* break; */
				case T_BREAK :
					vm_exec_break_state( frame );
				break;
				/* next; */
				case T_NEXT :
					vm_exec_next_state( frame );
				break;
				/* return */
				case T_RETURN :
					return vm_exec_return( vm, frame, node );
                /* (condition ? expression : expression) */
                case T_QUESTION :
                    return vm_exec_question( vm, frame, node );
				/* switch statement */
                case T_SWITCH :
                    return vm_exec_switch( vm, frame, node );
                case T_EXPLODE :
                	return vm_exec_explode( vm, frame, node );
                /* throw expression; */
                case T_THROW :
					return vm_exec_throw( vm, frame, node );
				/* try-catch statement */
                case T_TRY :
                	return vm_exec_try_catch( vm, frame, node );
            }
        break;

        /* expressions */
        case H_NT_EXPRESSION   :
            switch( node->value.m_expression ){
                /* identifier = expression */
                case T_ASSIGN    :
                    return vm_exec_assign( vm, frame, node );
                /* expression ; */
                case T_EOSTMT  :
                    return vm_exec_eostmt( vm, frame, node );
                /* [ a, b, c, d ] */
                case T_ARRAY :
					return vm_exec_array( vm, frame, node );
				/* [ a : b, c : d ] */
                case T_MAP :
					return vm_exec_map( vm, frame, node );
                /* & expression */
                case T_REF :
                	return vm_exec_reference( vm, frame, node );
                break;
                /* `string` */
                case T_BACKTICK :
					return vm_exec_backtick( vm, frame, node );
                break;
                /* $ */
                case T_DOLLAR :
                    return vm_exec_dollar( vm, frame, node );
                /* @ */
                case T_VARGS :
                	return vm_exec_vargs( vm, frame, node );
                /* expression .. expression */
                case T_RANGE :
                    return vm_exec_range( vm, frame, node );
                /* array[] = object; */
                case T_SUBSCRIPTADD :
                    return vm_exec_subscript_push( vm, frame, node );
                /* (identifier)? = object[ expression ]; */
                case T_SUBSCRIPTGET :
                    return vm_exec_subscript_get( vm, frame, node );
                /* object[ expression ] = expression */
                case T_SUBSCRIPTSET :
                    return vm_exec_subscript_set( vm, frame, node );
                /* -expression */
                case T_UMINUS :
                    return vm_exec_uminus( vm, frame, node );
                /* expression ~= expression */
                case T_REGEX_OP :
                    return vm_exec_regex( vm, frame, node );
                /* expression + expression */
                case T_PLUS    :
                    return vm_exec_add( vm, frame, node );
                /* expression += expression */
                case T_PLUSE   :
                    return vm_exec_inplace_add( vm, frame, node );
                /* expression - expression */
                case T_MINUS    :
                    return vm_exec_sub( vm, frame, node );
                /* expression -= expression */
                case T_MINUSE   :
                    return vm_exec_inplace_sub( vm, frame, node );
                /* expression * expression */
                case T_MUL	:
                    return vm_exec_mul( vm, frame, node );
                /* expression *= expression */
                case T_MULE	:
                    return vm_exec_inplace_mul( vm, frame, node );
                /* expression / expression */
                case T_DIV    :
                    return vm_exec_div( vm, frame, node );
                /* expression /= expression */
                case T_DIVE   :
                    return vm_exec_inplace_div( vm, frame, node );
                /* expression % expression */
                case T_MOD    :
                    return vm_exec_mod( vm, frame, node );
                /* expression %= expression */
                case T_MODE   :
                    return vm_exec_inplace_mod( vm, frame, node );
                /* expression++ */
                case T_INC    :
                    return vm_exec_inc( vm, frame, node );
                /* expression-- */
                case T_DEC    :
                    return vm_exec_dec( vm, frame, node );
                /* expression ^ expression */
                case T_XOR    :
                    return vm_exec_xor( vm, frame, node );
                /* expression ^= expression */
                case T_XORE   :
                    return vm_exec_inplace_xor( vm, frame, node );
                /* expression & expression */
                case T_AND    :
                    return vm_exec_and( vm, frame, node );
                /* expression &= expression */
                case T_ANDE   :
                    return vm_exec_inplace_and( vm, frame, node );
                /* expression | expression */
                case T_OR     :
                    return vm_exec_or( vm, frame, node );
                /* expression |= expression */
                case T_ORE    :
                    return vm_exec_inplace_or( vm, frame, node );
                /* expression << expression */
                case T_SHIFTL  :
                    return vm_exec_shiftl( vm, frame, node );
                /* expression <<= expression */
                case T_SHIFTLE :
                    return vm_exec_inplace_shiftl( vm, frame, node );
                /* expression >> expression */
                case T_SHIFTR  :
                    return vm_exec_shiftr( vm, frame, node );
                /* expression >>= expression */
                case T_SHIFTRE :
                    return vm_exec_inplace_shiftr( vm, frame, node );
                /* expression! */
                case T_FACT :
                    return vm_exec_fact( vm, frame, node );
                /* ~expression */
                case T_NOT    :
                    return vm_exec_not( vm, frame, node );
                /* !expression */
                case T_L_NOT   :
                    return vm_exec_lnot( vm, frame, node );
                /* expression < expression */
                case T_LESS    :
                    return vm_exec_less( vm, frame, node );
                /* expression > expression */
                case T_GREATER    :
                    return vm_exec_greater( vm, frame, node );
                /* expression >= expression */
                case T_GREATER_EQ     :
                    return vm_exec_ge( vm, frame, node );
                /* expression <= expression */
                case T_LESS_EQ     :
                    return vm_exec_le( vm, frame, node );
                /* expression != expression */
                case T_NOT_SAME     :
                    return vm_exec_ne( vm, frame, node );
                /* expression == expression */
                case T_SAME     :
                    return vm_exec_eq( vm, frame, node );
                /* expression && expression */
                case T_L_AND   :
                    return vm_exec_land( vm, frame, node );
                /* expression || expression */
                case T_L_OR    :
                    return vm_exec_lor( vm, frame, node );
            }
    }

    return H_DEFAULT_RETURN;
}

INLINE Object *vm_exec_identifier( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    Node   *function   = H_UNDEFINED;
    char   *identifier = (char *)node->value.m_identifier.c_str();

    /*
   	 * First thing first, check for a constant object name.
   	 */
   	if( (o = vm->vconst.get(identifier)) != H_UNDEFINED ){
   		return o;
   	}
   	/*
	 * Search for the identifier definition on
	 * the function local stack frame.
	 */
   	else if( (o = frame->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Let's check if the address of this frame si different from
	 * global frame one, in that case try to search the definition
	 * on the global frame too.
	 */
	else if( H_ADDRESS_OF(frame) != H_ADDRESS_OF(&vm->vmem) && (o = vm->vmem.get( identifier )) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Check for an user defined object (structure or class) name.
	 */
	else if( (o = vm->vtypes.get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * So, it's neither defined on local frame nor in the global one,
	 * let's search for it in the vm->vcode frame.
	 */
	else if( (function = vm->vcode.find( identifier )) != H_UNDEFINED ){
		/*
		 * Create an alias to that vm->vcode region (basically its index).
		 */
		return ob_dcast( gc_new_alias( H_ADDRESS_OF(function) ) );
	}
	/*
	 * Ok ok, got it! It's undefined, raise an error.
	 */
	else{
		/*
		 * Check for the special 'me' keyword.
		 * If 'me' instance is not defined anywhere, we are in the
		 * main program body or inside a static method.
		 */
		if( strcmp( identifier, "me" ) != 0 ){
			hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
		}
		else{
			hyb_error( H_ET_SYNTAX, "couldn't use 'me' instance inside a global or static scope" );
		}
	}
}

INLINE Object *vm_exec_attribute_request( vm_t *vm, vframe_t *frame, Node *node ){
	Object  *cobj      = H_UNDEFINED,
		    *attribute = H_UNDEFINED;
	char    *name,
			*owner_id;
	access_t access;
	Node    *member = node->value.m_member;

	cobj      = vm_exec( vm, frame, node->value.m_owner );
	owner_id  = (char *)node->value.m_owner->value.m_identifier.c_str();
	name      = (char *)member->value.m_identifier.c_str();
	attribute = ob_get_attribute( cobj, name, true );

	if( attribute == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' is not an attribute of object '%s'", name, ob_typename(cobj) );
	}
	/*
	 * Check attribute access.
	 */
	access = ob_attribute_access( cobj, name );
	/*
	 * If the attribute has public access, skip the access checking
	 * because everyone can access it.
	 */
	if( access != asPublic ){
		/*
		 * Protected attributes can be accessed only by the class itself
		 * or derived classes.
		 */
		if( access == asProtected && strcmp( owner_id, "me" ) != 0 ){
			hyb_error( H_ET_SYNTAX, "Protected attribute '%s' can be accessed only by derived classes of '%s'", name, ob_typename(cobj) );
		}
		/*
		 * The attribute is private, so only the owner can use it.
		 * Let's check if the class pointed by 'me' it's the owner of
		 * the private attribute.
		 */
		else if( access == asPrivate && strcmp( owner_id, "me" ) != 0 ){
			hyb_error( H_ET_SYNTAX, "Private attribute '%s' can be accessed only within '%s' class", name, ob_typename(cobj) );
		}
	}

	return attribute;
}

INLINE Object *vm_exec_method_call( vm_t *vm, vframe_t *frame, Node *node ){
	Object  *cobj   = H_UNDEFINED;
	char    *name,
			*owner_id;
	Node    *member = node->value.m_member;

	cobj 	 = vm_exec( vm, frame, node->value.m_owner );
	owner_id = (char *)node->value.m_owner->value.m_identifier.c_str();
	name 	 = (char *)member->value.m_call.c_str();

	return ob_call_method( vm, frame, cobj, owner_id, name, member );
}

INLINE Object *vm_exec_constant( vm_t *vm, vframe_t *frame, Node *node ){
	/*
	 * Constants are obviously not evaluated every time, just
	 * the first time when the parser is traveling around the
	 * syntax tree and finds out a constant.
	 * For this reason, constants do NOT go out of scope, out
	 * of references, out of fuel, out-what-u-like-most.
	 * If the garbage collector detects an object to be a constant,
	 * (the object->attributes bitmask is set to H_OT_CONSTANT), it
	 * simply skips it in the loop.
	 */
    return node->value.m_constant;
}

INLINE Object *vm_exec_function_declaration( vm_t *vm, vframe_t *frame, Node *node ){
    char *function_name = (char *)node->value.m_function.c_str();

    /* check for double definition */
    if( vm->vcode.get(function_name) != H_UNDEFINED ){
        hyb_error( H_ET_SYNTAX, "function '%s' already defined", function_name );
    }
    else if( vm_get_function( vm, function_name ) != H_UNDEFINED ){
        hyb_error( H_ET_SYNTAX, "function '%s' already defined as a language function", function_name );
    }
    /* add the function to the vm->vcode segment */
    vm->vcode.add( function_name, node );

    return H_UNDEFINED;
}

INLINE Object *vm_exec_structure_declaration( vm_t *vm, vframe_t *frame, Node * node ){
    char *structname = (char *)node->value.m_identifier.c_str();

	if( vm->vtypes.find(structname) != H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "Structure '%s' already defined", structname );
	}

	/* structure prototypes are not garbage collected */
    Object *s = (Object *)(new Structure());
    ll_foreach( &node->m_children, llitem ){
    	ob_add_attribute( s, (char *)ll_node( llitem )->value.m_identifier.c_str() );
    }

    /*
     * ::defineType will take care of the structure attributes
	 * to prevent it to be garbage collected (see ::onConstant).
     */
    vm_define_type( vm, structname, s );

    return H_UNDEFINED;
}

INLINE Object *vm_exec_class_declaration( vm_t *vm, vframe_t *frame, Node *node ){
	int        i, members( node->children() );
	char      *classname = (char *)node->value.m_identifier.c_str(),
			  *attrname;
	Node      *declchild,
			  *attribute;
	Object    *static_attr_value;

	if( vm->vtypes.find(classname) != H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "Class '%s' already defined", classname );
	}

	/* class prototypes are not garbage collected */
	Object *c = (Object *)(new Class());
	/*
	 * Set specific class name.
	 */
	((Class *)c)->name = classname;

	ll_foreach( &node->m_children, llitem ){
		declchild = ll_node( llitem );
		/*
		 * Define an attribute
		 */
		if( declchild->type() == H_NT_IDENTIFIER ){
			attribute = declchild;
			/*
			 * Initialize static attributes.
			 */
			if( attribute->value.m_static ){
				static_attr_value = vm_exec( vm, frame, attribute->child(0) );
				/*
				 * Static attributes are not garbage collectable until the end of
				 * the program is reached because they reside in a global scope.
				 */
				gc_set_alive(static_attr_value);
				/*
				 * Initialize the attribute definition in the prototype.
				 */
				ob_class_ucast(c)->c_attributes.insert( attribute->id(),
														new class_attribute_t( attribute->id(),
																			   attribute->value.m_access,
																			   static_attr_value,
																			   attribute->value.m_static ) );
			}
			else{
				/*
				 * Non static attribute, just define it with a void value.
				 */
				ob_define_attribute( c, attribute->id(), attribute->value.m_access, attribute->value.m_static );
			}
		}
		/*
		 * Define a method
		 */
		else if( declchild->type() == H_NT_METHOD_DECL ){
			ob_define_method( c, (char *)declchild->value.m_method.c_str(), declchild );
		}
		/*
		 * WTF this should not happen!
		 * The parser should not accept anything that's not an attribute
		 * or a method declaration.
		 */
		else{
			hyb_error( H_ET_GENERIC, "unexpected node type for class declaration" );
		}
	}
	/*
	 * Check if the class extends some other class.
	 */
	ClassNode *cnode = (ClassNode *)node;
	Node	  *baseclass;
	Object    *baseproto;

	if( ll_size(&cnode->m_extends) > 0 ){
		ll_foreach( &cnode->m_extends, llnode ){
			baseclass = ll_node( llnode );
			baseproto = vm_get_type( vm, (char *)baseclass->value.m_identifier.c_str() );
			if( baseproto == H_UNDEFINED ){
				hyb_error( H_ET_SYNTAX, "'%s' undeclared class type", baseclass->value.m_identifier.c_str() );
			}
			else if( ob_is_class(baseproto) == false ){
				hyb_error( H_ET_SYNTAX, "couldn't extend from '%s' type", ob_typename(baseproto) );
			}

			Class 			       *cobj = (Class *)baseproto;
			ClassAttributeIterator  ai;
			ClassMethodIterator 	mi;
			ClassPrototypesIterator pi;

			vv_foreach( ITree<class_attribute_t>, ai, cobj->c_attributes ){
				attrname  = (char *)(*ai)->label.c_str();

				ob_define_attribute( c, attrname, (*ai)->value->access, (*ai)->value->is_static );

				/*
				 * Initialize static attributes.
				 */
				if( (*ai)->value->is_static){
					ob_set_attribute_reference( c, attrname, (*ai)->value->value );
				}
			}

			vv_foreach( ITree<class_method_t>, mi, cobj->c_methods ){
				vv_foreach( vector<Node *>, pi, (*mi)->value->prototypes ){
					ob_define_method( c, (char *)(*mi)->label.c_str(), *pi );
				}
			}
		}
	}

	/*
	 * ::defineType will take care of the class attributes
	 * to prevent it to be garbage collected (see ::onConstant).
	 */
	vm_define_type( vm, classname, c );
}

INLINE Object *vm_exec_builtin_function_call( vm_t *vm, vframe_t *frame, Node * call ){
    char        *callname = (char *)call->value.m_call.c_str();
    vm_function_t* function;
    vframe_t     stack;
    Object      *result = H_UNDEFINED;

    if( (function = vm_get_function( vm, callname )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    vm_prepare_stack( vm, frame, function, stack, string(callname), call );

    vm_check_frame_exit(frame);

    /* call the function */
    result = function->function( vm, &stack );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

    vm_dismiss_stack( vm );

    /* return function evaluation value */
    return result;
}

Object *vm_exec_threaded_call( vm_t *vm, string function_name, vframe_t *frame, vmem_t *argv ){
	Node    *function = H_UNDEFINED;
	vframe_t stack;
	Object  *result   = H_UNDEFINED;
	Node	*body     = H_UNDEFINED;
	vector<string> identifiers;

	/* search first in the vm->vcode segment */
	if( (function = vm->vcode.get((char *)function_name.c_str())) == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' undeclared user function identifier", function_name.c_str() );
	}

	ll_foreach( &function->m_children, llitem ){
		body = ll_node( llitem );
    	if( body->type() == H_NT_IDENTIFIER ){
    		identifiers.push_back( body->value.m_identifier );
    	}
    	else{
    		break;
    	}
	}


    if( function->value.m_vargs ){
    	if( argv->size() < identifiers.size() ){
			hyb_error( H_ET_SYNTAX, "function '%s' requires at least %d parameters (called with %d)",
									function_name.c_str(),
									identifiers.size(),
									argv->size() );
    	}
    }
    else{
    	if( identifiers.size() != argv->size() ){
			hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
								   function_name.c_str(),
								   identifiers.size(),
								   argv->size() );
    	}
	}

	vm_prepare_stack( vm, frame, stack, function_name, identifiers, argv );

	vm_check_frame_exit(frame);

	/* call the function */
	result = vm_exec( vm, &stack, body );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

	vm_dismiss_stack( vm );

	/* return function evaluation value */
	return result;
}

Object *vm_exec_threaded_call( vm_t *vm, Node *function, vframe_t *frame, vmem_t *argv ){
	vframe_t stack;
	Object  *result = H_UNDEFINED;
	Node    *body   = H_UNDEFINED;
	vector<string> identifiers;

	ll_foreach( &function->m_children, llitem ){
		body = ll_node( llitem );
    	if( body->type() == H_NT_IDENTIFIER ){
    		identifiers.push_back( body->value.m_identifier );
    	}
    	else{
    		break;
    	}
	}

	if( identifiers.size() != argv->size() ){
		hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
							    function->value.m_function.c_str(),
							    identifiers.size(),
							    argv->size() );
	}

	vm_prepare_stack( vm, frame, stack, function->value.m_function, identifiers, argv );

	vm_check_frame_exit(frame);

	/* call the function */
	result = vm_exec( vm, &stack, body );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

	vm_dismiss_stack( vm );

	/* return function evaluation value */
	return result;
}

INLINE Object *vm_exec_user_function_call( vm_t *vm, vframe_t *frame, Node *call ){
    Node    *function = H_UNDEFINED;
    vframe_t stack;
    Object  *result   = H_UNDEFINED;
    Node    *body     = H_UNDEFINED;

    vector<string> identifiers;

    if( (function = vm_find_function( vm, frame, call )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }

    size_t 	   i(0),
			   argc( function->value.m_argc );
    ll_item_t *iitem;
    Node	  *idnode;

    ll_foreach_to( &function->m_children, iitem, i, argc ){
    	idnode = ll_node( iitem );
    	identifiers.push_back( idnode->value.m_identifier );
    }

    if( function->value.m_vargs ){
    	if( call->children() < identifiers.size() ){
   			hyb_error( H_ET_SYNTAX, "function '%s' requires at least %d parameters (called with %d)",
									function->value.m_function.c_str(),
   									identifiers.size(),
   									call->children() );
       }
   	}
    else{
		if( identifiers.size() != call->children() ){
			hyb_error( H_ET_SYNTAX, "function '%s' requires %d parameters (called with %d)",
									function->value.m_function.c_str(),
									identifiers.size(),
									call->children() );
		}
    }

    vm_prepare_stack( vm, frame, stack, function->value.m_function, identifiers, call );

    vm_check_frame_exit(frame);

    /* call the function */
    result = vm_exec( vm, &stack, function->body() );

    vm_dismiss_stack( vm );
	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		frame->state.set( Exception, stack.state.value );
	}

    /* return function evaluation value */
    return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

INLINE Object *vm_exec_new_operator( vm_t *vm, vframe_t *frame, Node *type ){
    char      *type_name = (char *)type->value.m_identifier.c_str();
    ll_item_t *llitem;
    Object    *user_type = H_UNDEFINED,
              *newtype   = H_UNDEFINED,
              *object    = H_UNDEFINED;
    size_t     i, children( type->children() );

	/*
	 * Search for the used defined type calls, most like C++
	 * class constructors but without strict prototypes.
	 */
    if( (user_type = vm_get_type( vm,type_name)) == H_UNDEFINED ){
    	hyb_error( H_ET_SYNTAX, "'%s' undeclared type", type_name );
    }
    newtype = ob_clone(user_type);

	/*
	 * It's ok to initialize less attributes that the structure/class
	 * has (non ini'ed attributes are set to 0 by default), but
	 * you can not set more attributes than the structure/class have.
	 */
	if( ob_is_struct(newtype) ){
		Structure *stype = (Structure *)newtype;

		if( children > stype->items ){
			hyb_error( H_ET_SYNTAX, "structure '%s' has %d attributes, initialized with %d",
								 type_name,
								 stype->items,
								 children );
		}

		ll_foreach_to( &type->m_children, llitem, i, children ){
			object = vm_exec( vm, frame, ll_node( llitem ) );
			ob_set_attribute( newtype, (char *)stype->s_attributes.label(i), object );
		}
	}
	else if( ob_is_class(newtype) ){
		/*
		 * Set specific class type name.
		 */
		((Class *)newtype)->name = type_name;
		/*
		 * First of all, check if the user has declared an explicit
		 * class constructor, in that case consider it instead of the
		 * default "by arg" constructor.
		 */
		Node *ctor = ob_get_method( newtype, type_name, children );
		if( ctor != H_UNDEFINED ){
			if( ctor->value.m_vargs ){
				if( children < ctor->value.m_argc ){
					hyb_error( H_ET_SYNTAX, "class '%s' constructor requires at least %d arguments, called with %d",
											 type_name,
											 ctor->value.m_argc,
											 children );
			   }
			}
			else{
				if( children > ctor->value.m_argc ){
					hyb_error( H_ET_SYNTAX, "class '%s' constructor requires %d arguments, called with %d",
											 type_name,
											 ctor->value.m_argc,
											 children );
				}
			}

			vframe_t stack;

			vm_prepare_stack( vm,
							  frame,
							  stack,
							  string(type_name) + "::" + string(type_name),
							  newtype,
							  children,
							  ctor,
							  type );

			vm_check_frame_exit(frame);

			/* call the ctor */
			vm_exec( vm, &stack, ctor->body() );

			vm_dismiss_stack( vm );

			/*
			 * Check for unhandled exceptions and put them on the root
			 * memory frame.
			 */
			if( stack.state.is(Exception) ){
				frame->state.set( Exception, stack.state.value );
			}
		}
	}

    return newtype;
}

INLINE Object *vm_exec_dll_function_call( vm_t *vm, vframe_t *frame, Node *call ){
    char    *callname   = (char *)call->value.m_call.c_str();
    vframe_t stack;
    Object  *result     = H_UNDEFINED,
            *fn_pointer = H_UNDEFINED;

    /*
     * We assume that dll module is already loaded, otherwise there shouldn't be
     * any vm_exec_dll_function_call call .
     */
    vm_function_t *dllcall = vm_get_function( vm, (char *)"dllcall" );

    if( (fn_pointer = frame->get( callname )) == H_UNDEFINED ){
        return H_UNDEFINED;
    }
    else if( ob_is_extern(fn_pointer) == false ){
        return H_UNDEFINED;
    }

    vm_prepare_stack( vm, frame, stack, string(callname), (Extern *)fn_pointer, call );

    vm_check_frame_exit(frame);

    /* call the function */
    result = dllcall->function( vm, &stack );

    vm_dismiss_stack( vm );

    /* return function evaluation value */
    return result;
}

Object *vm_exec_function_call( vm_t *vm, vframe_t *frame, Node *call ){
    Object *result = H_UNDEFINED;

    /* check if function is a builtin function */
    if( (result = vm_exec_builtin_function_call( vm, frame, call )) != H_UNDEFINED ){
    	return result;
    }
    /* check for an user defined function */
    else if( (result = vm_exec_user_function_call( vm, frame, call )) != H_UNDEFINED ){
    	return result;
    }
    /* check if the function is an extern identifier loaded by dll importing routines */
    else if( (result = vm_exec_dll_function_call( vm, frame, call )) != H_UNDEFINED ){
    	return result;
    }
    else{
    	hyb_error( H_ET_SYNTAX, "'%s' undeclared function identifier", call->value.m_call.c_str() );
    }

    return result;
}

INLINE Object *vm_exec_reference( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = vm_exec( vm, frame, node->child(0) );

    return (Object *)gc_new_reference(o);
}

INLINE Object *vm_exec_dollar( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;
    Node *function = H_UNDEFINED;
    string svalue;
    char *identifier;

    o 	   	   = vm_exec( vm, frame, node->child(0) );
    svalue 	   = ob_svalue(o);
    identifier = (char *)svalue.c_str();

    /*
   	 * Same as vm_exec_identifier.
   	 */
   	if( (o = vm->vconst.get(identifier)) != H_UNDEFINED ){
   		return o;
   	}
   	/*
	 * Search for the identifier definition on
	 * the function local stack frame.
	 */
   	else if( (o = frame->get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Let's check if the address of this frame si different from
	 * global frame one, in that case try to search the definition
	 * on the global frame too.
	 */
	else if( H_ADDRESS_OF(frame) != H_ADDRESS_OF(&vm->vmem) && (o = vm->vmem.get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * Check for an user defined object (structure or class) name.
	 */
	else if( (o = vm->vtypes.get(identifier)) != H_UNDEFINED ){
		return o;
	}
	/*
	 * So, it's neither defined on local frame nor in the global one,
	 * let's search for it in the vm->vcode frame.
	 */
	else if( (function = vm->vcode.find(identifier)) != H_UNDEFINED ){
		/*
		 * Create an alias to that vm->vcode region (basically its index).
		 */
		return ob_dcast( gc_new_alias( H_ADDRESS_OF(function) ) );
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' undeclared identifier", identifier );
	}
}

INLINE Object *vm_exec_return( vm_t *vm, vframe_t *frame, Node *node ){
	/*
	 * Set break and return state to make every loop and/or condition
	 * statement to exit with this return value.
	 */
    frame->state.value = vm_exec( vm, frame, node->child(0) );
    frame->state.set( Break );
    frame->state.set( Return );

    return frame->state.value;
}

INLINE Object *vm_exec_backtick( vm_t *vm, vframe_t *frame, Node *node ){
	Object *cmd  = vm_exec( vm, frame, node->child(0) );
	FILE   *pipe = popen( ob_svalue(cmd).c_str(), "r" );

	if( !pipe ){
		return H_DEFAULT_ERROR;
	}

	char buffer[128];
	std::string result = "";

	while( !feof(pipe) ){
		if( fgets( buffer, 128, pipe ) != NULL ){
			result += buffer;
		}
	}
	pclose(pipe);

	return (Object *)gc_new_string( result.c_str() );
}

INLINE Object *vm_exec_vargs( vm_t *vm, vframe_t *frame, Node *node ){
	Object *vargs = (Object *)gc_new_vector();
	int i, argc( frame->size() );

	for( i = 0; i < argc; ++i ){
		if( strcmp( frame->label(i), "me" ) != 0 ){
			ob_cl_push( vargs, frame->at(i) );
		}
	}

	return vargs;
}

INLINE Object *vm_exec_range( vm_t *vm, vframe_t *frame, Node *node ){
    Object *range = H_UNDEFINED,
           *from  = H_UNDEFINED,
           *to    = H_UNDEFINED;

    from  = vm_exec( vm, frame, node->child(0) );
   	to    = vm_exec( vm, frame, node->child(1) );

   	vm_check_frame_exit(frame)

	range = ob_range( from, to );

	return range;
}

INLINE Object *vm_exec_subscript_push( vm_t *vm, vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *object = H_UNDEFINED,
           *res    = H_UNDEFINED;

    array  = vm_exec( vm, frame, node->child(0) );
	object = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	res    = ob_cl_push( array, object );

	return res;
}

INLINE Object *vm_exec_subscript_get( vm_t *vm, vframe_t *frame, Node *node ){
    Object *identifier = H_UNDEFINED,
           *array      = H_UNDEFINED,
           *index      = H_UNDEFINED,
           *result     = H_UNDEFINED;

    if( node->children() == 3 ){
		array 	   = vm_exec( vm, frame, node->child(1) );
		identifier = vm_exec( vm, frame, node->child(0) );
		index      = vm_exec( vm, frame, node->child(2) );

		vm_check_frame_exit(frame)

		ob_assign( identifier,
					ob_cl_at( array, index )
				  );

		result = identifier;
	}
	else{
		array  = vm_exec( vm, frame, node->child(0) );
		index  = vm_exec( vm, frame, node->child(1) );

		vm_check_frame_exit(frame)

		result = ob_cl_at( array, index );
	}

		return result;
}

INLINE Object *vm_exec_subscript_set( vm_t *vm, vframe_t *frame, Node *node ){
    Object *array  = H_UNDEFINED,
           *index  = H_UNDEFINED,
           *object = H_UNDEFINED;

    array  = vm_exec( vm, frame, node->child(0) );
   	index  = vm_exec( vm, frame, node->child(1) );
   	object = vm_exec( vm, frame, node->child(2) );

   	vm_check_frame_exit(frame)

   	ob_cl_set( array, index, object );

   	return array;
}

INLINE Object *vm_exec_while( vm_t *vm, vframe_t *frame, Node *node ){
    Node   *condition,
		   *body;
    Object *result  = H_UNDEFINED;

    condition = node->child(0);
    body      = node->child(1);

    while( ob_lvalue( vm_exec( vm, frame, condition ) ) ){
   		result = vm_exec( vm, frame, body );

   		vm_check_frame_exit(frame)

   		frame->state.unset(Next);
        if( frame->state.is(Break) ){
        	frame->state.unset(Break);
			break;
        }
    }

    return result;
}

INLINE Object *vm_exec_do( vm_t *vm, vframe_t *frame, Node *node ){
    Node *condition,
         *body;

    Object *result  = H_UNDEFINED;

    body      = node->child(0);
    condition = node->child(1);
    do{
		result = vm_exec( vm, frame, body );

		vm_check_frame_exit(frame)

		frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }
    while( ob_lvalue( vm_exec( vm, frame, condition ) ) );

    return result;
}

INLINE Object *vm_exec_for( vm_t *vm, vframe_t *frame, Node *node ){
    Node   *condition,
           *increment,
		   *body;
    Object *result  = H_UNDEFINED;

    condition = node->child(1);
    increment = node->child(2);
    body      = node->child(3);

    vm_exec( vm, frame, node->child(0) );

    vm_check_frame_exit(frame)

    for( ; ob_lvalue( vm_exec( vm, frame, condition ) ); vm_exec( vm, frame, increment ) ){

    	vm_check_frame_exit(frame)

    	result = vm_exec( vm, frame, body );

		vm_check_frame_exit(frame)

		frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }

    return result;
}

INLINE Object *vm_exec_foreach( vm_t *vm, vframe_t *frame, Node *node ){
    int     size;
    Node   *body;
    Object *v      = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *identifier;
    Integer index(0);

    identifier = (char *)node->child(0)->value.m_identifier.c_str();
    v          = vm_exec( vm, frame, node->child(1) );
    body       = node->child(2);
    size       = ob_get_size(v);

    /*
     * Prevent the vector from being garbage collected, because may cause
     * seg faults in situations like :
     *
     * 		foreach( i of 1..10 )
     */
    frame->push_tmp(v);

    for( ; index.value < size; ++index.value ){
        frame->add( identifier, ob_cl_at( v, (Object *)&index ) );

        result = vm_exec( vm, frame, body );

        if( frame->state.is(Exception) || frame->state.is(Return) ){
        	result = frame->state.value;
        	break;
	    }

        frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }

    frame->remove_tmp(v);

    return result;
}

INLINE Object *vm_exec_foreach_mapping( vm_t *vm, vframe_t *frame, Node *node ){
    int     i, size;
    Node   *body;
    Object *map    = H_UNDEFINED,
           *result = H_UNDEFINED;
    char   *key_identifier,
           *value_identifier;

    key_identifier   = (char *)node->child(0)->value.m_identifier.c_str();
    value_identifier = (char *)node->child(1)->value.m_identifier.c_str();
    map              = vm_exec( vm, frame, node->child(2) );
    body             = node->child(3);
    size             = ob_get_size(map);

    /*
     * Prevent the map from being garbage collected, because may cause
     * seg faults in situations like :
     *
     * 		foreach( i of map( ... ) )
     */
    frame->push_tmp(map);

    for( i = 0; i < size; ++i ){
        frame->add( key_identifier,   ob_map_ucast(map)->keys[i] );
        frame->add( value_identifier, ob_map_ucast(map)->values[i] );

        result = vm_exec( vm, frame, body );

        if( frame->state.is(Exception) || frame->state.is(Return) ){
			result = frame->state.value;
			break;
		}

        frame->state.unset(Next);
		if( frame->state.is(Break) ){
			frame->state.unset(Break);
			break;
		}
    }

    frame->remove_tmp(map);

    return result;
}

INLINE Object *vm_exec_unless( vm_t *vm, vframe_t *frame, Node *node ){
	Object *boolean = H_UNDEFINED,
		   *result  = H_UNDEFINED;

	boolean = vm_exec( vm, frame, node->child(1) );

	if( !ob_lvalue(boolean) ){
		result = vm_exec( vm, frame, node->child(0) );
	}

	vm_check_frame_exit(frame)

	return H_UNDEFINED;
}

INLINE Object *vm_exec_if( vm_t *vm, vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = vm_exec( vm, frame, node->child(0) );

    if( ob_lvalue(boolean) ){
        result = vm_exec( vm, frame, node->child(1) );
    }
    /* handle else case */
    else if( node->children() > 2 ){
        result = vm_exec( vm, frame, node->child(2) );
    }

    vm_check_frame_exit(frame)

    return H_UNDEFINED;
}

INLINE Object *vm_exec_question( vm_t *vm, vframe_t *frame, Node *node ){
    Object *boolean = H_UNDEFINED,
           *result  = H_UNDEFINED;

    boolean = vm_exec( vm, frame, node->child(0) );

    if( ob_lvalue(boolean) ){
        result = vm_exec( vm, frame, node->child(1) );
    }
    else{
        result = vm_exec( vm, frame, node->child(2) );
    }

    vm_check_frame_exit(frame)

    return result;
}

INLINE Object *vm_exec_switch( vm_t *vm, vframe_t *frame, Node *node){
	ll_item_t *stmt_item,
			  *case_item;
    Node   *case_node = H_UNDEFINED,
           *stmt_node = H_UNDEFINED;
    Object *target    = H_UNDEFINED,
           *compare   = H_UNDEFINED,
           *result    = H_UNDEFINED;

    target = vm_exec( vm, frame, node->value.m_switch );

    // exec case labels
    for( case_item = node->m_children.head; case_item; case_item = case_item->next ){
    	stmt_item = case_item->next;

        stmt_node = ll_node( stmt_item );
        case_node = ll_node( case_item );

        if( case_node != H_UNDEFINED && stmt_node != H_UNDEFINED ){
            compare = vm_exec( vm, frame, case_node );

            vm_check_frame_exit(frame)

            if( ob_cmp( target, compare ) == 0 ){
                return vm_exec( vm, frame, stmt_node );
            }
        }
        /*
         * case_item += 2
         */
        case_item = case_item->next;
    }

    // exec default case
    if( node->value.m_default != H_UNDEFINED ){
        result = vm_exec( vm, frame, node->value.m_default );

        vm_check_frame_exit(frame)
    }

    return result;
}

INLINE Object *vm_exec_explode( vm_t *vm, vframe_t *frame, Node *node ){
	ll_item_t *llitem;
	Node   *expr  = H_UNDEFINED;
	Object *value = H_UNDEFINED;

	expr  = node->child(0);
	value = vm_exec( vm, frame, expr );

	size_t n_ids   = node->children() - 1,
		   n_items = ob_get_size(value),
		   n_end   = (n_ids > n_items ? n_items : n_ids),
		   i;

	/*
	 * Initialize all the identifiers with a <false>.
	 */
	for( i = 0, llitem = node->m_children.head; i < n_ids; ++i, llitem = llitem->next ){
		frame->add( (char *)ll_node(llitem)->value.m_identifier.c_str(),
					(Object *)gc_new_boolean(false) );
	}

	/*
	 * Fill initializers until the iterable object ends, leave
	 * the rest of them to <null>.
	 */
	Integer index(0);
	for( llitem = node->m_children.head; (unsigned)index.value < n_end; ++index.value, llitem = llitem->next ){
		frame->add( (char *)ll_node(llitem)->value.m_identifier.c_str(),
					 ob_cl_at( value, (Object *)&index ) );
	}

	return value;
}

INLINE Object *vm_exec_throw( vm_t *vm, vframe_t *frame, Node *node ){
	Object *exception = H_UNDEFINED;

	exception = vm_exec( vm, frame, node->child(0) );

	/*
	 * Make sure the exception object will not be freed until someone
	 * catches it or the program ends.
	 */
	gc_set_alive(exception);

	frame->state.set( Exception, exception );

	return exception;
}

INLINE Object *vm_exec_try_catch( vm_t *vm, vframe_t *frame, Node *node ){
	Node   *main_body    = node->value.m_try_block,
		   *ex_ident     = node->value.m_exp_id,
		   *catch_body   = node->value.m_catch_block,
		   *finally_body = node->value.m_finally_block;
	Object *exception    = H_UNDEFINED;

	vm_exec( vm, frame, main_body );

	if( frame->state.is(Exception) ){
		exception = frame->state.value;

		assert( exception != H_UNDEFINED );

		frame->add( (char *)ex_ident->value.m_identifier.c_str(), exception );

		frame->state.unset(Exception);

		vm_exec( vm, frame, catch_body );
	}

	if( finally_body != NULL ){
		vm_exec( vm, frame, finally_body );
	}

	return H_DEFAULT_RETURN;
}

INLINE Object *vm_exec_eostmt( vm_t *vm, vframe_t *frame, Node *node ){
    Object *res_1 = H_UNDEFINED,
           *res_2 = H_UNDEFINED;

    res_1 = vm_exec( vm, frame, node->child(0) );
    res_2 = vm_exec( vm, frame, node->child(1) );

    vm_check_frame_exit(frame)

    return res_2;
}

INLINE Object *vm_exec_array( vm_t *vm, vframe_t *frame, Node *node ){
	Object *v = (Object *)gc_new_vector();

	ll_foreach( &node->m_children, llitem ){
		ob_cl_push_reference( v, vm_exec( vm, frame, ll_node( llitem ) ) );
	}

	return v;
}

INLINE Object *vm_exec_map( vm_t *vm, vframe_t *frame, Node *node ){
	ll_item_t *key,
			  *val;
	Object 	  *m = (Object *)gc_new_map();


	for( key = node->m_children.head; key; key= key->next ){
		val = key->next;

		ob_cl_set_reference( m,
							 vm_exec( vm, frame, ll_node(key) ),
							 vm_exec( vm, frame, ll_node(val) ) );

		/*
		 * key += 2
		 */
		key = key->next;
	}

	return m;
}

INLINE Object *vm_exec_assign( vm_t *vm, vframe_t *frame, Node *node ){
    Object *object = H_UNDEFINED,
           *value  = H_UNDEFINED;
    Node   *lexpr  = node->child(0);

    /*
     * If the first child is an identifier, we are just defining
     * a new variable or assigning it a new value, nothing
     * complicated about it.
     */
    if( lexpr->type() == H_NT_IDENTIFIER ){
    	if( lexpr->value.m_identifier == "me" ){
    		hyb_error( H_ET_SYNTAX, "'me' is a reserved word" );
    	}

    	value  = vm_exec( vm, frame, node->child(1) );

    	vm_check_frame_exit(frame)

		object = frame->add( (char *)lexpr->value.m_identifier.c_str(), value );

		return object;
    }
    /*
     * If not, we evaluate the first node as a "owner->child->..." sequence,
     * just like the vm_exec_attribute_request handler.
     */
    else if( lexpr->type() == H_NT_ATTRIBUTE ){
    	Node *member    = lexpr,
			 *owner     = member->value.m_owner,
			 *attribute = member->value.m_member;

    	Object *obj = vm_exec( vm, frame, owner ),
    		   *value;

    	vm_check_frame_exit(frame)
    	/*
    	 * Prevent obj from being garbage collected.
    	 */
    	frame->push_tmp(obj);

		value = vm_exec( vm, frame, node->child(1) );

		frame->remove_tmp(obj);

    	vm_check_frame_exit(frame)

    	ob_set_attribute( obj, (char *)attribute->value.m_identifier.c_str(), value );

    	return obj;
    }
    else{
    	hyb_error( H_ET_SYNTAX, "Unexpected constant expression for = operator" );
    }
}

INLINE Object *vm_exec_uminus( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *result = H_UNDEFINED;

    o = vm_exec( vm, frame, node->child(0) );

    vm_check_frame_exit(frame)

    result = ob_uminus(o);

    return result;
}

INLINE Object *vm_exec_regex( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o      = H_UNDEFINED,
           *regexp = H_UNDEFINED,
           *result = H_UNDEFINED;

    o 	   = vm_exec( vm, frame, node->child(0) );
	regexp = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	result = ob_apply_regexp( o, regexp );

	return result;
}

INLINE Object *vm_exec_add( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_add( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_add( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_inplace_add( a, b );

	return a;
}

INLINE Object *vm_exec_sub( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_sub( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_sub( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_inplace_sub( a, b );

	return a;
}

INLINE Object *vm_exec_mul( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_mul( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_mul( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_inplace_mul( a, b );

	return a;
}

INLINE Object *vm_exec_div( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_div( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_div( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_inplace_div( a, b );

	return a;
}

INLINE Object *vm_exec_mod( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_mod( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_mod( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_inplace_mod( a, b );

	return a;
}

INLINE Object *vm_exec_inc( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = vm_exec( vm, frame, node->child(0) );

	vm_check_frame_exit(frame)

	return ob_increment(o);
}

INLINE Object *vm_exec_dec( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED;

    o = vm_exec( vm, frame, node->child(0) );

	vm_check_frame_exit(frame)

	return ob_decrement(o);
}

INLINE Object *vm_exec_xor( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_bw_xor( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_xor( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_bw_inplace_xor( a, b );

	return a;
}

INLINE Object *vm_exec_and( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_bw_and( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_and( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_bw_inplace_and( a, b );

	return a;
}

INLINE Object *vm_exec_or( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_bw_or( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_or( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_bw_inplace_or( a, b );

	return a;
}

INLINE Object *vm_exec_shiftl( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	if( frame->state.is(Exception) ){
		return frame->state.value;
	}

	vm_check_frame_exit(frame)

	c = ob_bw_lshift( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_shiftl( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_bw_inplace_lshift( a, b );

	return a;
}

INLINE Object *vm_exec_shiftr( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_bw_rshift( a, b );

	return c;
}

INLINE Object *vm_exec_inplace_shiftr( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	ob_bw_inplace_rshift( a, b );

	return a;
}

INLINE Object *vm_exec_fact( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = vm_exec( vm, frame, node->child(0) );

	vm_check_frame_exit(frame)

    r = ob_factorial(o);

    return r;
}

INLINE Object *vm_exec_not( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = vm_exec( vm, frame, node->child(0) );

	vm_check_frame_exit(frame)

   	r = ob_bw_not(o);

   	return r;
}

INLINE Object *vm_exec_lnot( vm_t *vm, vframe_t *frame, Node *node ){
    Object *o = H_UNDEFINED,
           *r = H_UNDEFINED;

    o = vm_exec( vm, frame, node->child(0) );

	vm_check_frame_exit(frame)

	r = ob_l_not(o);

	return r;
}

INLINE Object *vm_exec_less( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_less( a, b );

	return c;
}

INLINE Object *vm_exec_greater( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_greater( a, b );

	return c;
}

INLINE Object *vm_exec_ge( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_greater_or_same( a, b );

	return c;
}

INLINE Object *vm_exec_le( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_less_or_same( a, b );

	return c;
}

INLINE Object *vm_exec_ne( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_diff( a, b );

	return c;
}

INLINE Object *vm_exec_eq( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_same( a, b );

	return c;
}

INLINE Object *vm_exec_land( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_and( a, b );

	return c;
}

INLINE Object *vm_exec_lor( vm_t *vm, vframe_t *frame, Node *node ){
    Object *a = H_UNDEFINED,
           *b = H_UNDEFINED,
           *c = H_UNDEFINED;

    a = vm_exec( vm, frame, node->child(0) );
	b = vm_exec( vm, frame, node->child(1) );

	vm_check_frame_exit(frame)

	c = ob_l_or( a, b );

	return c;
}

