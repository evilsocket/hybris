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
HYBRIS_DEFINE_FUNCTION(hpthread_exit);
HYBRIS_DEFINE_FUNCTION(hpthread_join);
HYBRIS_DEFINE_FUNCTION(hpthread_kill);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "pthread_create",      hpthread_create, H_REQ_ARGC(1,2), { H_REQ_TYPES(otString), H_REQ_TYPES(otVector) } },
	{ "pthread_exit", 		 hpthread_exit,   H_NO_ARGS },
	{ "pthread_join", 		 hpthread_join,   H_REQ_ARGC(1),   { H_REQ_TYPES(otInteger) } },
	{ "pthread_kill", 		 hpthread_kill,   H_REQ_ARGC(2),   { H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "", NULL }
};

typedef struct {
	string  function;
    vmem_t *frame;
    vm_t   *vm;
}
thread_args_t;

static pthread_mutex_t __vm_sync_mutex = PTHREAD_MUTEX_INITIALIZER;

void * hyb_pthread_worker( void *arg ){
	/*
	 * This will cause the thread to wait until the main process
	 * finishes its job and releases the mutex.
	 */
	pthread_mutex_lock(&__vm_sync_mutex);
	pthread_mutex_unlock(&__vm_sync_mutex);

	thread_args_t *args = (thread_args_t *)arg;

	vm_exec_threaded_call( args->vm,
						   args->function,
						   args->frame );

    vm_depool( args->vm );

	delete args->frame;
	delete args;

    pthread_exit(NULL);
}

HYBRIS_DEFINE_FUNCTION(hpthread_create){
	Vector    *thread_argv = NULL;
	pthread_t tid;
	int       code, argc;
	Integer index(0);
	thread_args_t *args = new thread_args_t;

	vm_parse_argv( "sV", &args->function, &thread_argv );

	argc  		= (thread_argv ? ob_get_size( (Object *)thread_argv ) : 0);
	args->frame = new vmem_t;
	args->vm    = vm;

	for( ; index.value < argc; ++index.value ){
		args->frame->push( ob_cl_at( (Object *)thread_argv,
									 (Object *)&index )
						 );
	}

	/*
	 * Make sure the thread is not goin to start until we want
	 * it to start.
	 */
	pthread_mutex_lock(&__vm_sync_mutex);

    if( (code = pthread_create( &tid, NULL, hyb_pthread_worker, (void *)args )) == 0 ){
        /*
         * Create the memory scope for the thread.
         */
    	vm_scope_t *scope = vm_pool( vm, tid );
    	/*
    	 * Append the newly created frame.
    	 */
    	ll_append( scope, args->frame );
    	/*
    	 * Ok, it's safe to start the thread now.
    	 */
    	pthread_mutex_unlock(&__vm_sync_mutex);

    	return ob_dcast( gc_new_integer(tid) );
    }
    else{
    	pthread_mutex_unlock(&__vm_sync_mutex);

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
    long  tid;
    void *status;

    vm_parse_argv( "l", &tid );

    if( tid > 0 ){
    	pthread_join( tid, &status );
		return H_DEFAULT_RETURN;
    }
    else{
    	return H_DEFAULT_ERROR;
    }
}

HYBRIS_DEFINE_FUNCTION(hpthread_kill){
	long  tid,
		  sig;

	vm_parse_argv( "ll", &tid, &sig );

	if( tid > 0 ){
		return (Object *)gc_new_integer( pthread_kill( tid, sig ) );
	}
	else{
		return H_DEFAULT_ERROR;
	}
}

