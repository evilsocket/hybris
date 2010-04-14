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

extern "C" named_function_t hybris_module_functions[] = {
	{ "pthread_create", hpthread_create },
	{ "pthread_exit", hpthread_exit },
	{ "pthread_join", hpthread_join },
	{ "", NULL }
};

typedef struct {
    vmem_t  *data;
    Context *ctx;
}
thread_args_t;

void * hyb_pthread_worker( void *arg ){
    thread_args_t *args = (thread_args_t *)arg;
    Context       *ctx  = args->ctx;
    vmem_t        *data = args->data;

    ctx->pool();

    Node *call         = new Node(H_NT_CALL);
    call->value.m_call = STRING_ARGV(0).c_str();

	if( HYB_ARGC() > 1 ){
		unsigned int i;
		for( i = 1; i < HYB_ARGC(); ++i ){
			switch( HYB_ARGV(i)->type->code ){
				case otInteger : call->addChild( new ConstantNode( INT_ARGV(i) ) );   break;
				case otFloat   : call->addChild( new ConstantNode( FLOAT_ARGV(i) ) ); break;
				case otChar    : call->addChild( new ConstantNode( CHAR_ARGV(i) ) );   break;
				case otString  : call->addChild( new ConstantNode( (char *)STRING_ARGV(i).c_str() ) ); break;
				default :
                    ctx->depool();
                    hyb_throw( H_ET_GENERIC, "type %d not supported for pthread call", HYB_ARGV(i)->type->name );
			}
		}
	}

	Object *_return = ctx->engine->onFunctionCall( data, call, 1 );
	delete call;
    delete _return;

    args->data->release();
    delete args;

    ctx->depool();

    pthread_exit(NULL);
}

HYBRIS_DEFINE_FUNCTION(hpthread_create){
	if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'pthread_create' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

    pthread_t tid;
    int       code;
    thread_args_t *args = new thread_args_t;

    args->data = data->clone();
    args->ctx  = ctx;

    if( (code = pthread_create( &tid, NULL, hyb_pthread_worker, (void *)args )) == 0 ){
    	return OB_DOWNCAST( MK_INT_OBJ(tid) );
    }
    else{
    	switch( code ){
			case EAGAIN :
				hyb_throw( H_ET_WARNING, "The system lacked the necessary resources to create another thread, or the system-imposed "
										 "limit on the total number of threads in a process PTHREAD_THREADS_MAX would be exceeded" );
			break;

			case EINVAL :
				hyb_throw( H_ET_WARNING, "Invalid attribute value for pthread_create" );
			break;

			case EPERM  :
				hyb_throw( H_ET_WARNING, "The caller does not have appropriate permission to set the required scheduling parameters or scheduling policy" );
			break;

			default :
				hyb_throw( H_ET_WARNING, "Unknown system error while creating the thread" );
    	}

    	return OB_DOWNCAST( MK_INT_OBJ(-1) );
    }
}

HYBRIS_DEFINE_FUNCTION(hpthread_exit){
    ctx->depool();

	pthread_exit(NULL);
    return OB_DOWNCAST( MK_INT_OBJ(0) );
}

HYBRIS_DEFINE_FUNCTION(hpthread_join){
    if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'pthread_join' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

    pthread_t tid = static_cast<pthread_t>( INT_ARGV(0) );
    void *status;

    // fix issue #0000014
    if( tid != -1 ){
    	pthread_join( tid, &status );
		ctx->depool();
		return OB_DOWNCAST( MK_INT_OBJ(0) );
    }
    else{
    	return OB_DOWNCAST( MK_INT_OBJ(-1) );
    }
}

