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

HYBRIS_DEFINE_FUNCTION(hload);
HYBRIS_DEFINE_FUNCTION(heval);
HYBRIS_DEFINE_FUNCTION(hvar_names);
HYBRIS_DEFINE_FUNCTION(hvar_values);
HYBRIS_DEFINE_FUNCTION(huser_functions);
HYBRIS_DEFINE_FUNCTION(hdyn_functions);
HYBRIS_DEFINE_FUNCTION(hmethods);
HYBRIS_DEFINE_FUNCTION(hcall);
HYBRIS_DEFINE_FUNCTION(hcall_method);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "load", hload },
	{ "eval", heval },
	{ "var_names", hvar_names },
	{ "var_values", hvar_values },
	{ "user_functions", huser_functions },
	{ "dyn_functions", hdyn_functions },
	{ "methods", hmethods },
	{ "call", hcall },
	{ "call_method", hcall_method },
	{ "", NULL }
};

extern "C" void hybris_module_init( vm_t * vm ){
	extern vm_t  *__hyb_vm;

	/*
	 * This module is linked against libhybris.so.1 which contains a compiled
	 * parser.cpp, which has an uninitialized __hyb_vm pointer.
	 * The real vm_t is passed to this function by the core, so we have to initialize
	 * the pointer with the right data.
	 */
	__hyb_vm = vm;
}

HYBRIS_DEFINE_FUNCTION(heval){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'eval' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otString, "eval" );

	extern int  yy_scan_string(const char *);
	extern int  yyparse(void);
	extern void yypop_buffer_state(void);

	/*
	 * Create a buffer from a string and make yyin points to it instead of
	 * a file handle.
	 */
	yy_scan_string( ob_svalue( ob_argv(0) ).c_str() );
	/*
	 * Do actual parsing (yyparse calls yylex).
	 */
	yyparse();
	/*
	 * Restore previous buffer.
	 */
	yypop_buffer_state();

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hload){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'load' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otString, "load" );

	extern int  yy_scan_string(const char *);
	extern int  yyparse(void);
	extern void yypop_buffer_state(void);

	string filename = ob_svalue( ob_argv(0) ),
		   buffer;

	if( hyb_file_exists( (char *)filename.c_str() ) == 0 ){
		return H_DEFAULT_ERROR;
	}

	FILE  *fp = fopen( filename.c_str(), "rt" );
	char   line[1024] = {0};

    while( fgets( line, 1024, fp ) != NULL ){
    	buffer += line;
    }

	fclose(fp);

	/*
	 * Create a buffer from a string and make yyin points to it instead of
	 * a file handle.
	 */
	yy_scan_string( buffer.c_str() );
	/*
	 * Do actual parsing (yyparse calls yylex).
	 */
	yyparse();
	/*
	 * Restore previous buffer.
	 */
	yypop_buffer_state();

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hvar_names){
	unsigned int i;
	VectorObject *array = gc_new_vector();
	for( i = 0; i < vm->vmem.size(); ++i ){
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(vm->vmem.label(i)) ) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(hvar_values){
	unsigned int i;
	VectorObject *array = gc_new_vector();
	for( i = 0; i < vm->vmem.size(); ++i ){
		ob_cl_push( ob_dcast(array), vm->vmem.at(i) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(huser_functions){
	unsigned int i;
	VectorObject *array = gc_new_vector();
	for( i = 0; i < vm->vcode.size(); ++i ){
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(vm->vcode.label(i)) ) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(hdyn_functions){
    unsigned int i, j, mods = vm->modules.size(), dyns;

    Object *map = ob_dcast( gc_new_map() );
    for( i = 0; i < mods; ++i ){
        module_t *mod = vm->modules[i];
        Object   *dyn = ob_dcast( gc_new_vector() );
        dyns          = mod->functions.size();
        for( j = 0; j < dyns; ++j ){
        	ob_cl_push_reference( dyn, ob_dcast( gc_new_string(mod->functions[j]->identifier.c_str()) ) );
        }

        ob_cl_set_reference( map, ob_dcast( gc_new_string( mod->name.c_str() ) ), dyn );
    }
    return map;
}

HYBRIS_DEFINE_FUNCTION(hmethods){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'methods' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otClass, "methods" );

	ClassObject  *co = (ClassObject *)ob_argv(0);
	Object 		 *vo = (Object *)gc_new_vector();
	ClassObjectMethodIterator i;

	for( i = co->c_methods.begin(); i != co->c_methods.end(); i++ ){
		ob_cl_push_reference( vo, (Object *)gc_new_string( (*i)->label.c_str() ) );
	}

	return vo;
}

HYBRIS_DEFINE_FUNCTION(hcall){
	if( ob_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "function 'call' requires at least 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otString, "call" );

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

	Object *_return = engine_on_function_call( vm->engine, data, call );
	delete call;

	return _return;
}

HYBRIS_DEFINE_FUNCTION(hcall_method){
	if( ob_argc() != 3 ){
		hyb_error( H_ET_SYNTAX, "function 'call_method' requires 3 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otClass,  "call_method" );
	ob_argv_type_assert( 1, otString, "call_method" );
	ob_argv_type_assert( 2, otVector, "call_method" );

	Object *classref   = ob_argv(0);
	char   *methodname = (char *)((StringObject *)ob_argv(1))->value.c_str();

	return ob_call_method( vm, classref, (char *)ob_typename(classref), methodname, ob_argv(2) );
}

