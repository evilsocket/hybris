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

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "var_names", hvar_names },
	{ "var_values", hvar_values },
	{ "user_functions", huser_functions },
	{ "dyn_functions", hdyn_functions },
	{ "call", hcall },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hvar_names){
	unsigned int i;
	VectorObject *array = gc_new_vector();
	for( i = 0; i < ctx->vmem.size(); ++i ){
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(ctx->vmem.label(i)) ) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(hvar_values){
	unsigned int i;
	VectorObject *array = gc_new_vector();
	for( i = 0; i < ctx->vmem.size(); ++i ){
		ob_cl_push( ob_dcast(array), ctx->vmem.at(i) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(huser_functions){
	unsigned int i;
	VectorObject *array = gc_new_vector();
	for( i = 0; i < ctx->vcode.size(); ++i ){
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(ctx->vcode.label(i)) ) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(hdyn_functions){
    unsigned int i, j, mods = ctx->modules.size(), dyns;

    Object *map = ob_dcast( gc_new_map() );
    for( i = 0; i < mods; ++i ){
        module_t *mod = ctx->modules[i];
        Object   *dyn = ob_dcast( gc_new_vector() );
        dyns          = mod->functions.size();
        for( j = 0; j < dyns; ++j ){
        	ob_cl_push_reference( dyn, ob_dcast( gc_new_string(mod->functions[j]->identifier.c_str()) ) );
        }

        ob_cl_set_reference( map, ob_dcast( gc_new_string( mod->name.c_str() ) ), dyn );
    }
    return map;
}

HYBRIS_DEFINE_FUNCTION(hcall){
	if( ob_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "function 'call' requires at least 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

	Node *call         = new Node(H_NT_CALL);
    call->value.m_call = string_argv(0);
	if( ob_argc() > 1 ){
		unsigned int i;
		for( i = 1; i < ob_argc(); ++i ){
			switch( ob_argv(i)->type->code ){
				case otInteger : call->addChild( new ConstantNode( int_argv(i) ) );   break;
				case otFloat   : call->addChild( new ConstantNode( float_argv(i) ) ); break;
				case otChar    : call->addChild( new ConstantNode( char_argv(i) ) );   break;
				case otString  : call->addChild( new ConstantNode( (char *)string_argv(i).c_str() ) ); break;

				default :
                    hyb_error( H_ET_GENERIC, "type %s not supported for reflected call", ob_argv(i)->type->name );
			}
		}
	}

	Object *_return = ctx->engine->onFunctionCall( data, call, 0 );
	delete call;

	return _return;
}
