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

HYBRIS_DEFINE_FUNCTION(henv);
HYBRIS_DEFINE_FUNCTION(hexec);
HYBRIS_DEFINE_FUNCTION(hfork);
HYBRIS_DEFINE_FUNCTION(hgetpid);
HYBRIS_DEFINE_FUNCTION(hwait);
HYBRIS_DEFINE_FUNCTION(hpopen);
HYBRIS_DEFINE_FUNCTION(hpclose);
HYBRIS_DEFINE_FUNCTION(hexit);
HYBRIS_DEFINE_FUNCTION(hkill);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "env",    henv,    H_NO_ARGS },
	{ "exec",   hexec,   H_REQ_ARGC(1),   { H_REQ_TYPES(otString) } },
	{ "fork",   hfork,   H_NO_ARGS },
	{ "getpid", hgetpid, H_NO_ARGS },
	{ "wait",   hwait,   H_REQ_ARGC(1),   { H_REQ_TYPES(otInteger) } },
	{ "popen",  hpopen,  H_REQ_ARGC(2),   { H_REQ_TYPES(otString), H_REQ_TYPES(otString) } },
	{ "pclose", hpclose, H_REQ_ARGC(1),   { H_REQ_TYPES(otHandle) } },
	{ "exit",   hexit,   H_REQ_ARGC(0,1), { H_REQ_TYPES(otInteger) } },
	{ "kill",   hkill,   H_REQ_ARGC(2),   { H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "", NULL }
};

static char **__envp;

extern "C" void hybris_module_init( vm_t * vm ){
	extern vm_t  *__hyb_vm;

	/*
	 * This module is linked against libhybris.so.1 which contains a compiled
	 * parser.cpp, which has an uninitialized __hyb_vm pointer.
	 * The real vm_t is passed to this function by the core, so we have to initialize
	 * the pointer with the right data.
	 */
	__hyb_vm = vm;
	__envp   = vm->env;

	HYBRIS_DEFINE_CONSTANT( vm, "SIGHUP", gc_new_integer(SIGHUP) ); /* Hangup (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGINT", gc_new_integer(SIGINT) ); /* Interrupt (ANSI).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGQUIT", gc_new_integer(SIGQUIT) ); /* Quit (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGILL", gc_new_integer(SIGILL) ); /* Illegal instruction (ANSI).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGTRAP", gc_new_integer(SIGTRAP) ); /* Trace trap (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGABRT", gc_new_integer(SIGABRT) ); /* Abort (ANSI).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGIOT", gc_new_integer(SIGIOT) ); /* IOT trap (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGBUS", gc_new_integer(SIGBUS) ); /* BUS error (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGFPE", gc_new_integer(SIGFPE) ); /* Floating-point exception (ANSI).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGKILL", gc_new_integer(SIGKILL) ); /* Kill, unblockable (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGSEGV", gc_new_integer(SIGSEGV) ); /* Segmentation violation (ANSI).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGPIPE", gc_new_integer(SIGPIPE) ); /* Broken pipe (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGALRM", gc_new_integer(SIGALRM) ); /* Alarm clock (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGTERM", gc_new_integer(SIGTERM) ); /* Termination (ANSI).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGSTKFLT", gc_new_integer(SIGSTKFLT) ); /* Stack fault.  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGCLD", gc_new_integer(SIGCHLD) ); /* Same as SIGCHLD (System V).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGCHLD", gc_new_integer(SIGCHLD) ); /* Child status has changed (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGCONT", gc_new_integer(SIGCONT) ); /* Continue (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGSTOP", gc_new_integer(SIGSTOP) ); /* Stop, unblockable (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGTSTP", gc_new_integer(SIGTSTP) ); /* Keyboard stop (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGTTIN", gc_new_integer(SIGTTIN) ); /* Background read from tty (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGTTOU", gc_new_integer(SIGTTOU) ); /* Background write to tty (POSIX).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGURG", gc_new_integer(SIGURG) ); /* Urgent condition on socket (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGXCPU", gc_new_integer(SIGXCPU) ); /* CPU limit exceeded (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGXFSZ", gc_new_integer(SIGXFSZ) ); /* File size limit exceeded (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGVTALRM", gc_new_integer(SIGVTALRM) ); /* Virtual alarm clock (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGPROF", gc_new_integer(SIGPROF) ); /* Profiling alarm clock (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGWINCH", gc_new_integer(SIGWINCH) ); /* Window size change (4.3 BSD, Sun).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGPOLL", gc_new_integer(SIGIO) ); /* Pollable event occurred (System V).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGIO", gc_new_integer(SIGIO) ); /* I/O now possible (4.2 BSD).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGPWR", gc_new_integer(SIGPWR) ); /* Power failure restart (System V).  */
	HYBRIS_DEFINE_CONSTANT( vm, "SIGSYS", gc_new_integer(SIGSYS) ); /* Bad system call.  */
}

HYBRIS_DEFINE_FUNCTION(henv){
	char   		 **penv;
	string		   env;
	Object  	  *map  = (Object *)gc_new_map();
	vector<string> duple;

	for( penv = __envp; *penv != 0; penv++ ){
		duple.clear();
		env = *penv;
		vm_str_split( env, "=", duple );
		if( duple.size() >= 2 ){
			ob_cl_set( map, (Object *)gc_new_string( duple[0].c_str() ), (Object *)gc_new_string( duple[1].c_str() ) );
		}
	}

	return map;
}

HYBRIS_DEFINE_FUNCTION(hexec){
    return ob_dcast( gc_new_integer( system( string_argv(0).c_str() ) ) );
}

HYBRIS_DEFINE_FUNCTION(hfork){
    return ob_dcast( gc_new_integer( fork() ) );
}

HYBRIS_DEFINE_FUNCTION(hgetpid){
    return ob_dcast( gc_new_integer( getpid() ) );
}

HYBRIS_DEFINE_FUNCTION(hwait){
	return ob_dcast( gc_new_integer( wait( &(int_argv(0)) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hpopen){
	return ob_dcast( gc_new_handle( popen( string_argv(0).c_str(), string_argv(1).c_str() ) ) );
}

HYBRIS_DEFINE_FUNCTION(hpclose){
   	if( handle_argv(0) == NULL ){
		return H_DEFAULT_ERROR;
	}

	pclose( (FILE *)handle_argv(0) );
	/*
	 * Make sure the handle is set to NULL to prevent SIGSEGV
	 * when p* functions try to use this file handle.
	 */
	ob_ref_ucast( ob_argv(0) )->value = NULL;
}

HYBRIS_DEFINE_FUNCTION(hexit){
    int code = 0;
    if( ob_argc() > 0 ){
		code = (long)int_argv(0);
	}
	exit(code);

    return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hkill){
	return (Object *)gc_new_integer( kill( int_argv(0), int_argv(1) ) );
}
