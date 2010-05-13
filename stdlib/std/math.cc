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

HYBRIS_EXPORTED_FUNCTIONS() {
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

extern "C" void hybris_module_init( vm_t * vm ){
	#if defined __USE_BSD || defined __USE_XOPEN
	HYBRIS_DEFINE_CONSTANT( vm, "M_E",        gc_new_float(M_E) );        /* e */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LOG2E",    gc_new_float(M_LOG2E) );    /* log_2 e */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LOG10E",   gc_new_float(M_LOG10E) );   /* log_10 e */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LN2",      gc_new_float(M_LN2) );      /* log_e 2 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LN10",     gc_new_float(M_LN10) );     /* log_e 10 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_PI",       gc_new_float(M_PI) );       /* pi */
	HYBRIS_DEFINE_CONSTANT( vm, "M_PI_2",     gc_new_float(M_PI_2) );     /* pi/2 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_PI_4",     gc_new_float(M_PI_4) );     /* pi/4 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_1_PI",     gc_new_float(M_1_PI) );     /* 1/pi */
	HYBRIS_DEFINE_CONSTANT( vm, "M_2_PI",     gc_new_float(M_2_PI) );     /* 2/pi */
	HYBRIS_DEFINE_CONSTANT( vm, "M_2_SQRTPI", gc_new_float(M_2_SQRTPI) ); /* 2/sqrt(pi) */
	HYBRIS_DEFINE_CONSTANT( vm, "M_SQRT2",    gc_new_float(M_SQRT2) );    /* sqrt(2) */
	HYBRIS_DEFINE_CONSTANT( vm, "M_SQRT1_2",  gc_new_float(M_SQRT1_2) );  /* 1/sqrt(2) */
	#endif
	/* The above constants are not adequate for computation using `long double's.
	   Therefore we provide as an extension constants with similar names as a
	   GNU extension.  Provide enough digits for the 128-bit IEEE quad.  */
	#ifdef __USE_GNU
	HYBRIS_DEFINE_CONSTANT( vm, "M_El",        gc_new_float(M_El) );        /* e */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LOG2El",    gc_new_float(M_LOG2El) );    /* log_2 e */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LOG10El",   gc_new_float(M_LOG10El) );   /* log_10 e */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LN2l",      gc_new_float(M_LN2l) );      /* log_e 2 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_LN10l",     gc_new_float(M_LN10l) );     /* log_e 10 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_PIl",       gc_new_float(M_PIl) );       /* pi */
	HYBRIS_DEFINE_CONSTANT( vm, "M_PI_2l",     gc_new_float(M_PI_2l) );     /* pi/2 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_PI_4l",     gc_new_float(M_PI_4l) );     /* pi/4 */
	HYBRIS_DEFINE_CONSTANT( vm, "M_1_PIl",     gc_new_float(M_1_PIl) );     /* 1/pi */
	HYBRIS_DEFINE_CONSTANT( vm, "M_2_PIl",     gc_new_float(M_2_PIl) );     /* 2/pi */
	HYBRIS_DEFINE_CONSTANT( vm, "M_2_SQRTPIl", gc_new_float(M_2_SQRTPIl) ); /* 2/sqrt(pi) */
	HYBRIS_DEFINE_CONSTANT( vm, "M_SQRT2l",    gc_new_float(M_SQRT2l) );    /* sqrt(2) */
	HYBRIS_DEFINE_CONSTANT( vm, "M_SQRT1_2l",  gc_new_float(M_SQRT1_2l) );  /* 1/sqrt(2) */
	#endif
}

HYBRIS_DEFINE_FUNCTION(hacos){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'acos' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "acos" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( acos(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( acos(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hasin){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'asin' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "asin" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( asin(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( asin(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'atan' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "atan" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( atan(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( atan(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan2){
    if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'atan2' requires 2 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "atan2" );
    ob_argv_types_assert( 1, otInteger, otFloat, "atan2" );

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
		hyb_error( H_ET_SYNTAX, "function 'ceil' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "ceil" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_integer( ceil(int_argv(0) ) );
    }
    else{
        return (Object *)gc_new_integer( ceil(float_argv(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hcos){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'cos' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "cos" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( cos(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( cos(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hcosh){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'cosh' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "cosh" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( cosh(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( cosh(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hexp){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'exp' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "exp" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( exp(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( exp(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfabs){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'fabs' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "fabs" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( fabs(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( fabs(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfloor){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'floor' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "floor" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_integer( floor(int_argv(0) ) );
    }
    else{
        return (Object *)gc_new_integer( floor(float_argv(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hfmod){
    if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'fmod' requires 2 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "fmod" );
    ob_argv_types_assert( 1, otInteger, otFloat, "fmod" );

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
		hyb_error( H_ET_SYNTAX, "function 'hlog' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "log" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( log(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( log(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hlog10){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'log10' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "log10" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( log10(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( log10(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hpow){
    if( ob_argc() != 2 ){
		hyb_error( H_ET_SYNTAX, "function 'pow' requires 2 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "pow" );
    ob_argv_types_assert( 1, otInteger, otFloat, "pow" );

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
		hyb_error( H_ET_SYNTAX, "function 'sin' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "sin" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sin(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sin(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsinh){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'sinh' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "sinh" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sinh(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sinh(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsqrt){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'sqrt' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "sqrt" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sqrt(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sqrt(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htan){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'tan' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "tan" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( tan(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( tan(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htanh){
    if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'tanh' requires 1 parameter (called with %d)", ob_argc() );
	}
    ob_argv_types_assert( 0, otInteger, otFloat, "tanh" );

    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( tanh(ob_int_val( ob_argv(0) )));
    }
    else{
        return (Object *)gc_new_float( tanh(float_argv(0) ));
    }
}







