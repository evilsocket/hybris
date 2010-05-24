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
#ifndef _HVM_H_
#   define _HVM_H_

#include <vector>
#include <string>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <map>
#include "types.h"
#include "memory.h"
#include "code.h"

using std::string;
using std::vector;
using std::map;

typedef vector<string> vstr_t;

/* pre declaration of structure vm_t */
typedef struct _vm_t vm_t;

/* initializer function pointer prototype */
typedef void     (*initializer_t)( vm_t * );
/* function pointer prototype */
typedef Object * (*function_t)( vm_t *, vmem_t * );

/* macro to declare a hybris function */
#define HYBRIS_DEFINE_FUNCTION(name) Object *name( vm_t *vm, vmem_t *data )
/* macro to define a constant value */
#define HYBRIS_DEFINE_CONSTANT( vm, name, value ) vm->vconst.addConstant( (char *)name, (Object *)value )
/* macro to define a new structure type given its name and its attribute names */
#define HYBRIS_DEFINE_STRUCTURE( vm, name, n, attrs ) vm_define_structure( vm, name, n, attrs )
/* macro to easily define allowed argument number */
#define H_REQ_ARGC(...) { __VA_ARGS__, -1 }
/* only -1, any argument number is allowed */
#define H_ANY_ARGC       H_REQ_ARGC(-1)
/* no argument needed */
#define H_NO_ARGS		 H_REQ_ARGC(0)
/* macro to easily define single argument allowed types */
#define H_REQ_TYPES(...) { __VA_ARGS__, otEndMarker }
/* only otEndMarker, any type is allowed */
#define H_ANY_TYPE  	 H_REQ_TYPES(otEndMarker)
/* macro to define module exported functions structure */
#define HYBRIS_EXPORTED_FUNCTIONS() extern "C" named_function_t hybris_module_functions[] =
/* macro to easily access hybris functions parameters */
#define ob_argv(i)    ((*data)[i])
/* macro to easily access hybris functions parameters number */
#define ob_argc()     (data->size())
/* typed versions of ob_argv macro */
#define int_argv(i)    ob_int_val( ob_argv(i) )
#define alias_argv(i)  ob_alias_val( ob_argv(i) )
#define extern_argv(i) ob_extern_val( ob_argv(i) )
#define float_argv(i)  ob_float_val( ob_argv(i) )
#define char_argv(i)   ob_char_val( ob_argv(i) )
#define string_argv(i) ob_string_val( ob_argv(i) )
#define binary_argv(i) ob_binary_val( ob_argv(i) )
#define vector_argv(i) ob_vector_val( ob_argv(i) )
#define map_argv(i)    ob_map_val( ob_argv(i) )
#define struct_argv(i) ob_struct_val( ob_argv(i) )
#define class_argv(i)  ob_class_val( ob_argv(i) )
#define handle_argv(i) ob_handle_val( ob_argv(i) )

/* macros to assert an object type */
#define ob_type_assert(o,t,f)      if( o->type->code != t ){ \
                                    hyb_error( H_ET_SYNTAX, "Unexpected '%s' variable for function " f ", expected '%s'", o->type->name, ob_type_to_string(t) ); \
                                 }
#define ob_types_assert(o,t1,t2,f) if( o->type->code != t1 && o->type->code != t2 ){ \
									   hyb_error( H_ET_SYNTAX, "Unexpected '%s' variable for function " f ", expected '%s' or '%s'", o->type->name, ob_type_to_string(t1), ob_type_to_string(t2) ); \
                                   }

#define ob_argv_type_assert( i, t, f ) 		 ob_type_assert( ob_argv(i), t, f )
#define ob_argv_types_assert( i, t1, t2, f ) ob_types_assert( ob_argv(i), t1, t2, f )

#define HYB_TIMER_START 1
#define HYB_TIMER_STOP  0

typedef struct _named_function_t {
	/*
	 * Function identifier.
	 */
    string     	  identifier;
    /*
     * Function pointer.
     */
    function_t    function;
    /*
     * Number of arguments required.
     */
    int		      argc[HMAXARGS];
    /*
     * Allowed types of each argument.
     */
    H_OBJECT_TYPE types[HMAXARGS][20];
}
named_function_t;

typedef vector<named_function_t *> named_functions_t;

/* module structure definition */
typedef struct _module_t {
	void			 *handle;
    vstr_t            tree;
    string            name;
    initializer_t     initializer;
    named_functions_t functions;
}
module_t;

/* module initializer function pointer prototype */
typedef void (*initializer_t)( vm_t * );

typedef vector<module_t *>        h_modules_t;
typedef ITree<named_function_t> h_mcache_t;

enum vm_state_t {
	vmNone    = 0,
	vmParsing,
	vmExecuting
};

typedef list< vframe_t * > 			   vm_scope_t;
typedef map< pthread_t, vm_scope_t *> vm_thread_scope_t;

typedef struct _vm_t {
	/*
	 * Main thread id.
	 */
	pthread_t main_tid;
	/*
	 * The current state of the vm.
	 *
	 * NOTE : The state is set only by the main process, so
	 * it's safe to not have a mutex here.
	 */
	vm_state_t state;
	/*
	 * Current executing/parsing line number.
	 */
	size_t lineno;
	/*
	 * Mutex to lock to change lineno.
	 */
	pthread_mutex_t line_mutex;
	/*
	 * The list of active memory frames on the main thread.
	 */
	vm_scope_t frames;
	/*
	 * The list of active memory frames on other threads.
	 */
	vm_thread_scope_t th_frames;
	/*
	 * Frames list mutex.
	 */
	pthread_mutex_t  mm_pool_mutex;
	/*
	 * Source file handle
	 */
	FILE          *fp;
	/*
	 * Pointer to environment variables listing.
	 */
	char         **env;
	/*
	 * Constats memory segment.
	 */
	vmem_t         vconst;
	/*
	 * Main memory segment.
	 */
	vmem_t         vmem;
	/*
	 * Main code segment.
	 */
	vcode_t        vcode;
	/*
	 * Type definitions segment, where structures and classes
	 * prototypes are defined.
	 */
	vmem_t         vtypes;
	/*
	 * VM command line arguments.
	 */
	h_args_t       args;
	/*
	 * Functions lookup hashtable cache.
	 */
	h_mcache_t	   mcache;
	/*
	 * Lookup cache mutex.
	 */
	pthread_mutex_t mcache_mutex;
	/*
	 * Dynamically loaded modules instances.
	 */
	h_modules_t    modules;
	/*
	 * Compiled regular expressions cache.
	 */
	ITree<pcre>  pcre_cache;
	/*
	 * Regexp cache mutex.
	 */
	pthread_mutex_t pcre_mutex;
	/*
	 * Flag set to true when vm_release is called, used to prevent
	 * recursive calls when an error is triggered inside a class destructor.
	 */
	bool 		   releasing;
}
vm_t;


/*
 * Macros to lock and unlock the vm mutexes.
 */
#define vm_line_lock( vm )      pthread_mutex_lock( &vm->line_mutex )
#define vm_line_unlock( vm )    pthread_mutex_unlock( &vm->line_mutex )
#define vm_mm_lock( vm )  	    pthread_mutex_lock( &vm->mm_pool_mutex )
#define vm_mm_unlock( vm )   	pthread_mutex_unlock( &vm->mm_pool_mutex )
#define vm_mcache_lock( vm )    pthread_mutex_lock( &vm->mcache_mutex )
#define vm_mcache_unlock( vm )  pthread_mutex_unlock( &vm->mcache_mutex )
#define vm_pcre_lock( vm )      pthread_mutex_lock( &vm->pcre_mutex )
#define vm_pcre_unlock( vm )    pthread_mutex_unlock( &vm->pcre_mutex )

/*
 * Alloc a virtual machine instance.
 */
vm_t 	   *vm_create();
/*
 * Initialize the virtual machine attributes and global constants.
 */
void 	    vm_init( vm_t *vm, int argc, char *argv[], char *envp[] );
/*
 * Release the virtual machine content (free memory).
 */
void 		vm_release( vm_t *vm );
/*
 * Pretty self explainatory.
 */
#define     vm_free( vm ) delete vm
/*
 * Open the file given in 'args' structure by main, otherwise
 * return the stdin handler.
 */
FILE 	   *vm_fopen( vm_t *vm );
/*
 * If vm->fp != stdin close it.
 */
void 		vm_fclose( vm_t *vm );
/*
 * Change working directory to the script one.
 */
int 		vm_chdir( vm_t *vm );
/*
 * The main SIGSEGV handler to print the stack trace.
 */
static void vm_signal_handler( int signo );
/*
 * Split 'str' into 'tokens' vector using 'delimiters'.
 */
void 		vm_str_split( string& str, string delimiters, vector<string>& tokens );
/*
 * Load a .so module given its full path and name.
 */
void    	vm_load_module( vm_t *vm, string path, string name );
/*
 * Load a dynamic module given its namespace or load an entire
 * namespace tree if the '*' token is given.
 *
 * i.e. std.* will load the entire std modules tree.
 *
 */
void		vm_load_module( vm_t *vm, char *module );
/*
 * Handle an entire namespace modules loading.
 */
void   		vm_load_namespace( vm_t *vm, string path );
/*
 * Throw an exception inside the script, causing the active frame,
 * if any, to be set with an exception state.
 */
Object 	   *vm_raise_exception( const char *fmt, ... );
/*
 * Print the calling stack trace.
 */
void 		vm_print_stack_trace( vm_t *vm, bool force = false );
/*
 * Set current line number.
 */
#define vm_set_lineno( vm, line ) vm_line_lock(vm); \
								  vm->lineno = line; \
								  vm_line_unlock(vm)
/*
 * Return current line number accordingly to the vm state.
 */
INLINE size_t vm_get_lineno( vm_t *vm ){
	/*
	 * None state, no lines processed yet.
	 */
	if( vm->state == vmNone ){
		return 0;
	}
	/*
	 * Parsing state, return global line counter.
	 */
	else if( vm->state == vmParsing ){
		extern int yylineno;
		return yylineno;
	}
	/*
	 * Executing state, return inner line counter.
	 */
	else if( vm->state == vmExecuting ){
		return vm->lineno;
	}
	else{
		/*
		 * THIS SHOULD NEVER HAPPEN!
		 */
		assert(false);
	}
}
/*
 * Add a thread to the threads pool.
 */
INLINE void vm_pool( vm_t *vm, pthread_t tid = 0 ){
	tid = (tid == 0 ? pthread_self() : tid);
	vm_mm_lock(vm);
		vm->th_frames[tid] = new vm_scope_t;
	vm_mm_unlock(vm);
}
/*
 * Remove a thread from the threads pool.
 */
INLINE void vm_depool( vm_t *vm, pthread_t tid = 0 ){
	tid = (tid == 0 ? pthread_self() : tid);
	vm_mm_lock( vm );
	vm_thread_scope_t::iterator i_scope = vm->th_frames.find(tid);
	if( i_scope != vm->th_frames.end() ){
		delete i_scope->second;
		vm->th_frames.erase( i_scope );
	}
	vm_mm_unlock( vm );
}

INLINE vm_scope_t *vm_find_scope( vm_t *vm ){
	pthread_t tid = pthread_self();
	/*
	 * Main thread id, return main scope.
	 */
	if( tid == vm->main_tid ){
		return &vm->frames;
	}
	else{
		return vm->th_frames.find(tid)->second;
	}
}
/*
 * Push a frame to the trace stack.
 */
#define vm_add_frame( vm, frame ) vm_mm_lock( vm ); \
								  vm_find_scope(vm)->push_back(frame); \
								  vm_mm_unlock( vm )
/*
 * Remove the last frame from the trace stack.
 */
#define vm_pop_frame( vm ) vm_mm_lock( vm ); \
						   vm_find_scope(vm)->pop_back(); \
						   vm_mm_unlock( vm )

#define vm_scope_size( vm ) vm_find_scope(vm)->size()

/*
 * Return the active frame pointer (last in the list).
 */
#define vm_frame( vm ) vm_find_scope(vm)->back()

/*
 * Compute execution time and print it.
 */
INLINE void vm_timer( vm_t *vm, int start = 0 ){
    if( vm->args.tm_timer ){
        /* first call */
        if( vm->args.tm_start == 0 && start ){
            vm->args.tm_start = hyb_uticks();
        }
        /* last call */
        else if( !start ){
            vm->args.tm_end = hyb_uticks();
            char buffer[0xFF] = {0};
            hyb_timediff( vm->args.tm_end - vm->args.tm_start, buffer );
            fprintf( stdout, "\033[01;33m[TIME] Elapsed %s .\n\033[00m", buffer );
        }
    }
}
/*
 * Find out if a function has been registered by some previously
 * loaded module and return its pointer.
 * Handle function pointer caching.
 */
INLINE named_function_t *vm_get_function( vm_t *vm, char *identifier ){
	unsigned int i, j,
				 ndyns( vm->modules.size() ),
				 nfuncs;

	/* first check if it's already in cache */
	named_function_t * cache = vm->mcache.find(identifier);
	if( cache != H_UNDEFINED ){
		return cache;
	}

	/* search it in dynamic loaded modules */
	for( i = 0; i < ndyns; ++i ){
		/* for each function of the module */
		nfuncs = vm->modules[i]->functions.size();
		for( j = 0; j < nfuncs; ++j ){
			if( vm->modules[i]->functions[j]->identifier == identifier ){

				vm_mcache_lock( vm );

				/* found it, add to the cache and return */
				cache = vm->modules[i]->functions[j];
				vm->mcache.insert( identifier, cache );

				vm_mcache_unlock( vm );

				return cache;
			}
		}
	}

	return H_UNDEFINED;
}

/*
 * Define a structure inside the vtypes member.
 */
INLINE Object * vm_define_structure( vm_t *vm, char *name, size_t nattrs, char *attributes[] ){
   Structure *type = new Structure();
   size_t	        i;

   for( i = 0; i < nattrs; ++i ){
	   ob_add_attribute( (Object *)type, attributes[i] );
   }
  /*
   * Prevent the structure or class definition from being deleted by the gc.
   */
   return vm->vtypes.addConstant( name, (Object *)type );
}
/*
 * Same as before, but the structure or the class will be defined from an alreay
 * created object.
 */
INLINE void vm_define_type( vm_t *vm, char *name, Object *type ){
  /*
   * Prevent the structure or class definition from being deleted by the gc.
   */
   vm->vtypes.addConstant( name, type );
}
/*
 * Find the object pointer of a user defined type (i.e. structures or classes).
 */
INLINE Object * vm_get_type( vm_t *vm, char *name ){
   return vm->vtypes.find(name);
}
/*
 * Compile a regular expression and put it in a global cache.
 */
INLINE pcre *vm_pcre_compile( vm_t *vm, string& pattern, int opts, const char **perror, int *poffset ){
	pcre *compiled;
	/*
	 * Is it cached ?
	 */
	if( (compiled = vm->pcre_cache.find((char *)pattern.c_str())) == NULL ){
		/*
		 * Not cached, compile it.
		 */
		compiled = ::pcre_compile( pattern.c_str(), opts, perror, poffset, 0 );
		/*
		 * If it's a valid regex, put the compilation result to the cache.
		 */
		if( compiled ){
			vm_pcre_lock( vm );
				vm->pcre_cache.insert( (char *)pattern.c_str(), compiled );
			vm_pcre_unlock( vm );
		}
	}
	/*
	 * Return compiled regex.
	 */
	return compiled;
}
/*
 * Parse the frame to extract C-Type or Hybris-Type arguments accordingly to
 * given format.
 * See the implementation for more details about type formats.
 */
void vm_parse_frame_argv( vframe_t *argv, char *format, ... );
/*
 * Same as vm_parse_frame_argv, just easier to use with dynamic modules.
 */
#define vm_parse_argv( format, ... ) vm_parse_frame_argv( data, format, __VA_ARGS__ )

/*
 * Here starts the vm execution functions definition.
 */
/*
 * Max recursions or nested functions calls.
 */
#define VM_MAX_RECURSION 10000

#define vm_exec_break_state( frame ) frame->state.set(Break)
#define vm_exec_next_state( frame )  frame->state.set(Next)

#define vm_check_frame_exit(frame) if( frame->state.is(Exception) || frame->state.is(Return) ){ \
									   return frame->state.value; \
								   }
/*
 * Special case to handle threaded function calls by name and by alias.
 */
Object   *vm_exec_threaded_call( vm_t *vm, string function_name, vframe_t *frame, vmem_t *argv );
Object   *vm_exec_threaded_call( vm_t *vm, Node *function, vframe_t *frame, vmem_t *argv );
/*
 * Node handler dispatcher.
 */
Object 	 *vm_exec( vm_t *vm, vframe_t *frame, Node *node );


#endif
