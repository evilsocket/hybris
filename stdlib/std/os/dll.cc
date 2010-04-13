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
    else if( o->type == Integer_Type ){
		pa->type    = &ffi_type_sint;
		pa->value.i = o->value.m_integer;
	}
	else if( o->type == H_OT_CHAR ){
        pa->type    = &ffi_type_schar;
        pa->value.c = o->value.m_char;
	}
	else if( o->type == Float_Type ){
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
        hyb_throw( H_ET_SYNTAX, "could not use '%s' type for dllcall function", Object::type_name(o) );
	}
}

HYBRIS_DEFINE_FUNCTION(hdllopen){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'dllopen' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

    return PTR_TO_INT_OBJ( dlopen( (const char *)(*HYB_ARGV(0)), RTLD_LAZY ) );
}

HYBRIS_DEFINE_FUNCTION(hdlllink){
    if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'dlllink' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type    );
	HYB_TYPE_ASSERT( HYB_ARGV(1), H_OT_STRING );

    void *hdll = reinterpret_cast<void *>( (long)(*HYB_ARGV(0)) );

    return new Object( reinterpret_cast<long>( dlsym( hdll, (const char *)(*HYB_ARGV(1)) ) ), 1 );
}

HYBRIS_DEFINE_FUNCTION(hdllcall){
    if( HYB_ARGC() < 1 ){
        hyb_throw( H_ET_SYNTAX, "function 'dllcall' requires at least 1 parameter (called with %d)", HYB_ARGC() );
    }
    else if( HYB_ARGC() > CALL_MAX_ARGS + 1 ){
        hyb_throw( H_ET_SYNTAX, "function 'dllcall' support at max %d parameters (called with %d)", CALL_MAX_ARGS, HYB_ARGC() );
    }

    HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type );

    typedef int (* function_t)(void);
    function_t function = (function_t)(long)(*HYB_ARGV(0));

    ffi_cif    cif;
    ffi_arg    ul_ret;
    int        dsize( HYB_ARGC() ),
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
        ctype_convert( HYB_ARGV(i), parg );
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
        hyb_throw( H_ET_GENERIC, "ffi_prep_cif failed" );
    }

    ffi_call( &cif, FFI_FN(function), &ul_ret, args_v );

    /* release dynamically allocated data */
    for( i = 0; i < argc; ++i ){
		if( args[i].dynamic == true ){
            delete [] args[i].value.p;
		}
	}

    return MK_INT_OBJ(ul_ret);
}

HYBRIS_DEFINE_FUNCTION(hdllclose){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'dllclose' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type );

	dlclose( (void *)(long)(*HYB_ARGV(0)) );

    return MK_INT_OBJ(0);
}

