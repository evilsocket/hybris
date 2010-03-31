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
#include "common.h"
#include "vmem.h"
#include "context.h"
#include "node.h"

typedef struct {
    vmem_t      *data;
    Context *ctx;
}
thread_args_t;

void * hybris_pthread_worker( void *arg ){
    thread_args_t *args = (thread_args_t *)arg;
    Context       *ctx  = args->ctx;
    vmem_t        *data = args->data;

    ctx->pool();

    Node *call  = new Node(H_NT_CALL);
    call->value.m_call = data->at(0)->xstring;
	if( data->size() > 1 ){
		unsigned int i;
		for( i = 1; i < data->size(); i++ ){
			switch( data->at(i)->xtype ){
				case H_OT_INT    : call->addChild( new ConstantNode(data->at(i)->xint) );                    break;
				case H_OT_FLOAT  : call->addChild( new ConstantNode(data->at(i)->xfloat) );                  break;
				case H_OT_CHAR   : call->addChild( new ConstantNode(data->at(i)->xchar) );                   break;
				case H_OT_STRING : call->addChild( new ConstantNode((char *)data->at(i)->xstring.c_str()) ); break;
				default :
                    ctx->depool();
                    hybris_generic_error( "type not supported for pthread call" );
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

HYBRIS_BUILTIN(hpthread_create){
	if( data->size() < 1 ){
		hybris_syntax_error( "function 'pthread_create' requires at least 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

    pthread_t tid;
    thread_args_t *args = new thread_args_t;

    args->data = data->clone();
    args->ctx  = ctx;
    pthread_create( &tid, NULL, hybris_pthread_worker, (void *)args );

    return new Object( static_cast<long>(tid) );
}

HYBRIS_BUILTIN(hpthread_exit){
    ctx->depool();

	pthread_exit(NULL);
    return NULL;
}

HYBRIS_BUILTIN(hpthread_join){
    if( data->size() < 1 ){
		hybris_syntax_error( "function 'pthread_join' requires at least 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );

    pthread_t tid = static_cast<pthread_t>( data->at(0)->xint );
    void *status;

    pthread_join( tid, &status );

    ctx->depool();

    return NULL;
}

