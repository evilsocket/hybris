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
HYBRIS_DEFINE_FUNCTION(hdllclose);

extern "C" named_function_t hybris_module_functions[] = {
	{ "dllopen", hdllopen },
	{ "dlllink", hdlllink },
	{ "dllcall", hdllcall },
	{ "dllclose", hdllclose },
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

unsigned char *binary_serialize( Object *o ){
    unsigned int   i, size( o->size );
    unsigned char *buffer = new unsigned char[ size ];

    for( i = 0; i < size; ++i ){
        buffer[i] = (unsigned char)o->value.m_array[i]->value.m_char;
    }

    return buffer;
}

static void ctype_convert( Object *o, dll_arg_t *pa ) {
    pa->dynamic = false;
	if( o->type == H_OT_VOID ){
        pa->type    = &ffi_type_pointer;
        pa->value.p = H_UNDEFINED;
	}
    else if( o->type == H_OT_INT ){
		pa->type    = &ffi_type_sint;
		pa->value.i = o->value.m_integer;
	}
	else if( o->type == H_OT_CHAR ){
        pa->type    = &ffi_type_schar;
        pa->value.c = o->value.m_char;
	}
	else if( o->type == H_OT_FLOAT ){
	    pa->type    = &ffi_type_double;
        pa->value.d = o->value.m_double;
	}
    else if( o->type == H_OT_STRING ){
		pa->type    = &ffi_type_pointer;
		pa->value.p = (void *)o->value.m_string.c_str();
	}
	else if( o->type == H_OT_BINARY ){
	    pa->dynamic = true;
        pa->type    = &ffi_type_pointer;
        pa->value.p = (void *)binary_serialize(o);
	}
	else{
        hyb_syntax_error( "could not use '%s' type for dllcall function", Object::type_name(o) );
	}
}

HYBRIS_DEFINE_FUNCTION(hdllopen){
	if( data->size() != 1 ){
		hyb_syntax_error( "function 'dllopen' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_STRING );

    return new Object( reinterpret_cast<long>( dlopen( data->at(0)->value.m_string.c_str(), RTLD_LAZY ) ) );
}

HYBRIS_DEFINE_FUNCTION(hdlllink){
    if( data->size() != 2 ){
		hyb_syntax_error( "function 'dlllink' requires 2 parameters (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT    );
	hyb_type_assert( data->at(1), H_OT_STRING );

    void *hdll = reinterpret_cast<void *>( data->at(0)->value.m_integer );

    return new Object( reinterpret_cast<long>( dlsym( hdll, data->at(1)->value.m_string.c_str() ) ), 1 );
}

HYBRIS_DEFINE_FUNCTION(hdllcall){
    if( data->size() < 1 ){
        hyb_syntax_error( "function 'dllcall' requires at least 1 parameter (called with %d)", data->size() );
    }
    else if( data->size() > CALL_MAX_ARGS + 1 ){
        hyb_syntax_error( "function 'dllcall' support at max %d parameters (called with %d)", CALL_MAX_ARGS, data->size() );
    }

    hyb_type_assert( data->at(0), H_OT_INT );

    typedef int (* function_t)(void);
    function_t function = (function_t)data->at(0)->value.m_integer;

    ffi_cif    cif;
    ffi_arg    ul_ret;
    int        dsize( data->size() ),
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
        ctype_convert( data->at(i), parg );
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
        hyb_generic_error( "ffi_prep_cif failed" );
    }

    ffi_call( &cif, FFI_FN(function), &ul_ret, args_v );

    /* release dynamically allocated data */
    for( i = 0; i < argc; ++i ){
		if( args[i].dynamic == true ){
            delete [] args[i].value.p;
		}
	}

    return new Object( static_cast<long>(ul_ret) );
}

HYBRIS_DEFINE_FUNCTION(hdllclose){
	if( data->size() != 1 ){
		hyb_syntax_error( "function 'dllclose' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT );

	dlclose( (void *)data->at(0)->value.m_integer );

    return new Object( static_cast<long>(0) );
}

