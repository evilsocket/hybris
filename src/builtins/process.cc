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
#include <sys/types.h>
#include <sys/wait.h>


HYBRIS_BUILTIN(hexec){
	htype_assert( data->at(0), H_OT_STRING );
    Object *_return = NULL;
    if( data->size() ){
        _return = new Object( static_cast<long>( system( data->at(0)->value.m_string.c_str() ) ) );
    }
	else{
		hybris_syntax_error( "function 'exec' requires 1 parameter (called with %d)", data->size() );
	}
    return _return;
}

HYBRIS_BUILTIN(hfork){
    return new Object( static_cast<long>( fork() ) );
}

HYBRIS_BUILTIN(hgetpid){
    return new Object( static_cast<long>( getpid() ) );
}

HYBRIS_BUILTIN(hwait){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'wait' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );
    return new Object( static_cast<long>( wait( &data->at(0)->value.m_integer ) ) );
}

HYBRIS_BUILTIN(hpopen){
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );

    if( data->size() == 2 ){
        return new Object( reinterpret_cast<long>( popen( data->at(0)->value.m_string.c_str(), data->at(1)->value.m_string.c_str() ) ) );
    }
	else{
		hybris_syntax_error( "function 'popen' requires 2 parameters (called with %d)", data->size() );
	}
}

HYBRIS_BUILTIN(hpclose){
	htype_assert( data->at(0), H_OT_INT );
    if( data->size() ){
		pclose( (FILE *)data->at(0)->value.m_integer );
    }
    return new Object(static_cast<long>(0));
}

HYBRIS_BUILTIN(hexit){
    int code = 0;
    if( data->size() > 0 ){
		htype_assert( data->at(0), H_OT_INT );
		code = data->at(0)->value.m_integer;
	}
	exit(code);

    return new Object(static_cast<long>(0));
}
