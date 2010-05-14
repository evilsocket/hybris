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
	{ "acos",  hacos,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "asin",  hasin,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "atan",  hatan,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "atan2", hatan2, H_REQ_ARGC( 2 ), { H_REQ_TYPES( otInteger, otFloat ), H_REQ_TYPES( otInteger, otFloat ) } },
	{ "ceil",  hceil,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "cos",   hcos,   H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "cosh",  hcosh,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "exp",   hexp,   H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "fabs",  hfabs,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "floor", hfloor, H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "fmod",  hfmod,  H_REQ_ARGC( 2 ), { H_REQ_TYPES( otInteger, otFloat ), H_REQ_TYPES( otInteger, otFloat ) } },
	{ "log",   hlog,   H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "log10", hlog10, H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "pow",   hpow,   H_REQ_ARGC( 2 ), { H_REQ_TYPES( otInteger, otFloat ), H_REQ_TYPES( otInteger, otFloat ) } },
	{ "sin",   hsin,   H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "sinh",  hsinh,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "sqrt",  hsqrt,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "tan",   htan,   H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },
	{ "tanh",  htanh,  H_REQ_ARGC( 1 ), { H_REQ_TYPES( otInteger, otFloat ) } },

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
	if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( acos(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( acos(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hasin){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( asin(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( asin(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( atan(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( atan(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan2){
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
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_integer( ceil(int_argv(0) ) );
    }
    else{
        return (Object *)gc_new_integer( ceil(float_argv(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hcos){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( cos(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( cos(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hcosh){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( cosh(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( cosh(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hexp){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( exp(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( exp(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfabs){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( fabs(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( fabs(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hfloor){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_integer( floor(int_argv(0) ) );
    }
    else{
        return (Object *)gc_new_integer( floor(float_argv(0) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hfmod){
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
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( log(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( log(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hlog10){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( log10(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( log10(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hpow){
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
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sin(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sin(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsinh){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sinh(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sinh(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(hsqrt){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( sqrt(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( sqrt(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htan){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( tan(int_argv(0) ));
    }
    else{
        return (Object *)gc_new_float( tan(float_argv(0) ));
    }
}

HYBRIS_DEFINE_FUNCTION(htanh){
    if( ob_is_int( ob_argv(0) ) ){
        return (Object *)gc_new_float( tanh(ob_int_val( ob_argv(0) )));
    }
    else{
        return (Object *)gc_new_float( tanh(float_argv(0) ));
    }
}







