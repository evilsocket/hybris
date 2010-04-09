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
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)acos(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)acos(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hasin){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'asin' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)asin(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)asin(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'atan' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)atan(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)atan(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan2){
    if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'atan2' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );
    HYB_TYPES_ASSERT( HYB_ARGV(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( HYB_ARGV(0)->type == H_OT_INT ){
        a = (double)HYB_ARGV(0)->value.m_integer;
    }
    else{
        a = HYB_ARGV(0)->value.m_double;
    }
    if( HYB_ARGV(1)->type == H_OT_INT ){
        b = (double)HYB_ARGV(1)->value.m_integer;
    }
    else{
        b = HYB_ARGV(1)->value.m_double;
    }
    return new Object((double)atan2(a,b));
}

HYBRIS_DEFINE_FUNCTION(hceil){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'ceil' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object( static_cast<long>( ceil(HYB_ARGV(0)->value.m_integer) ) );
    }
    else{
        return new Object(static_cast<long>( ceil(HYB_ARGV(0)->value.m_double) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hcos){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'cos' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)cos(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)cos(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hcosh){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'cosh' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)cosh(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)cosh(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hexp){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'exp' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)exp(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)exp(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hfabs){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fabs' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)fabs(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)fabs(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hfloor){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'floor' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object( static_cast<long>( floor(HYB_ARGV(0)->value.m_integer) ) );
    }
    else{
        return new Object( static_cast<long>( floor(HYB_ARGV(0)->value.m_double) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hfmod){
    if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'fmod' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );
    HYB_TYPES_ASSERT( HYB_ARGV(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( HYB_ARGV(0)->type == H_OT_INT ){
        a = (double)HYB_ARGV(0)->value.m_integer;
    }
    else{
        a = HYB_ARGV(0)->value.m_double;
    }
    if( HYB_ARGV(1)->type == H_OT_INT ){
        b = (double)HYB_ARGV(1)->value.m_integer;
    }
    else{
        b = HYB_ARGV(1)->value.m_double;
    }
    return new Object((double)fmod(a,b));
}

HYBRIS_DEFINE_FUNCTION(hlog){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'hlog' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)log(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)log(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hlog10){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'log10' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)log10(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)log10(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hpow){
    if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'pow' requires 2 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );
    HYB_TYPES_ASSERT( HYB_ARGV(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( HYB_ARGV(0)->type == H_OT_INT ){
        a = (double)HYB_ARGV(0)->value.m_integer;
    }
    else{
        a = HYB_ARGV(0)->value.m_double;
    }
    if( HYB_ARGV(1)->type == H_OT_INT ){
        b = (double)HYB_ARGV(1)->value.m_integer;
    }
    else{
        b = HYB_ARGV(1)->value.m_double;
    }
    return new Object((double)pow(a,b));
}

HYBRIS_DEFINE_FUNCTION(hsin){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sin' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)sin(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)sin(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hsinh){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sinh' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)sinh(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)sinh(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hsqrt){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sqrt' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)sqrt(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)sqrt(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(htan){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'tan' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)tan(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)tan(HYB_ARGV(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(htanh){
    if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'tanh' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_FLOAT );

    if( HYB_ARGV(0)->type == H_OT_INT ){
        return new Object((double)tanh(HYB_ARGV(0)->value.m_integer));
    }
    else{
        return new Object((double)tanh(HYB_ARGV(0)->value.m_double));
    }
}






