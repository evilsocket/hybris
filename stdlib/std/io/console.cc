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
#include <iostream>
#include <dlfcn.h>
#include <ffi.h>
#include <readline/readline.h>
#include <readline/history.h>

using std::cin;

HYBRIS_DEFINE_FUNCTION(hprint);
HYBRIS_DEFINE_FUNCTION(hprintln);
HYBRIS_DEFINE_FUNCTION(hprintf);
HYBRIS_DEFINE_FUNCTION(hinput);
HYBRIS_DEFINE_FUNCTION(hreadline);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "print",    hprint,    H_ANY_ARGC },
	{ "println",  hprintln,  H_ANY_ARGC },
	{ "printf",   hprintf,   H_REQ_ARGC(1),   { H_REQ_TYPES(otString) } },
	{ "input",    hinput,    H_REQ_ARGC(1,2), { H_REQ_TYPES(otString), H_ANY_TYPE } },
	{ "readline", hreadline, H_REQ_ARGC(0,1), { H_REQ_TYPES(otString) } },
	{ "", NULL }
};

#define PRINTF_MAX_ARGS 256

union call_arg_ctype {
    char   c;
    int    i;
    double d;
    void  *p;
};

typedef struct {
	ffi_type            *type;
	union call_arg_ctype value;
	bool                 dynamic;
}
dll_arg_t;

byte *binary_serialize( Object *o ){
    size_t i, size( ob_get_size(o) );
    byte *buffer = new byte[ size ];

    for( i = 0; i < size; ++i ){
        buffer[i] = (byte)ob_ivalue( ob_binary_ucast(o)->value[i] );
    }

    return buffer;
}

static void ctype_convert( Object *o, dll_arg_t *pa ) {
    pa->dynamic = false;
	if( o->type->code == otVoid ){
        pa->type    = &ffi_type_pointer;
        pa->value.p = H_UNDEFINED;
	}
    else if( o->type->code == otInteger || o->type->code == otAlias || o->type->code == otExtern ){
		pa->type    = &ffi_type_sint;
		pa->value.i = ob_ivalue(o);
	}
	else if( o->type->code == otChar ){
        pa->type    = &ffi_type_schar;
        pa->value.c = ob_ivalue(o);
	}
	else if( o->type->code == otFloat ){
	    pa->type    = &ffi_type_double;
        pa->value.d = ob_fvalue(o);
	}
    else if( o->type->code == otString ){
		pa->type    = &ffi_type_pointer;
		pa->value.p = (void *)ob_string_val(o).c_str();
	}
	else if( o->type->code == otBinary ){
	    pa->dynamic = true;
        pa->type    = &ffi_type_pointer;
        pa->value.p = (void *)binary_serialize(o);
	}
	else{
		string s = ob_svalue(o);
		pa->type    = &ffi_type_pointer;
		pa->value.p = (void *)s.c_str();
	}
}

HYBRIS_DEFINE_FUNCTION(hprint){
    unsigned int i;
    for( i = 0; i < vm_argc(); ++i ){
        ob_print( vm_argv(i) );
    }
    return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hprintln){
    if( vm_argc() ){
        unsigned int i;
        for( i = 0; i < vm_argc(); ++i ){
            ob_print( vm_argv(i) );
            fprintf( stdout, "\n" );
        }
    }
    else{
        fprintf( stdout,"\n");
    }
    return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hprintf){
	if( vm_argc() > PRINTF_MAX_ARGS ){
		hyb_error( H_ET_SYNTAX, "function 'printf' supports at max %d parameters (called with %d)", PRINTF_MAX_ARGS, vm_argc() );
	}

	typedef int (* function_t)(void);
	function_t function = (function_t)printf;

	ffi_cif    cif;
	ffi_arg    ul_ret;
	int        dsize( vm_argc() ),
			   argc( dsize ),
			   i;
	ffi_type **args_t;
	void     **args_v;
	dll_arg_t *args, *parg;

	args   = (dll_arg_t *)alloca( sizeof(dll_arg_t) * argc );
	args_t = (ffi_type **)alloca( sizeof(ffi_type *) * argc );
	args_v = (void **)alloca( sizeof(void *) * argc );

	memset( args, 0, sizeof(dll_arg_t) * argc );

	/* convert objects to c-type equivalents */
	ctype_convert( vm_argv(0), &args[0] );
	for( i = 1, parg = &args[1]; i < dsize; ++i, ++parg ){
		ctype_convert( vm_argv(i), parg );
	}
	/* assign types and values */
	for( i = 0; i < dsize; ++i ){
		args_t[i] = args[i].type;
		if( args_t[i]->type == FFI_TYPE_STRUCT ){
			args_v[i] = (void *)args[i].value.p;
		}
		else{
			args_v[i] = (void *)&args[i].value;
		}
	}

	if( ffi_prep_cif( &cif, FFI_DEFAULT_ABI, argc, &ffi_type_ulong, args_t ) != FFI_OK ){
		hyb_error( H_ET_GENERIC, "ffi_prep_cif failed" );
	}

	ffi_call( &cif, FFI_FN(function), &ul_ret, args_v );

	/* release dynamically allocated data */
	for( i = 0; i < argc; ++i ){
		if( args[i].dynamic == true ){
			delete [] args[i].value.p;
		}
	}

	return ob_dcast( gc_new_integer(ul_ret) );
}

HYBRIS_DEFINE_FUNCTION(hinput){
    Object *_return;
    if( vm_argc() == 2 ){
        ob_print( vm_argv(0) );
        ob_input( vm_argv(1) );
        _return = vm_argv(1);
    }
    else if( vm_argc() == 1 ){
        ob_input( vm_argv(0) );
        _return = vm_argv(0);
    }

    return _return;
}

HYBRIS_DEFINE_FUNCTION(hreadline){
	char   *prompt = NULL,
		   *line   = NULL;
	Object *retn;

	vm_parse_argv( "p", &prompt );

	line = readline(prompt);
	if( !line ){
		retn = (Object *)gc_new_string("");
	}
	else{
		add_history(line);

		retn = (Object *)gc_new_string(line);

		free(line);
	}

	return retn;
}
