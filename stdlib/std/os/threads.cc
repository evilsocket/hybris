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
#include <hybris.h>
#include <errno.h>

HYBRIS_DEFINE_FUNCTION(hpthread_create);
HYBRIS_DEFINE_FUNCTION(hpthread_create_argv);
HYBRIS_DEFINE_FUNCTION(hpthread_exit);
HYBRIS_DEFINE_FUNCTION(hpthread_join);
HYBRIS_DEFINE_FUNCTION(hpthread_kill);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "pthread_create",      hpthread_create,      H_REQ_ARGC(1), { H_REQ_TYPES(otString) } },
	{ "pthread_create_argv", hpthread_create_argv, H_REQ_ARGC(2), { H_REQ_TYPES(otString), H_REQ_TYPES(otVector) } },
	{ "pthread_exit", 		 hpthread_exit,        H_NO_ARGS },
	{ "pthread_join", 		 hpthread_join,        H_REQ_ARGC(1), { H_REQ_TYPES(otInteger) } },
	{ "pthread_kill", 		 hpthread_kill,        H_REQ_ARGC(2), { H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "", NULL }
};

typedef struct {
    vmem_t *data;
    vm_t   *vm;
}
thread_args_t;

void * hyb_pthread_worker( void *arg ){
    thread_args_t *args = (thread_args_t *)arg;
    vm_t      	  *vm  = args->vm;
    vmem_t        *data = args->data,
				   stack;

    vm_pool( vm );

    if( ob_argc() > 1 ){
		unsigned int i;
		for( i = 1; i < ob_argc(); ++i ){
			stack.push( ob_argv(i) );
		}
	}

    engine_on_threaded_call( vm->engine, string_argv(0), data, &stack );

    delete args;

    vm_depool( vm );

    pthread_exit(NULL);
}

HYBRIS_DEFINE_FUNCTION(hpthread_create){
	pthread_t tid;
    int       code;

    thread_args_t *args = new thread_args_t;

    args->data = data->clone();
    args->vm  = vm;

    if( (code = pthread_create( &tid, NULL, hyb_pthread_worker, (void *)args )) == 0 ){
    	return ob_dcast( gc_new_integer(tid) );
    }
    else{
    	switch( code ){
			case EAGAIN :
				hyb_error( H_ET_WARNING, "The system lacked the necessary resources to create another thread, or the system-imposed "
										 "limit on the total number of threads in a process PTHREAD_THREADS_MAX would be exceeded" );
			break;

			case EINVAL :
				hyb_error( H_ET_WARNING, "Invalid attribute value for pthread_create" );
			break;

			case EPERM  :
				hyb_error( H_ET_WARNING, "The caller does not have appropriate permission to set the required scheduling parameters or scheduling policy" );
			break;

			default :
				hyb_error( H_ET_WARNING, "Unknown system error while creating the thread" );
    	}

    	return H_DEFAULT_ERROR;
    }
}

HYBRIS_DEFINE_FUNCTION(hpthread_create_argv){
	pthread_t tid;
    int       i, code, argc( ob_get_size( ob_argv(1) ) );
    thread_args_t *args = new thread_args_t;

    args->data 	   = new vmem_t;
    args->vm = vm;

	args->data->push( ob_argv(0) );
    for( i = 0; i < argc; ++i ){
    	args->data->push( ((VectorObject *)ob_argv(1))->value[i] );
    }

    if( (code = pthread_create( &tid, NULL, hyb_pthread_worker, (void *)args )) == 0 ){
    	return ob_dcast( gc_new_integer(tid) );
    }
    else{
    	switch( code ){
			case EAGAIN :
				hyb_error( H_ET_WARNING, "The system lacked the necessary resources to create another thread, or the system-imposed "
										 "limit on the total number of threads in a process PTHREAD_THREADS_MAX would be exceeded" );
			break;

			case EINVAL :
				hyb_error( H_ET_WARNING, "Invalid attribute value for pthread_create" );
			break;

			case EPERM  :
				hyb_error( H_ET_WARNING, "The caller does not have appropriate permission to set the required scheduling parameters or scheduling policy" );
			break;

			default :
				hyb_error( H_ET_WARNING, "Unknown system error while creating the thread" );
    	}

    	return H_DEFAULT_ERROR;
    }
}

HYBRIS_DEFINE_FUNCTION(hpthread_exit){
	vm_depool( vm );

	pthread_exit(NULL);
    return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hpthread_join){
    pthread_t tid = static_cast<pthread_t>( int_argv(0) );
    void *status;

    // fix issue #0000014
    if( tid > 0 ){
    	pthread_join( tid, &status );
		return H_DEFAULT_RETURN;
    }
    else{
    	return H_DEFAULT_ERROR;
    }
}

HYBRIS_DEFINE_FUNCTION(hpthread_kill){
	if( int_argv(0) > 0 ){
		return (Object *)gc_new_integer( pthread_kill( int_argv(0), int_argv(1) ) );
	}
	else{
		return H_DEFAULT_ERROR;
	}
}

