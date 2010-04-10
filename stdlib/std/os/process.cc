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
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );
    Object *_return = NULL;
    if( HYB_ARGC() ){
        _return = MK_INT_OBJ( system( (const char *)(*HYB_ARGV(0)) ) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'exec' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hfork){
    return MK_INT_OBJ( fork() );
}

HYBRIS_DEFINE_FUNCTION(hgetpid){
    return MK_INT_OBJ( getpid() );
}

HYBRIS_DEFINE_FUNCTION(hwait){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'wait' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );

	return MK_INT_OBJ( wait( &HYB_ARGV(0)->value.m_integer ) );
}

HYBRIS_DEFINE_FUNCTION(hpopen){
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );
	HYB_TYPE_ASSERT( HYB_ARGV(1), H_OT_STRING );

    if( HYB_ARGC() == 2 ){
        return  PTR_TO_INT_OBJ( popen( (const char *)(*HYB_ARGV(0)), (const char *)(*HYB_ARGV(1)) ) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'popen' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
}

HYBRIS_DEFINE_FUNCTION(hpclose){
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );
    if( HYB_ARGC() ){
		pclose( (FILE *)(long)(*HYB_ARGV(0)) );
    }
    return MK_INT_OBJ(0);
}

HYBRIS_DEFINE_FUNCTION(hexit){
    int code = 0;
    if( HYB_ARGC() > 0 ){
		HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );
		code = (long)(*HYB_ARGV(0));
	}
	exit(code);

    return MK_INT_OBJ(0);
}
