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
#include "common.h"
#include "vmem.h"
#include "builtin.h"
#include <dlfcn.h>
#include <ffi.h>

#define CALL_MAX_ARGS 256

HYBRIS_BUILTIN(hdllopen){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'dllopen' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

    return new Object( reinterpret_cast<long>( dlopen( data->at(0)->xstring.c_str(), RTLD_LAZY ) ) );
}

HYBRIS_BUILTIN(hdlllink){
    if( data->size() != 2 ){
		hybris_syntax_error( "function 'dlllink' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT    );
	htype_assert( data->at(1), H_OT_STRING );

    void *hdll = reinterpret_cast<void *>( data->at(0)->xint );

    return new Object( reinterpret_cast<long>( dlsym( hdll, data->at(1)->xstring.c_str() ) ), 1 );
}

HYBRIS_BUILTIN(hdllcall){
    if( data->size() < 1 ){
        hybris_syntax_error( "function 'dllcall' requires at least 1 parameter (called with %d)", data->size() );
    }
    else if( data->size() > CALL_MAX_ARGS + 1 ){
        hybris_syntax_error( "function 'dllcall' support at max %d parameters (called with %d)", CALL_MAX_ARGS, data->size() );
    }

    htype_assert( data->at(0), H_OT_INT );

    typedef int (* function_t)(void);
    function_t function = (function_t)data->at(0)->xint;

    ffi_cif    cif;
    ffi_arg    ul_ret;
    int        argc( data->size() - 1 );
    ffi_type  *args_t[CALL_MAX_ARGS];
	void      *args_v[CALL_MAX_ARGS];
	vector<unsigned char *> raw_gbc;
	vector<unsigned long *> ulg_gbc;
    int    i, j;
    int    iv;
    double fv;
    char   cv;
    unsigned char *raw;
    unsigned long *ulval;
    Object        *arg;

    for( i = 1, j = 0; i < data->size(); ++i, ++j ){
        arg   = data->at(i);
        raw   = arg->serialize();
        ulval = new unsigned long;

        raw_gbc.push_back(raw);
        ulg_gbc.push_back(ulval);

        switch( arg->xtype ){
            case H_OT_INT    : memcpy( &iv, raw, sizeof(long) );   *ulval = (unsigned long)iv; break;
            case H_OT_FLOAT  : memcpy( &fv, raw, sizeof(double) ); *ulval = (unsigned long)fv; break;
            case H_OT_CHAR   : memcpy( &cv, raw, sizeof(char) );   *ulval = (unsigned long)cv; break;
            case H_OT_STRING : *ulval = (unsigned long)raw; 								   break;
			case H_OT_MAP    : *ulval = (unsigned long)raw; 								   break;
			case H_OT_ALIAS  : memcpy( ulval, raw, sizeof(unsigned long) ); 				   break;

            default :
               hybris_syntax_error( "could not use '%s' type for dllcall function", Object::type(arg) );
        }
        /* fill the stack vector */
         args_t[j] = &ffi_type_ulong;
         args_v[j] = ulval;
    }

    if( ffi_prep_cif( &cif, FFI_DEFAULT_ABI, argc, &ffi_type_ulong, args_t ) != FFI_OK ){
        hybris_generic_error( "ffi_prep_cif failed" );
    }

    ffi_call( &cif, FFI_FN(function), &ul_ret, args_v );

    for( i = 0; i < argc; ++i ){
        delete[] raw_gbc[i];
        delete   ulg_gbc[i];
    }

    return new Object( static_cast<long>(ul_ret) );
}

HYBRIS_BUILTIN(hdllclose){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'dllclose' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );

	dlclose( (void *)data->at(0)->xint );

    return new Object( static_cast<long>(0) );
}

