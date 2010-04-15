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
#include <math.h>

HYBRIS_DEFINE_FUNCTION(hacos);
HYBRIS_DEFINE_FUNCTION(hasin);
HYBRIS_DEFINE_FUNCTION(hatan);
HYBRIS_DEFINE_FUNCTION(hatan2);
HYBRIS_DEFINE_FUNCTION(hceil);
HYBRIS_DEFINE_FUNCTION(hcos);
HYBRIS_DEFINE_FUNCTION(hcosh);
HYBRIS_DEFINE_FUNCTION(hexp);
HYBRIS_DEFINE_FUNCTION(hfabs);
HYBRIS_DEFINE_FUNCTION(hfloor);
HYBRIS_DEFINE_FUNCTION(hfmod);
HYBRIS_DEFINE_FUNCTION(hlog);
HYBRIS_DEFINE_FUNCTION(hlog10);
HYBRIS_DEFINE_FUNCTION(hpow);
HYBRIS_DEFINE_FUNCTION(hsin);
HYBRIS_DEFINE_FUNCTION(hsinh);
HYBRIS_DEFINE_FUNCTION(hsqrt);
HYBRIS_DEFINE_FUNCTION(htan);
HYBRIS_DEFINE_FUNCTION(htanh);

extern "C" named_function_t hybris_module_functions[] = {
    {"acos", hacos },
    {"asin", hasin },
    {"atan", hatan },
    {"atan2", hatan2 },
    {"ceil", hceil },
    {"cos", hcos },
    {"cosh", hcosh },
    {"exp", hexp },
    {"fabs", hfabs },
    {"floor", hfloor },
    {"fmod", hfmod },
    {"log", hlog },
    {"log10", hlog10 },
    {"pow", hpow },
    {"sin", hsin },
    {"sinh", hsinh },
    {"sqrt", hsqrt },
    {"tan", htan },
    {"tanh", htanh },
    { "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hacos){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'acos' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( acos(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( acos(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hasin){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'asin' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( asin(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( asin(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'atan' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( atan(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( atan(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan2){
    if( ob_argc() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'atan2' requires 2 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );
    ob_types_assert( ob_argv(1), otInteger, otFloat );

    double a, b;

    if( ob_is_int( ob_argv(0) ) ){
        a = (double)ob_int_val( ob_argv(0) );
    }
    else{
        a = ob_float_val( ob_argv(0) );
    }
    if( ob_is_int( ob_argv(1) ) ){
        b = (double)ob_int_val( ob_argv(0) );
    }
    else{
        b = ob_float_val( ob_argv(1) );
    }
    return (Object *)gc_new_float( atan2(a,b));
}

HYBRIS_DEFINE_FUNCTION(hceil){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'ceil' requires 1 parameter (called with %d)", ob_argc() );
	}

	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_integer( ceil(int_argv(0) ) );
    }
    else{
        return (Object *)gc_new_integer( ceil(float_argv(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hcos){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'cos' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( cos(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( cos(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hcosh){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'cosh' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( cosh(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( cosh(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hexp){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'exp' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( exp(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( exp(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfabs){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fabs' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( fabs(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( fabs(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfloor){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'floor' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_integer( floor(int_argv(0) ) );
    }
    else{
        return (Object *)gc_new_integer( floor(float_argv(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hfmod){
    if( ob_argc() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'fmod' requires 2 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );
    ob_types_assert( ob_argv(1), otInteger, otFloat );

    double a, b;

    if( ob_is_int( ob_argv(0) ) ){
        a = (double)ob_int_val( ob_argv(0) );
    }
    else{
        a = ob_float_val( ob_argv(0) );
    }
    if( ob_is_int( ob_argv(1) ) ){
        b = (double)ob_int_val( ob_argv(0) );
    }
    else{
        b = ob_float_val( ob_argv(1) );
    }
    return (Object *)gc_new_float( fmod(a,b));
}

HYBRIS_DEFINE_FUNCTION(hlog){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'hlog' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( log(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( log(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hlog10){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'log10' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( log10(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( log10(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hpow){
    if( ob_argc() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'pow' requires 2 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );
    ob_types_assert( ob_argv(1), otInteger, otFloat );

    double a, b;

    if( ob_is_int( ob_argv(0) ) ){
        a = (double)ob_int_val( ob_argv(0) );
    }
    else{
        a = ob_float_val( ob_argv(0) );
    }
    if( ob_is_int( ob_argv(1) ) ){
        b = (double)ob_int_val( ob_argv(0) );
    }
    else{
        b = ob_float_val( ob_argv(1) );
    }
    return (Object *)gc_new_float( pow(a,b));
}

HYBRIS_DEFINE_FUNCTION(hsin){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sin' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sin(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sin(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsinh){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sinh' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sinh(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sinh(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsqrt){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sqrt' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sqrt(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sqrt(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htan){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'tan' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( tan(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( tan(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htanh){
    if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'tanh' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otFloat );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( tanh(ob_int_val( ob_argv(0) )));
    }
    else{
        return (Object *)gc_new_float( tanh(float_argv(0) ));
    }
}







