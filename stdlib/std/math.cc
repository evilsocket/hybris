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
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'acos' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( acos(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( acos(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hasin){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'asin' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( asin(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( asin(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'atan' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( atan(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( atan(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan2){
    if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'atan2' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );
    HYB_TYPES_ASSERT( HYB_ARGV(1), otInteger, otFloat );

    double a, b;

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        a = (double)INT_VALUE( HYB_ARGV(0) );
    }
    else{
        a = FLOAT_VALUE( HYB_ARGV(0) );
    }
    if( IS_INTEGER_TYPE( HYB_ARGV(1) ) ){
        b = (double)INT_VALUE( HYB_ARGV(0) );
    }
    else{
        b = FLOAT_VALUE( HYB_ARGV(1) );
    }
    return (Object *)MK_FLOAT_OBJ( atan2(a,b));
}

HYBRIS_DEFINE_FUNCTION(hceil){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'ceil' requires 1 parameter (called with %d)", HYB_ARGC() );
	}

	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_INT_OBJ( ceil(INT_ARGV(0) ) );
    }
    else{
        return (Object *)MK_INT_OBJ( ceil(FLOAT_ARGV(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hcos){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'cos' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( cos(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( cos(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hcosh){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'cosh' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( cosh(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( cosh(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hexp){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'exp' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( exp(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( exp(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfabs){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fabs' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( fabs(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( fabs(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfloor){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'floor' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_INT_OBJ( floor(INT_ARGV(0) ) );
    }
    else{
        return (Object *)MK_INT_OBJ( floor(FLOAT_ARGV(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hfmod){
    if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'fmod' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );
    HYB_TYPES_ASSERT( HYB_ARGV(1), otInteger, otFloat );

    double a, b;

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        a = (double)INT_VALUE( HYB_ARGV(0) );
    }
    else{
        a = FLOAT_VALUE( HYB_ARGV(0) );
    }
    if( IS_INTEGER_TYPE( HYB_ARGV(1) ) ){
        b = (double)INT_VALUE( HYB_ARGV(0) );
    }
    else{
        b = FLOAT_VALUE( HYB_ARGV(1) );
    }
    return (Object *)MK_FLOAT_OBJ( fmod(a,b));
}

HYBRIS_DEFINE_FUNCTION(hlog){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'hlog' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( log(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( log(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hlog10){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'log10' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( log10(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( log10(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hpow){
    if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'pow' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );
    HYB_TYPES_ASSERT( HYB_ARGV(1), otInteger, otFloat );

    double a, b;

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        a = (double)INT_VALUE( HYB_ARGV(0) );
    }
    else{
        a = FLOAT_VALUE( HYB_ARGV(0) );
    }
    if( IS_INTEGER_TYPE( HYB_ARGV(1) ) ){
        b = (double)INT_VALUE( HYB_ARGV(0) );
    }
    else{
        b = FLOAT_VALUE( HYB_ARGV(1) );
    }
    return (Object *)MK_FLOAT_OBJ( pow(a,b));
}

HYBRIS_DEFINE_FUNCTION(hsin){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sin' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( sin(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( sin(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsinh){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sinh' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( sinh(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( sinh(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsqrt){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sqrt' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( sqrt(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( sqrt(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htan){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'tan' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( tan(INT_ARGV(0) ));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( tan(FLOAT_ARGV(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htanh){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'tanh' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otFloat );

    if( IS_INTEGER_TYPE( HYB_ARGV(0) ) ){
        return (Object *)MK_FLOAT_OBJ( tanh(INT_VALUE( HYB_ARGV(0) )));
    }
    else{
        return (Object *)MK_FLOAT_OBJ( tanh(FLOAT_ARGV(0) ));
    }
}







