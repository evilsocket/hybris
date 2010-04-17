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

HYBRIS_EXPORTED_FUNCTIONS() {
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
	ob_type_assert( ob_argv(0), otString );
    Object *_return = NULL;
    if( ob_argc() ){
        _return = ob_dcast( gc_new_integer( system( string_argv(0).c_str() ) ) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'exec' requires 1 parameter (called with %d)", ob_argc() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hfork){
    return ob_dcast( gc_new_integer( fork() ) );
}

HYBRIS_DEFINE_FUNCTION(hgetpid){
    return ob_dcast( gc_new_integer( getpid() ) );
}

HYBRIS_DEFINE_FUNCTION(hwait){
	if( ob_argc() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'wait' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger );

	return ob_dcast( gc_new_integer( wait( &(int_argv(0)) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hpopen){
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );

    if( ob_argc() == 2 ){
        return  ob_dcast( PTR_TO_INT_OBJ( popen( string_argv(0).c_str(), string_argv(1).c_str() ) ) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'popen' requires 2 parameters (called with %d)", ob_argc() );
	}
}

HYBRIS_DEFINE_FUNCTION(hpclose){
	ob_type_assert( ob_argv(0), otInteger );
    if( ob_argc() ){
		pclose( (FILE *)int_argv(0) );
    }
    return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hexit){
    int code = 0;
    if( ob_argc() > 0 ){
		ob_type_assert( ob_argv(0), otInteger );
		code = (long)int_argv(0);
	}
	exit(code);

    return H_DEFAULT_RETURN;
}
