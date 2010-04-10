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
    vmem_t      *data;
    Context *ctx;
}
thread_args_t;

void * hyb_pthread_worker( void *arg ){
    thread_args_t *args = (thread_args_t *)arg;
    Context       *ctx  = args->ctx;
    vmem_t        *data = args->data;

    ctx->pool();

    Node *call         = new Node(H_NT_CALL);
    call->value.m_call = (const char *)(*HYB_ARGV(0));

	if( HYB_ARGC() > 1 ){
		unsigned int i;
		for( i = 1; i < data->size(); i++ ){
			switch( HYB_ARGV(i)->type ){
				case H_OT_INT    : call->addChild( new ConstantNode( (long)(*HYB_ARGV(i)) ) );   break;
				case H_OT_FLOAT  : call->addChild( new ConstantNode( (double)(*HYB_ARGV(i)) ) ); break;
				case H_OT_CHAR   : call->addChild( new ConstantNode( (char)(*HYB_ARGV(i)) ) );   break;
				case H_OT_STRING : call->addChild( new ConstantNode( (char *)(*HYB_ARGV(i)) ) ); break;
				default :
                    ctx->depool();
                    hyb_throw( H_ET_GENERIC, "type %d not supported for pthread call", Object::type_name(HYB_ARGV(i)) );
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
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

    pthread_t tid;
    thread_args_t *args = new thread_args_t;

    args->data = data->clone();
    args->ctx  = ctx;
    pthread_create( &tid, NULL, hyb_pthread_worker, (void *)args );

    return MK_INT_OBJ(tid);
}

HYBRIS_DEFINE_FUNCTION(hpthread_exit){
    ctx->depool();

	pthread_exit(NULL);
    return NULL;
}

HYBRIS_DEFINE_FUNCTION(hpthread_join){
    if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'pthread_join' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );

    pthread_t tid = static_cast<pthread_t>( (long)(*HYB_ARGV(0)) );
    void *status;

    pthread_join( tid, &status );

    ctx->depool();

    return NULL;
}

