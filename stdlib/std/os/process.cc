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
#include <sys/types.h>
#include <sys/wait.h>
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hexec);
HYBRIS_DEFINE_FUNCTION(hfork);
HYBRIS_DEFINE_FUNCTION(hgetpid);
HYBRIS_DEFINE_FUNCTION(hwait);
HYBRIS_DEFINE_FUNCTION(hpopen);
HYBRIS_DEFINE_FUNCTION(hpclose);
HYBRIS_DEFINE_FUNCTION(hexit);

extern "C" named_function_t hybris_module_functions[] = {
	{ "exec", hexec },
	{ "fork", hfork },
	{ "getpid", hgetpid },
	{ "wait", hwait },
	{ "popen", hpopen },
	{ "pclose", hpclose },
	{ "exit", hexit },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hexec){
	hyb_type_assert( data->at(0), H_OT_STRING );
    Object *_return = NULL;
    if( data->size() ){
        _return = new Object( static_cast<long>( system( data->at(0)->value.m_string.c_str() ) ) );
    }
	else{
		hyb_syntax_error( "function 'exec' requires 1 parameter (called with %d)", data->size() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hfork){
    return new Object( static_cast<long>( fork() ) );
}

HYBRIS_DEFINE_FUNCTION(hgetpid){
    return new Object( static_cast<long>( getpid() ) );
}

HYBRIS_DEFINE_FUNCTION(hwait){
	if( data->size() != 1 ){
		hyb_syntax_error( "function 'wait' requires 1 parameter (called with %d)", data->size() );
	}
	hyb_type_assert( data->at(0), H_OT_INT );
    return new Object( static_cast<long>( wait( &data->at(0)->value.m_integer ) ) );
}

HYBRIS_DEFINE_FUNCTION(hpopen){
	hyb_type_assert( data->at(0), H_OT_STRING );
	hyb_type_assert( data->at(1), H_OT_STRING );

    if( data->size() == 2 ){
        return new Object( reinterpret_cast<long>( popen( data->at(0)->value.m_string.c_str(), data->at(1)->value.m_string.c_str() ) ) );
    }
	else{
		hyb_syntax_error( "function 'popen' requires 2 parameters (called with %d)", data->size() );
	}
}

HYBRIS_DEFINE_FUNCTION(hpclose){
	hyb_type_assert( data->at(0), H_OT_INT );
    if( data->size() ){
		pclose( (FILE *)data->at(0)->value.m_integer );
    }
    return new Object(static_cast<long>(0));
}

HYBRIS_DEFINE_FUNCTION(hexit){
    int code = 0;
    if( data->size() > 0 ){
		hyb_type_assert( data->at(0), H_OT_INT );
		code = data->at(0)->value.m_integer;
	}
	exit(code);

    return new Object(static_cast<long>(0));
}