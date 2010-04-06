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

HYBRIS_DEFINE_FUNCTION(hvar_names);
HYBRIS_DEFINE_FUNCTION(hvar_values);
HYBRIS_DEFINE_FUNCTION(huser_functions);
HYBRIS_DEFINE_FUNCTION(hdyn_functions);
HYBRIS_DEFINE_FUNCTION(hcall);

extern "C" named_function_t hybris_module_functions[] = {
	{ "var_names", hvar_names },
	{ "var_values", hvar_values },
	{ "user_functions", huser_functions },
	{ "dyn_functions", hdyn_functions },
	{ "call", hcall },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hvar_names){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < ctx->vmem.size(); i++ ){
		array->push( new Object((char *)ctx->vmem.label(i)) );
	}
	return array;
}

HYBRIS_DEFINE_FUNCTION(hvar_values){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < ctx->vmem.size(); i++ ){
		array->push( ctx->vmem.at(i) );
	}
	return array;
}

HYBRIS_DEFINE_FUNCTION(huser_functions){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < ctx->vcode.size(); i++ ){
		array->push( new Object((char *)ctx->vcode.label(i)) );
	}
	return array;
}

HYBRIS_DEFINE_FUNCTION(hdyn_functions){
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

HYBRIS_DEFINE_FUNCTION(hcall){
	if( data->size() < 1 ){
		hyb_syntax_error( "function 'call' requires at least 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_STRING );

	Node *call  = new Node(H_NT_CALL);
    call->value.m_call = data->at(0)->value.m_string;
	if( data->size() > 1 ){
		unsigned int i;
		for( i = 1; i < data->size(); i++ ){
			switch( data->at(i)->type ){
				case H_OT_INT    : call->addChild( new ConstantNode(data->at(i)->value.m_integer) );                    break;
				case H_OT_FLOAT  : call->addChild( new ConstantNode(data->at(i)->value.m_double) );                  break;
				case H_OT_CHAR   : call->addChild( new ConstantNode(data->at(i)->value.m_char) );                   break;
				case H_OT_STRING : call->addChild( new ConstantNode((char *)data->at(i)->value.m_string.c_str()) ); break;

				default : hyb_generic_error( "type not supported for reflected call" );
			}
		}
	}

	Object *_return = ctx->engine->onFunctionCall( data, call, 0 );
	delete call;

	return _return;
}