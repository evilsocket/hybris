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
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'acos' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)acos(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)acos(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hasin){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'asin' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)asin(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)asin(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'atan' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)atan(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)atan(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hatan2){
    if( data->size() != 2 ){
		hyb_syntax_error( "function 'atan2' requires 2 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );
    hyb_type_assert( data->at(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( data->at(0)->type == H_OT_INT ){
        a = (double)data->at(0)->value.m_integer;
    }
    else{
        a = data->at(0)->value.m_double;
    }
    if( data->at(1)->type == H_OT_INT ){
        b = (double)data->at(1)->value.m_integer;
    }
    else{
        b = data->at(1)->value.m_double;
    }
    return new Object((double)atan2(a,b));
}

HYBRIS_DEFINE_FUNCTION(hceil){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'ceil' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object( static_cast<long>( ceil(data->at(0)->value.m_integer) ) );
    }
    else{
        return new Object(static_cast<long>( ceil(data->at(0)->value.m_double) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hcos){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'cos' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)cos(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)cos(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hcosh){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'cosh' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)cosh(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)cosh(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hexp){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'exp' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)exp(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)exp(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hfabs){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'fabs' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)fabs(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)fabs(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hfloor){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'floor' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object( static_cast<long>( floor(data->at(0)->value.m_integer) ) );
    }
    else{
        return new Object( static_cast<long>( floor(data->at(0)->value.m_double) ) );
    }
}

HYBRIS_DEFINE_FUNCTION(hfmod){
    if( data->size() != 2 ){
		hyb_syntax_error( "function 'fmod' requires 2 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );
    hyb_type_assert( data->at(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( data->at(0)->type == H_OT_INT ){
        a = (double)data->at(0)->value.m_integer;
    }
    else{
        a = data->at(0)->value.m_double;
    }
    if( data->at(1)->type == H_OT_INT ){
        b = (double)data->at(1)->value.m_integer;
    }
    else{
        b = data->at(1)->value.m_double;
    }
    return new Object((double)fmod(a,b));
}

HYBRIS_DEFINE_FUNCTION(hlog){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'hlog' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)log(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)log(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hlog10){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'log10' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)log10(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)log10(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hpow){
    if( data->size() != 2 ){
		hyb_syntax_error( "function 'pow' requires 2 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );
    hyb_type_assert( data->at(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( data->at(0)->type == H_OT_INT ){
        a = (double)data->at(0)->value.m_integer;
    }
    else{
        a = data->at(0)->value.m_double;
    }
    if( data->at(1)->type == H_OT_INT ){
        b = (double)data->at(1)->value.m_integer;
    }
    else{
        b = data->at(1)->value.m_double;
    }
    return new Object((double)pow(a,b));
}

HYBRIS_DEFINE_FUNCTION(hsin){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'sin' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)sin(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)sin(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hsinh){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'sinh' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)sinh(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)sinh(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(hsqrt){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'sqrt' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)sqrt(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)sqrt(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(htan){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'tan' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)tan(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)tan(data->at(0)->value.m_double));
    }
}

HYBRIS_DEFINE_FUNCTION(htanh){
    if( data->size() != 1 ){
		hyb_syntax_error( "function 'tanh' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->type == H_OT_INT ){
        return new Object((double)tanh(data->at(0)->value.m_integer));
    }
    else{
        return new Object((double)tanh(data->at(0)->value.m_double));
    }
}






