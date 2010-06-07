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
	{ "load",           hload,           H_REQ_ARGC(1), { H_REQ_TYPES(otString)} },
	{ "eval",           heval,           H_REQ_ARGC(1), { H_REQ_TYPES(otString)} },
	{ "var_names",      hvar_names,      H_NO_ARGS },
	{ "var_values",     hvar_values,     H_NO_ARGS },
	{ "user_functions", huser_functions, H_NO_ARGS },
	{ "dyn_functions",  hdyn_functions,  H_NO_ARGS },
	{ "methods",        hmethods,        H_REQ_ARGC(1), { H_REQ_TYPES(otClass)  } },
	{ "call",           hcall,		     H_REQ_ARGC(1), { H_REQ_TYPES(otString) } },
	{ "call_method",    hcall_method,    H_REQ_ARGC(3), { H_REQ_TYPES(otClass), H_REQ_TYPES(otString), H_REQ_TYPES(otVector) } },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(heval){
	char *code;

	vm_parse_argv( "p", &code );

	hyb_parse_string( vm, code );

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hload){
	char  *filename;
	string buffer;

	vm_parse_argv( "p", &filename );

	if( hyb_file_exists( filename ) == 0 ){
		return H_DEFAULT_ERROR;
	}

	hyb_parse_file( vm, filename );

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hvar_names){
	unsigned int i;
	Vector *array = gc_new_vector();
	for( i = 0; i < vm->vmem.size(); ++i ){
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(vm->vmem.label(i)) ) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(hvar_values){
	unsigned int i;
	Vector *array = gc_new_vector();
	for( i = 0; i < vm->vmem.size(); ++i ){
		ob_cl_push( ob_dcast(array), vm->vmem.at(i) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(huser_functions){
	unsigned int i;
	Vector *array = gc_new_vector();
	for( i = 0; i < vm->vcode.size(); ++i ){
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(vm->vcode.label(i)) ) );
	}
	return ob_dcast(array);
}

HYBRIS_DEFINE_FUNCTION(hdyn_functions){
	ll_item_t	  *m_item,
				  *f_item;
	vm_module_t   *module;
	vm_function_t *function;

	Object *map = (Object *)gc_new_map(),
		   *vec;

	for( m_item = vm->modules.head; m_item; m_item = m_item->next ){
		module = ll_data( vm_module_t *, m_item );
		vec    = (Object *)gc_new_vector();

		for( f_item = module->functions.head; f_item; f_item = f_item->next ){
			ob_cl_push_reference( vec, (Object *)gc_new_string( ll_data( vm_function_t *, f_item )->identifier.c_str()  ) );
		}

		ob_cl_set_reference( map, (Object *)gc_new_string( module->name.c_str() ), vec );
	}

	return map;
}

HYBRIS_DEFINE_FUNCTION(hmethods){
	Class  *co;
	Object *vo = (Object *)gc_new_vector();
	ClassMethodIterator i;

	vm_parse_argv( "C", &co );

	itree_foreach( class_method_t, i, co->c_methods ){
		ob_cl_push_reference( vo, (Object *)gc_new_string( (*i)->label.c_str() ) );
	}

	return vo;
}

HYBRIS_DEFINE_FUNCTION(hcall){
	string function;
	vmem_t stack;

	vm_parse_argv( "s", &function );

	if( vm_argc() > 1 ){
		unsigned int i;
		for( i = 1; i < vm_argc(); ++i ){
			stack.push( vm_argv(i) );
		}
	}

	Object *state = vm_exec_threaded_call( vm, function, &stack );

	return state;
}

HYBRIS_DEFINE_FUNCTION(hcall_method){
	Class  *classref;
	char   *methodname;
	Vector *argv;

	vm_parse_argv( "CpV", &classref, &methodname, &argv );

	char    *classname  = (char *)ob_typename( (Object *)classref );
	Node    *method = H_UNDEFINED;
	vframe_t stack;
	Object  *value  = H_UNDEFINED,
			*result = H_UNDEFINED;
	Integer index(0);
	size_t j, argc( ob_get_size( (Object *)argv ) );

	method = ob_get_method( (Object *)classref, methodname, 2 );
	if( method == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' does not name a method neither an attribute of '%s'", methodname, classname );
	}
	stack.owner = string(classname) + "::" + methodname;
	classref->referenced = true;
	stack.insert( "me", (Object *)classref );
	for( ; (unsigned)index.value < argc; ++index.value ){
		value = ob_cl_at( (Object *)argv, (Object *)&index );
		stack.insert( method->child( index.value )->id(), value  );
	}

	vm_add_frame( vm, &stack );

	/* call the method */
	result = vm_exec( vm, &stack, method->body );

	vm_pop_frame( vm );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm_frame( vm )->state.set( Exception, stack.state.e_value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

