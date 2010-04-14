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
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hsettimeout);
HYBRIS_DEFINE_FUNCTION(hconnect);
HYBRIS_DEFINE_FUNCTION(hserver);
HYBRIS_DEFINE_FUNCTION(haccept);
HYBRIS_DEFINE_FUNCTION(hrecv);
HYBRIS_DEFINE_FUNCTION(hsend);
HYBRIS_DEFINE_FUNCTION(hclose);

extern "C" named_function_t hybris_module_functions[] = {
	{ "settimeout", hsettimeout },
	{ "connect", hconnect },
	{ "server", hserver },
	{ "accept", haccept },
	{ "recv", hrecv },
	{ "send", hsend },
	{ "close", hclose },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hsettimeout){
	if( HYB_ARGC() != 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'settimeout' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );
	HYB_TYPE_ASSERT( HYB_ARGV(1), otInteger );

	struct timeval tout = { 0 , INT_ARGV(1) };

	setsockopt( INT_ARGV(0), SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
	setsockopt( INT_ARGV(0), SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );

	return OB_DOWNCAST( MK_INT_OBJ(0) );
}

HYBRIS_DEFINE_FUNCTION(hconnect){
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

	Object *_return = NULL;
	if( HYB_ARGC() >= 2 ){
		int sd = socket( AF_INET, SOCK_STREAM, 0 );
		if( sd <= 0 ){
			return OB_DOWNCAST( MK_INT_OBJ(-1) );
		}
		if( HYB_ARGC() == 3 ){
			HYB_TYPE_ASSERT( HYB_ARGV(2), otInteger );
			struct timeval tout = { 0 , INT_ARGV(2) };

			setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
			setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
		}

		struct sockaddr_in server;
		hostent * host = gethostbyname( STRING_ARGV(0).c_str() );
		if( host ){
			bzero( &server, sizeof(server) );
			server.sin_family      = AF_INET;
			server.sin_port        = htons( INT_ARGV(1) );
			bcopy( host->h_addr, &(server.sin_addr.s_addr), host->h_length );
		}

		if( connect( sd, (struct sockaddr*)&server, sizeof(server) ) != 0 ){
			return OB_DOWNCAST( MK_INT_OBJ(-1) );
		}

		_return = OB_DOWNCAST( MK_INT_OBJ(sd) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'connect' requires at least 2 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hserver){
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	Object *_return = NULL;
	if( HYB_ARGC() >= 1 ){
		int sd = socket( AF_INET, SOCK_STREAM, 0 );
		if( sd <= 0 ){
			return OB_DOWNCAST( MK_INT_OBJ(-1) );
		}
		if( HYB_ARGC() == 2 ){
			HYB_TYPE_ASSERT( HYB_ARGV(1), otInteger );
			struct timeval tout = { 0 , INT_ARGV(1) };

			setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
			setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
		}

		short int port = INT_ARGV(0);
		struct    sockaddr_in servaddr;

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family      = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port        = htons(port);

		if ( bind( sd, (struct sockaddr *)&servaddr, sizeof(servaddr) ) < 0 ) {
			return OB_DOWNCAST( MK_INT_OBJ(-1) );
		}

		if ( listen( sd, 1024 ) < 0 ) {
			return OB_DOWNCAST( MK_INT_OBJ(-1) );
		}

		_return = OB_DOWNCAST( MK_INT_OBJ(sd) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'server' requires at least 1 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(haccept){
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	Object *_return = NULL;
	if( HYB_ARGC() >= 1 ){
		int sd = INT_ARGV(0),
		    csd;

		if( (csd = accept( sd, NULL, NULL) ) < 0 ) {
			return OB_DOWNCAST( MK_INT_OBJ(-1) );
		}

		_return = OB_DOWNCAST( MK_INT_OBJ(csd) );
	}
	else{
		hyb_throw( H_ET_SYNTAX, "function 'accept' requires 1 parameters (called with %d)", HYB_ARGC() );
	}

    return _return;
}

HYBRIS_DEFINE_FUNCTION(hrecv){
	if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'recv' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	int sd = INT_ARGV(0);
	size_t size = 0;
	Object *object   = HYB_ARGV(1);

	/* explicit size declaration */
	if( HYB_ARGC() == 3 ){
		size = INT_ARGV(2);
	}

	return ob_from_fd( object, sd, size );
}

HYBRIS_DEFINE_FUNCTION(hsend){
	if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'send' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	int sd = INT_ARGV(0);
	size_t size = 0;
	Object *object   = HYB_ARGV(1);

	/* explicit size declaration */
	if( HYB_ARGC() == 3 ){
		size = INT_ARGV(2);
	}

	return ob_to_fd( object, sd, size );
}

HYBRIS_DEFINE_FUNCTION(hclose){
    if( HYB_ARGC() ){
		close( INT_ARGV(0) );
    }
    return OB_DOWNCAST( MK_INT_OBJ(0) );
}
