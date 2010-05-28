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

/*
 * VM timer flags.
 */
#define VM_TIMER_START 1
#define VM_TIMER_STOP  0
/*
 * Macro to declare a Hybris dynamic function.
 */
#define HYBRIS_DEFINE_FUNCTION(name) Object *name( vm_t *vm, vmem_t *data )
/*
 * Macro to define a constant value.
 */
#define HYBRIS_DEFINE_CONSTANT( vm, name, value ) vm_define_constant( vm, (char *)name, (Object *)value )
/*
 * Macro to define a new structure type given its name and its attribute names
 */
#define HYBRIS_DEFINE_STRUCTURE( vm, name, n, attrs ) vm_define_structure( vm, name, n, attrs )
/*
 * Macro to easily define allowed argument number.
 */
#define H_REQ_ARGC(...) { __VA_ARGS__, -1 }
/*
 * Only -1, any argument number is allowed.
 */
#define H_ANY_ARGC       H_REQ_ARGC(-1)
/*
 * No argument needed.
 */
#define H_NO_ARGS		 H_REQ_ARGC(0)
/*
 * Macro to easily define single argument allowed types.
 */
#define H_REQ_TYPES(...) { __VA_ARGS__, otEndMarker }
/*
 * Only otEndMarker, any type is allowed.
 */
#define H_ANY_TYPE  	 H_REQ_TYPES(otEndMarker)
/*
 * Macro to define module exported functions structure.
 */
#define HYBRIS_EXPORTED_FUNCTIONS() extern "C" vm_function_t hybris_module_functions[] =
/*
 * Macro to easily access hybris functions parameters.
 */
#define vm_argv(i)    ((*data)[i])
/*
 * Macro to easily access hybris functions parameters number.
 */
#define vm_argc()     (data->size())
/*
 * Pre declaration of structure vm_t.
 */
typedef struct _vm_t vm_t;
/*
 * Module initializer function pointer prototype.
 */
typedef void     (*initializer_t)( vm_t * );
/*
 * Generic function pointer prototype.
 */
typedef Object * (*function_t)( vm_t *, vmem_t * );

typedef struct _vm_function_t {
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
vm_function_t;

/*
 * Split 'str' into 'tokens' vector using 'delimiters'.
 */
void vm_str_split( string& str, string delimiters, vector<string>& tokens );

/*
 * Module structure definition.
 */
typedef struct vm_module {
	void	      *handle;
    vector<string> domains;
    string         name;
    initializer_t  initializer;
    llist_t		   functions;

    vm_module( string& module_name, string& module_path, void *ptr, initializer_t init ) :
    	name(module_name),
    	handle(ptr),
    	initializer(init){

    	/*
    	 * Initialize functions linked list.
    	 */
    	ll_init(&functions);

    	/*
    	 * Split the path with '/' token and append each item
    	 * to the domains vector.
    	 */
    	vm_str_split( module_path, "/", domains );
    }
}
vm_module_t;

typedef llist_t				      	  vm_modules_t;
typedef ITree<vm_function_t> 	  	  vm_mcache_t;
typedef ITree<pcre>					  vm_pcache_t;
typedef llist_t		 			  	  vm_scope_t;
typedef map< pthread_t, vm_scope_t *> vm_thread_scope_t;

enum vm_state_t {
	vmNone    = 0,
	vmParsing,
	vmExecuting
};

typedef struct _vm_t {
	/*
	 * Main argc and argv references.
	 */
	int	   *argc;
	char ***argv;
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
	vm_args_t       args;
	/*
	 * Functions lookup hashtable cache.
	 */
	vm_mcache_t	   mcache;
	/*
	 * Lookup cache mutex.
	 */
	pthread_mutex_t mcache_mutex;
	/*
	 * Dynamically loaded modules instances.
	 */
	vm_modules_t    modules;
	/*
	 * Compiled regular expressions cache.
	 */
	vm_pcache_t  pcre_cache;
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
void 	    vm_init( vm_t *vm, int optind, int *argc, char **argv[], char *envp[] );
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
 * Increment current line number.
 */
#define vm_inc_lineno( vm ) vm_line_lock(vm); \
						    ++vm->lineno; \
						    vm_line_unlock(vm)
/*
 * Return current line number.
 */
INLINE size_t vm_get_lineno( vm_t *vm ){
	return vm->lineno;
}
/*
 * Add a thread to the threads pool.
 */
INLINE vm_scope_t *vm_pool( vm_t *vm, pthread_t tid = 0 ){
	tid = (tid == 0 ? pthread_self() : tid);
	vm_mm_lock(vm);
		vm_scope_t *scope = (vm_scope_t *)calloc( 1, sizeof(vm_scope_t) );
		vm->th_frames[tid] = scope;
	vm_mm_unlock(vm);

	return scope;
}
/*
 * Remove a thread from the threads pool.
 */
INLINE void vm_depool( vm_t *vm, pthread_t tid = 0 ){
	tid = (tid == 0 ? pthread_self() : tid);
	vm_mm_lock( vm );
	vm_thread_scope_t::iterator i_scope = vm->th_frames.find(tid);
	if( i_scope != vm->th_frames.end() ){

		ll_clear( i_scope->second );
		free( i_scope->second );

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
								  ll_append( vm_find_scope(vm), frame ); \
								  vm_mm_unlock( vm )
/*
 * Remove the last frame from the trace stack.
 */
#define vm_pop_frame( vm ) vm_scope_t *scope = vm_find_scope(vm); \
						   vm_mm_lock( vm ); \
						   ll_pop(scope); \
						   vm_mm_unlock( vm )

#define vm_scope_size( vm ) vm_find_scope(vm)->items

/*
 * Return the active frame pointer (last in the list).
 */
#define vm_frame( vm ) ( (vframe_t *)ll_back( vm_find_scope(vm) ) )

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
INLINE vm_function_t *vm_get_function( vm_t *vm, char *identifier ){
	vm_module_t   *module;
	vm_function_t *function;

	/*
	 * First check if it's already cached.
	 */
	if( (function = vm->mcache.find(identifier)) != H_UNDEFINED ){
		return function;
	}
	/*
	 * Search it in dynamic loaded modules.
	 */
	ll_foreach( &vm->modules, m_item ){
		module = ll_data( vm_module_t *, m_item );
		/*
		 * For each function of the module.
		 */
		ll_foreach( &module->functions, f_item ){
			function = ll_data( vm_function_t *, f_item );
			/*
			 * Found it, add to the cache and return.
			 */
			if( function->identifier == identifier ){
				vm_mcache_lock( vm );
					vm->mcache.insert( identifier, function );
				vm_mcache_unlock( vm );

				return function;
			}
		}
	}
	/*
	 * Nothing found, function not defined, nor cached.
	 */
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
INLINE void vm_define_constant( vm_t *vm, char *name, Object *value ){
  /*
   * Prevent the structure or class definition from being deleted by the gc.
   */
   vm->vtypes.addConstant( name, value );
}

#define vm_define_type vm_define_constant

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

#define vm_check_frame_exit(frame) if( frame->state.is(Exception) ){ \
									   return frame->state.e_value; \
								   } \
								   else if( frame->state.is(Return) ){ \
									   return frame->state.r_value; \
								   }
/*
 * Methods to initialize a stack given its owner, arguments, identifiers
 * and so on.
 *
 * Each prepare_stack method will increment by one the reference counter
 * of each value pushed/inserted into it, then the dismiss_stack method
 * will decrement them again.
 */
void 	  vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, Object *cobj, int argc, Node *prototype, Node *argv );
void 	  vm_prepare_stack( vm_t *vm, vframe_t &stack, string owner, Object *cobj, Node *ids, int argc, ... );
void 	  vm_prepare_stack( vm_t *vm, vframe_t &stack, string owner, vector<string> ids, vmem_t *argv );
void 	  vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, vector<string> ids, Node *argv );
void 	  vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, Extern *fn_pointer, Node *argv );
void 	  vm_prepare_stack( vm_t *vm, vframe_t *root, vframe_t &stack, string owner, Node *argv );
void 	  vm_prepare_stack( vm_t *vm, vframe_t *root, vm_function_t *function, vframe_t &stack, string owner, Node *argv );
void 	  vm_prepare_stack( vm_t *vm, vframe_t *root, Node *function, vframe_t &stack, string owner, Node *argv );
void 	  vm_dismiss_stack( vm_t *vm );
/*
 * Handle hybris builtin function call.
 */
Object   *vm_exec_builtin_function_call( vm_t *vm, vframe_t *, Node * );
/*
 * Handle user defined function call.
 */
Object   *vm_exec_user_function_call( vm_t *vm, vframe_t *, Node * );
/*
 * Handle dynamic loaded function call.
 */
Object   *vm_exec_dll_function_call( vm_t *vm, vframe_t *, Node * );
/*
 * Special case to handle threaded function calls by name and by alias.
 */
Object   *vm_exec_threaded_call( vm_t *vm, string function_name, vmem_t *argv );
Object   *vm_exec_threaded_call( vm_t *vm, Node *function, vframe_t *frame, vmem_t *argv );
/*
 * Node handler dispatcher.
 */
Object 	 *vm_exec( vm_t *vm, vframe_t *frame, Node *node );
/*
 * Identifier found, do a memory lookup for it.
 */
Object 	 *vm_exec_identifier( vm_t *vm, vframe_t *, Node * );
/*
 * expression->expression, evaluate first expression to find out if
 * it's a class or a structure, and then lookup inside it the second one.
 */
Object   *vm_exec_attribute_request( vm_t *vm, vframe_t *, Node * );
/*
 * expression->expression(...), evaluate first expression to find out if
 * it's a class or a structure, and then lookup inside it the second one.
 */
Object   *vm_exec_method_call( vm_t *vm, vframe_t *, Node * );
/*
 * Constant found, just return it from the evaluation tree.
 */
Object   *vm_exec_constant( vm_t *vm, vframe_t *, Node * );
/*
 * Create an array.
 */
Object   *vm_exec_array( vm_t *vm, vframe_t *, Node * );
/*
 * Create a map.
 */
Object   *vm_exec_map( vm_t *vm, vframe_t *, Node * );
/*
 * Function declaration, add it to the code segment.
 */
Object   *vm_exec_function_declaration( vm_t *vm, vframe_t *, Node * );
/*
 * Structure declaration, create the prototype instance and add it
 * to the types segment.
 */
Object   *vm_exec_structure_declaration( vm_t *vm, vframe_t *, Node * );
/*
 * Class declaration, create the prototype instance, and add it
 * to the types segment.
 */
Object   *vm_exec_class_declaration( vm_t *vm, vframe_t *, Node * );
/*
 * expression( ... ), evaluate each argument and the run the function.
 */
Object   *vm_exec_function_call( vm_t *vm, vframe_t *, Node * );
/*
 * new type( ... ), execute type lookup, clone the instance and if it's
 * a class and has a constructor, execute it, otherwise do default attributes
 * initialization.
 */
Object   *vm_exec_new_operator( vm_t *vm, vframe_t *, Node * );
/*
 * Statements.
 */
Object   *vm_exec_explode( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_return( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_throw( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_try_catch( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_while( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_do( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_for( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_foreach( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_foreach_mapping( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_unless( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_if( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_question( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_switch( vm_t *vm, vframe_t *, Node * );
/*
 * Expressions and operators.
 */
/* misc */
Object 	 *vm_exec_backtick( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_vargs( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_dollar( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_reference( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_range( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_subscript_push( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_subscript_get( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_subscript_set( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_eostmt( vm_t *vm, vframe_t *, Node * );
/* arithmetic */
Object   *vm_exec_assign( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_uminus( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_regex( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_add( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_add( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_sub( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_sub( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_mul( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_mul( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_div( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_div( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_mod( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_mod( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inc( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_dec( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_fact( vm_t *vm, vframe_t *, Node * );
/* bitwise */
Object   *vm_exec_xor( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_xor( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_and( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_and( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_or( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_or( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_shiftl( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_shiftl( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_shiftr( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_inplace_shiftr( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_not( vm_t *vm, vframe_t *, Node * );
/* logic */
Object   *vm_exec_lnot( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_less( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_greater( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_ge( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_le( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_ne( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_eq( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_land( vm_t *vm, vframe_t *, Node * );
Object   *vm_exec_lor( vm_t *vm, vframe_t *, Node * );

#endif
