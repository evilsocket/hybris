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
#include "types.h"
#include "memory.h"
#include "code.h"
#include "engine.h"

using std::string;
using std::vector;

typedef vector<string> vstr_t;

/* pre declaration of structure vm_t */
typedef struct _vm_t vm_t;
/* pre declaration of structure engine_t */
typedef struct _engine_t engine_t;

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

typedef struct _vm_t {
	/*
	 * Running threads pool vector, used to hard terminate remaining threads
	 * when the vm is released.
	 */
	vector<pthread_t> th_pool;
	/*
	 * Threads pool mutex.
	 */
	pthread_mutex_t   th_pool_mutex;
	/*
	 * The list of active memory frames.
	 */
	list<vframe_t *> frames;
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
	 * Code execution engine.
	 */
	engine_t      *engine;
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
#define vm_th_pool_lock( vm )   pthread_mutex_lock( &vm->th_pool_mutex )
#define vm_th_pool_unlock( vm ) pthread_mutex_unlock( &vm->th_pool_mutex )
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
 * Add a thread to the threads pool.
 */
__force_inline void vm_pool( vm_t *vm, pthread_t tid = 0 ){
	tid = (tid == 0 ? pthread_self() : tid);
	vm_th_pool_lock(vm);
		vm->th_pool.push_back(tid);
	vm_th_pool_unlock(vm);
}
/*
 * Remove a thread from the threads pool.
 */
__force_inline void vm_depool( vm_t *vm, pthread_t tid = 0 ){
	tid = (tid == 0 ? pthread_self() : tid);
	int size( vm->th_pool.size() );
	vm_th_pool_lock( vm );
	for( int pool_i = 0; pool_i < size; ++pool_i ){
		if( vm->th_pool[pool_i] == tid ){
			vm->th_pool.erase( vm->th_pool.begin() + pool_i );
			break;
		}
	}
	vm_th_pool_unlock( vm );
}

/*
 * Push a frame to the trace stack.
 */
#define vm_add_frame( vm, frame ) vm_mm_lock( vm ); \
								  vm->frames.push_back(frame); \
								  vm_mm_unlock( vm )
/*
 * Remove the last frame from the trace stack.
 */
#define vm_pop_frame( vm ) vm_mm_lock( vm ); \
						   vm->frames.pop_back(); \
						   vm_mm_unlock( vm )
/*
 * Set the active frame (from threaded calls).
 */
#define vm_set_frame( vm, frame ) vm_mm_lock( vm ); \
								  vm->frames.pop_back(); \
								  vm->frames.push_back(frame); \
								  vm_mm_unlock( vm )
/*
 * Return the active frame pointer (last in the list).
 */
#define vm_frame( vm ) vm->frames.back()
/*
 * Compute execution time and print it.
 */
__force_inline void vm_timer( vm_t *vm, int start = 0 ){
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
__force_inline named_function_t *vm_get_function( vm_t *vm, char *identifier ){
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
				// fix issue #0000014
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
__force_inline Object * vm_define_structure( vm_t *vm, char *name, int nattrs, char *attributes[] ){
   StructureObject *type = new StructureObject();
   unsigned int     i;

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
__force_inline void vm_define_type( vm_t *vm, char *name, Object *type ){
  /*
   * Prevent the structure or class definition from being deleted by the gc.
   */
   vm->vtypes.addConstant( name, type );
}
/*
 * Find the object pointer of a user defined type (i.e. structures or classes).
 */
__force_inline Object * vm_get_type( vm_t *vm, char *name ){
   return vm->vtypes.find(name);
}
/*
 * Compile a regular expression and put it in a global cache.
 */
__force_inline pcre *vm_pcre_compile( vm_t *vm, string& pattern, int opts, const char **perror, int *poffset ){
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

#endif
