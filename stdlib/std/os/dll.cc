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
#include <dlfcn.h>
#include <ffi.h>
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hdllopen);
HYBRIS_DEFINE_FUNCTION(hdlllink);
HYBRIS_DEFINE_FUNCTION(hdllcall);
HYBRIS_DEFINE_FUNCTION(hdllcall_argv);
HYBRIS_DEFINE_FUNCTION(hdllclose);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "dllopen", 	  hdllopen,      H_REQ_ARGC(1), { H_REQ_TYPES(otString) } },
	{ "dlllink",	  hdlllink,      H_REQ_ARGC(2), { H_REQ_TYPES(otInteger), H_REQ_TYPES(otString) } },
	{ "dllcall", 	  hdllcall,      H_REQ_ARGC(1), { H_REQ_TYPES(otExtern) } },
	{ "dllcall_argv", hdllcall_argv, H_REQ_ARGC(2), { H_REQ_TYPES(otExtern), H_REQ_TYPES(otVector) } },
	{ "dllclose", 	  hdllclose,     H_REQ_ARGC(1), { H_REQ_TYPES(otInteger) } },
	{ "", NULL }
};

#define CALL_MAX_ARGS 256

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
        hyb_error( H_ET_SYNTAX, "could not use '%s' type for dllcall function", o->type->name );
	}
}

HYBRIS_DEFINE_FUNCTION(hdllopen){
    return ob_dcast( PTR_TO_INT_OBJ( dlopen( string_argv(0).c_str(), RTLD_LAZY ) ) );
}

HYBRIS_DEFINE_FUNCTION(hdlllink){
    void *hdll = reinterpret_cast<void *>( int_argv(0) );

    return ob_dcast( gc_new_extern( H_ADDRESS_OF( dlsym( hdll, string_argv(1).c_str() ) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hdllcall){
	if( ob_argc() > CALL_MAX_ARGS + 1 ){
        hyb_error( H_ET_SYNTAX, "function 'dllcall' supports at max %d parameters (called with %d)", CALL_MAX_ARGS, ob_argc() );
    }

    typedef int (* function_t)(void);
    function_t function = (function_t)extern_argv(0);

    ffi_cif    cif;
    ffi_arg    ul_ret;
    int        dsize( ob_argc() ),
               argc( dsize - 1 ),
               i;
    ffi_type **args_t;
	void     **args_v;
	dll_arg_t *args, *parg;

    args   = (dll_arg_t *)alloca( sizeof(dll_arg_t) * argc );
    args_t = (ffi_type **)alloca( sizeof(ffi_type *) * argc );
    args_v = (void **)alloca( sizeof(void *) * argc );

	memset( args, 0, sizeof(dll_arg_t) * argc );

    /* convert objects to c-type equivalents */
    for( i = 1, parg = &args[0]; i < dsize; ++i, ++parg ){
        ctype_convert( ob_argv(i), parg );
    }
    /* assign types and values */
    for( i = 0; i < argc; ++i ){
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

HYBRIS_DEFINE_FUNCTION(hdllcall_argv){
    vframe_t stack;
    int size( ob_get_size( ob_argv(1) ) );
    Integer index(0);

    stack.push( ob_argv(0) );
    for( ; index.value < size; ++index.value ){
    	stack.push( ob_cl_at( ob_argv(1), (Object *)&index ) );
    }

    return hdllcall( vm, &stack );
}

HYBRIS_DEFINE_FUNCTION(hdllclose){
	if( int_argv(0) ){
		dlclose( (void *)int_argv(0) );
	}

    return H_DEFAULT_RETURN;
}

