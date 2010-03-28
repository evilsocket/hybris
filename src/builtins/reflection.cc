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
#include "builtin.h"
#include "tree.h"
#include "lexer.h"

extern Object *htree_function_call( h_context_t *ctx, vmem_t *stackframe, Node *call, int threaded );

HYBRIS_BUILTIN(hvar_names){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < ctx->vmem.size(); i++ ){
		array->push( new Object((char *)ctx->vmem.label(i)) );
	}
	return array;
}

HYBRIS_BUILTIN(hvar_values){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < ctx->vmem.size(); i++ ){
		array->push( ctx->vmem.at(i) );
	}
	return array;
}

HYBRIS_BUILTIN(huser_functions){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < ctx->vcode.size(); i++ ){
		array->push( new Object((char *)ctx->vcode.label(i)) );
	}
	return array;
}

HYBRIS_BUILTIN(hcore_functions){
	unsigned int i = 0;
	Object *array = new Object();

    for( i = 0; i < ctx->builtins.size(); i++ ){
        array->push( new Object((char *)ctx->builtins.label(i)) );
    }

	return array;
}

HYBRIS_BUILTIN(hdyn_functions){
    unsigned int i, j, mods = ctx->modules.size(), dyns;

    Object *map = new Object();
    for( i = 0; i < mods; i++ ){
        module_t *mod = ctx->modules[i];
        Object   *dyn = new Object();
        dyns          = mod->functions.size();
        for( j = 0; j < dyns; j++ ){
            dyn->push( new Object( (char *)mod->functions[j]->identifier.c_str() ) );
        }
        map->map( new Object( (char *)mod->name.c_str() ), dyn );
    }
    return map;
}

HYBRIS_BUILTIN(hcall){
	if( data->size() < 1 ){
		hybris_syntax_error( "function 'call' requires at least 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

	Node *call  = new Node(H_NT_CALL);
    call->_call = data->at(0)->xstring;
	if( data->size() > 1 ){
		unsigned int i;
		for( i = 1; i < data->size(); i++ ){
			switch( data->at(i)->xtype ){
				case H_OT_INT    : call->addChild( Tree::addInt(data->at(i)->xint) ); break;
				case H_OT_FLOAT  : call->addChild( Tree::addFloat(data->at(i)->xfloat) ); break;
				case H_OT_CHAR   : call->addChild( Tree::addChar(data->at(i)->xchar) ); break;
				case H_OT_STRING : call->addChild( Tree::addString((char *)data->at(i)->xstring.c_str()) ); break;
				default : hybris_generic_error( "type not supported for reflected call" );
			}
		}
	}

	Object *_return = ctx->executor->onFunctionCall( data, call, 0 );
	delete call;

	return _return;
}
