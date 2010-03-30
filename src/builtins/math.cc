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
#include "context.h"
#include <math.h>

HYBRIS_BUILTIN(hacos){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'acos' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)acos(data->at(0)->xint));
    }
    else{
        return new Object((double)acos(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hasin){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'asin' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)asin(data->at(0)->xint));
    }
    else{
        return new Object((double)asin(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hatan){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'atan' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)atan(data->at(0)->xint));
    }
    else{
        return new Object((double)atan(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hatan2){
    if( data->size() != 2 ){
		hybris_syntax_error( "function 'atan2' requires 2 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );
    htype_assert( data->at(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( data->at(0)->xtype == H_OT_INT ){
        a = (double)data->at(0)->xint;
    }
    else{
        a = data->at(0)->xfloat;
    }
    if( data->at(1)->xtype == H_OT_INT ){
        b = (double)data->at(1)->xint;
    }
    else{
        b = data->at(1)->xfloat;
    }
    return new Object((double)atan2(a,b));
}

HYBRIS_BUILTIN(hceil){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'ceil' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object( static_cast<long>( ceil(data->at(0)->xint) ) );
    }
    else{
        return new Object(static_cast<long>( ceil(data->at(0)->xfloat) ) );
    }
}

HYBRIS_BUILTIN(hcos){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'cos' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)cos(data->at(0)->xint));
    }
    else{
        return new Object((double)cos(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hcosh){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'cosh' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)cosh(data->at(0)->xint));
    }
    else{
        return new Object((double)cosh(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hexp){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'exp' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)exp(data->at(0)->xint));
    }
    else{
        return new Object((double)exp(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hfabs){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'fabs' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)fabs(data->at(0)->xint));
    }
    else{
        return new Object((double)fabs(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hfloor){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'floor' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object( static_cast<long>( floor(data->at(0)->xint) ) );
    }
    else{
        return new Object( static_cast<long>( floor(data->at(0)->xfloat) ) );
    }
}

HYBRIS_BUILTIN(hfmod){
    if( data->size() != 2 ){
		hybris_syntax_error( "function 'fmod' requires 2 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );
    htype_assert( data->at(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( data->at(0)->xtype == H_OT_INT ){
        a = (double)data->at(0)->xint;
    }
    else{
        a = data->at(0)->xfloat;
    }
    if( data->at(1)->xtype == H_OT_INT ){
        b = (double)data->at(1)->xint;
    }
    else{
        b = data->at(1)->xfloat;
    }
    return new Object((double)fmod(a,b));
}

HYBRIS_BUILTIN(hlog){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'hlog' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)log(data->at(0)->xint));
    }
    else{
        return new Object((double)log(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hlog10){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'log10' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)log10(data->at(0)->xint));
    }
    else{
        return new Object((double)log10(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hpow){
    if( data->size() != 2 ){
		hybris_syntax_error( "function 'pow' requires 2 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );
    htype_assert( data->at(1), H_OT_INT, H_OT_FLOAT );

    double a, b;

    if( data->at(0)->xtype == H_OT_INT ){
        a = (double)data->at(0)->xint;
    }
    else{
        a = data->at(0)->xfloat;
    }
    if( data->at(1)->xtype == H_OT_INT ){
        b = (double)data->at(1)->xint;
    }
    else{
        b = data->at(1)->xfloat;
    }
    return new Object((double)pow(a,b));
}

HYBRIS_BUILTIN(hsin){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'sin' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)sin(data->at(0)->xint));
    }
    else{
        return new Object((double)sin(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hsinh){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'sinh' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)sinh(data->at(0)->xint));
    }
    else{
        return new Object((double)sinh(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(hsqrt){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'sqrt' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)sqrt(data->at(0)->xint));
    }
    else{
        return new Object((double)sqrt(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(htan){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'tan' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)tan(data->at(0)->xint));
    }
    else{
        return new Object((double)tan(data->at(0)->xfloat));
    }
}

HYBRIS_BUILTIN(htanh){
    if( data->size() != 1 ){
		hybris_syntax_error( "function 'tanh' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT, H_OT_FLOAT );

    if( data->at(0)->xtype == H_OT_INT ){
        return new Object((double)tanh(data->at(0)->xint));
    }
    else{
        return new Object((double)tanh(data->at(0)->xfloat));
    }
}






