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
	VectorObject *array = MK_VECTOR_OBJ();
	for( i = 0; i < ctx->vmem.size(); ++i ){
		ob_cl_push_reference( OB_DOWNCAST(array), OB_DOWNCAST( MK_STRING_OBJ(ctx->vmem.label(i)) ) );
	}
	return OB_DOWNCAST(array);
}

HYBRIS_DEFINE_FUNCTION(hvar_values){
	unsigned int i;
	VectorObject *array = MK_VECTOR_OBJ();
	for( i = 0; i < ctx->vmem.size(); ++i ){
		ob_cl_push( OB_DOWNCAST(array), ctx->vmem.at(i) );
	}
	return OB_DOWNCAST(array);
}

HYBRIS_DEFINE_FUNCTION(huser_functions){
	unsigned int i;
	VectorObject *array = MK_VECTOR_OBJ();
	for( i = 0; i < ctx->vcode.size(); ++i ){
		ob_cl_push_reference( OB_DOWNCAST(array), OB_DOWNCAST( MK_STRING_OBJ(ctx->vcode.label(i)) ) );
	}
	return OB_DOWNCAST(array);
}

HYBRIS_DEFINE_FUNCTION(hdyn_functions){
    unsigned int i, j, mods = ctx->modules.size(), dyns;

    Object *map = OB_DOWNCAST( MK_MAP_OBJ() );
    for( i = 0; i < mods; ++i ){
        module_t *mod = ctx->modules[i];
        Object   *dyn = OB_DOWNCAST( MK_VECTOR_OBJ() );
        dyns          = mod->functions.size();
        for( j = 0; j < dyns; ++j ){
        	ob_cl_push_reference( dyn, OB_DOWNCAST( MK_STRING_OBJ(mod->functions[j]->identifier.c_str()) ) );
        }

        ob_cl_set_reference( map, OB_DOWNCAST( MK_STRING_OBJ( mod->name.c_str() ) ), dyn );
    }
    return map;
}

HYBRIS_DEFINE_FUNCTION(hcall){
	if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'call' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

	Node *call         = new Node(H_NT_CALL);
    call->value.m_call = STRING_ARGV(0);
	if( HYB_ARGC() > 1 ){
		unsigned int i;
		for( i = 1; i < HYB_ARGC(); ++i ){
			switch( HYB_ARGV(i)->type->code ){
				case otInteger : call->addChild( new ConstantNode( INT_ARGV(i) ) );   break;
				case otFloat   : call->addChild( new ConstantNode( FLOAT_ARGV(i) ) ); break;
				case otChar    : call->addChild( new ConstantNode( CHAR_ARGV(i) ) );   break;
				case otString  : call->addChild( new ConstantNode( (char *)STRING_ARGV(i).c_str() ) ); break;

				default :
                    hyb_throw( H_ET_GENERIC, "type %s not supported for reflected call", HYB_ARGV(i)->type->name );
			}
		}
	}

	Object *_return = ctx->engine->onFunctionCall( data, call, 0 );
	delete call;

	return _return;
}
