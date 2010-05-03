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
#include "types.h"
#include "memory.h"
#include "code.h"
#include "engine.h"

using std::string;
using std::vector;

typedef vector<string> vstr_t;

/* pre declaration of class VM */
class VM;
/* pre declaration of class Engine */
class Engine;

/* initializer function pointer prototype */
typedef void     (*initializer_t)( VM * );
/* function pointer prototype */
typedef Object * (*function_t)( VM *, vmem_t * );

/* macro to declare a hybris function */
#define HYBRIS_DEFINE_FUNCTION(name) Object *name( VM *vmachine, vmem_t *data )
/* macro to define a constant value */
#define HYBRIS_DEFINE_CONSTANT( vmachine, name, value ) vmachine->vmem.addConstant( (char *)name, (Object *)value )
/* macro to define a new structure type given its name and its attribute names */
#define HYBRIS_DEFINE_STRUCTURE( vmachine, name, n, attrs ) vmachine->defineStructure( name, n, attrs )
/* macro to define module exported functions structure */
#define HYBRIS_EXPORTED_FUNCTIONS() extern "C" named_function_t hybris_module_functions[] =

/* macro to easily access hybris functions parameters */
#define ob_argv(i)    (data->at(i))
/* macro to easily access hybris functions parameters number */
#define ob_argc()     (data->size())
/* typed versions of ob_argv macro */
#define int_argv(i)    ob_int_val( data->at(i) )
#define alias_argv(i)  ob_alias_val( data->at(i) )
#define extern_argv(i) ob_extern_val( data->at(i) )
#define float_argv(i)  ob_float_val( data->at(i) )
#define char_argv(i)   ob_char_val( data->at(i) )
#define string_argv(i) ob_string_val( data->at(i) )
#define binary_argv(i) ob_binary_val( data->at(i) )
#define vector_argv(i) ob_vector_val( data->at(i) )
#define map_argv(i)    ob_map_val( data->at(i) )
#define matrix_argv(i) ob_matrix_val( data->at(i) )
#define struct_argv(i) ob_struct_val( data->at(i) )
#define class_argv(i)  ob_class_val( data->at(i) )

/* macros to assert an object type */
#define ob_type_assert(o,t)      if( !(o->type->code == t) ){ \
                                     hyb_error( H_ET_SYNTAX, "'%s' is not a valid variable type", o->type->name ); \
                                  }
#define ob_types_assert(o,t1,t2) if( !(o->type->code == t1) && !(o->type->code == t2) ){ \
                                     hyb_error( H_ET_SYNTAX, "'%s' is not a valid variable type", o->type->name ); \
                                  }

#define HYB_TIMER_START 1
#define HYB_TIMER_STOP  0

typedef struct _named_function_t {
    string     identifier;
    function_t function;

    _named_function_t( string i, function_t f ) :
        identifier(i),
        function(f){

    }
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
typedef void (*initializer_t)( VM * );

typedef vector<module_t *>        h_modules_t;
typedef HashMap<named_function_t> h_mcache_t;

class VM {
    private :
		/*
		 * The main SIGSEGV handler to print the stack trace.
		 */
        static void signal_handler( int signo );
        /*
         * Split 'str' into 'tokens' vector using 'delimiters'.
         */
        void   str_split( string& str, string delimiters, vector<string>& tokens );
        /*
         * Load a .so module given its full path and name.
         */
        void   loadModule( string path, string name );
        /*
         * Handle an entire namespace modules loading.
         */
        void   loadNamespace( string path );

    public  :

		/* running threads pool vector */
		vector<pthread_t> th_pool;
		/* threads mutex */
		pthread_mutex_t   th_mutex;

		/* lock the pthread mutex */
		__force_inline void lock(){
			pthread_mutex_lock( &th_mutex );
		}
		/* release the pthread mutex */
		__force_inline void unlock(){
			pthread_mutex_unlock( &th_mutex );
		}
		/* add a thread to the threads pool */
		__force_inline void pool( pthread_t tid = 0 ){
			tid = (tid == 0 ? pthread_self() : tid);
			lock();
			th_pool.push_back(tid);
			unlock();
		}
		/* remove a thread from the threads pool */
		__force_inline void depool( pthread_t tid = 0 ){
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

		/*
		 * The list of calling frames.
		 */
		list<vframe_t *> frames;
		/*
		 * Push a frame to the trace stack.
		 */
		__force_inline void addFrame( vframe_t *frame ){
			lock();
				frames.push_back(frame);
			unlock();
		}
		/*
		 * Set the active frame (from threaded calls).
		 */
		__force_inline void setFrame( vframe_t *frame ){
			popFrame();
			addFrame(frame);
		}
		/*
		 * Remove the last frame from the trace stack.
		 */
		__force_inline void popFrame(){
			lock();
				frames.pop_back();
			unlock();
		}
		/*
		 * Return the active frame pointer (last in the list).
		 */
		__force_inline vframe_t *frame(){
			return frames.back();
		}

        /* source file handle */
        FILE          *fp;
        /* pointer to environment variables listing */
        char         **env;
        /* data segment */
        vmem_t         vmem;
        /* active memory frame pointer */
        vmem_t		  *vframe;
        /* code segment */
        vcode_t        vcode;
        /* type definitions segment (structs and classes) */
        vmem_t         vtypes;
        /* execution arguments */
        h_args_t       args;
        /* functions lookup hashtable cache */
        h_mcache_t	   mcache;
        /* dynamically loaded modules */
        h_modules_t    modules;
        /* compiled regular expressions cache */
        HashMap<pcre>  pcre_cache;
        /* code execution engine */
        Engine        *engine;


        VM();
        /*
         * Open the file given in 'args' structure by main, otherwise
         * return the stdin handler.
         */
        FILE *openFile();
        /*
         * If this->fp != stdin close it.
         */
        void closeFile();
        /*
         * Change working directory to the script one.
         */
        int chdir();

        /*
         * Compute execution time and print it.
         */
        __force_inline void timer( int start = 0 ){
            if( args.tm_timer ){
                /* first call */
                if( args.tm_start == 0 && start ){
                    args.tm_start = hyb_uticks();
                }
                /* last call */
                else if( !start ){
                    args.tm_end = hyb_uticks();
                    char buffer[0xFF] = {0};
                    hyb_timediff( args.tm_end - args.tm_start, buffer );
                    fprintf( stdout, "\033[01;33m[TIME] Elapsed %s .\n\033[00m", buffer );
                }
            }
        }
        /*
         * Initialize the context attributes and global constants.
         */
        void init( int argc, char *argv[], char *envp[] );
        /*
         * Release the context (free memory).
         */
        void release();
        /*
         * Load a dynamic module given its namespace or load an entire
         * namespace tree if the '*' token is given.
         *
         * i.e. std.* will load the entire std modules tree.
         *
         */
        void loadModule( char *module );
        /*
         * Print the calling stack trace.
         */
        void printStackTrace( bool force = false );
        /*
         * Find out if a function has been registered by some previously
         * loaded module and return its pointer.
         * Handle function pointer caching.
         */
        __force_inline function_t getFunction( char *identifier ){
        	unsigned int i, j,
						 ndyns( modules.size() ),
						 nfuncs;

			/* first check if it's already in cache */
			named_function_t * cache = mcache.find(identifier);
			if( cache != H_UNDEFINED ){
				return cache->function;
			}

			/* search it in dynamic loaded modules */
			for( i = 0; i < ndyns; ++i ){
				/* for each function of the module */
				nfuncs = modules[i]->functions.size();
				for( j = 0; j < nfuncs; ++j ){
					if( modules[i]->functions[j]->identifier == identifier ){
						// fix issue #0000014
						lock();
						/* found it, add to the cache and return */
						cache = modules[i]->functions[j];
						mcache.insert( identifier, cache );

						unlock();

						return cache->function;
					}
				}
			}
			return H_UNDEFINED;
        }
        /*
         * Define a structure inside the vtypes member.
         */
        __force_inline Object * defineStructure( char *name, int nattrs, char *attributes[] ){
            StructureObject *type = new StructureObject();
            unsigned int     i;

            for( i = 0; i < nattrs; ++i ){
                ob_add_attribute( (Object *)type, attributes[i] );
            }
           /*
            * Prevent the structure or class definition from being deleted by the gc.
            */
            return vtypes.addConstant( name, (Object *)type );
        }
        /*
         * Same as before, but the structure or the class will be defined from an alreay
         * created object.
         */
        __force_inline void defineType( char *name, Object *type ){
           /*
            * Prevent the structure or class definition from being deleted by the gc.
            */
            vtypes.addConstant( name, type );
        }
        /*
         * Find the object pointer of a user defined type (i.e. structures or classes).
         */
        __force_inline Object * getType( char *name ){
            return vtypes.find(name);
        }
        /*
         * Compile a regular expression and put it in a global cache.
         */
		__force_inline pcre *pcre_compile( string& pattern, int opts, const char **perror, int *poffset ){
			pcre *compiled;
			if( (compiled = pcre_cache.find((char *)pattern.c_str())) == NULL ){
				compiled = ::pcre_compile( pattern.c_str(), opts, perror, poffset, 0 );
				if( compiled ){
					lock();
						pcre_cache.insert( (char *)pattern.c_str(), compiled );
					unlock();
				}
			}
			return compiled;
		}
};

#endif
