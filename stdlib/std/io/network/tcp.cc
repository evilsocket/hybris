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
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	bool isEOF	    = false,
		 isEOL 		= false;
    Object *_return = NULL;
    if( HYB_ARGC() >= 2 ){
		int sd           = INT_ARGV(0);
		Object *object   = HYB_ARGV(1);
		unsigned int size, read = 0, i, n;
		char c;
		/* explicit size declaration */
		if( HYB_ARGC() == 3 ){
			size = INT_ARGV(2);
			switch( object->type->code ){
				case otInteger : read = recv( sd, &(INT_VALUE(object)), size, 0 ); break;
				case otFloat   : read = recv( sd, &(FLOAT_VALUE(object)), size, 0 );  break;
				case otChar    : read = recv( sd, &(CHAR_VALUE(object)), size, 0 );    break;
				case otString  :
					for( i = 0; i < size; i++ ){
						if( recv( sd, &c, sizeof(char), 0 ) > 0 ){
							read++ ;
							STRING_VALUE(object) += c;
							STRING_UPCAST(object)->items++;
						}
						else{
							read = 0;
							break;
						}
					}
				break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly deserialize type %s", object->type->name );
			}
			object->type->size = size;
		}
		/* handle size by type */
		else{
			switch( object->type->code ){
				case otInteger : read = recv( sd, &(INT_VALUE(object)), sizeof(long), 0 ); break;
				case otFloat   : read = recv( sd, &(FLOAT_VALUE(object)), sizeof(double), 0 );  break;
				case otChar    : read = recv( sd, &(CHAR_VALUE(object)), sizeof(char), 0 );    break;
				case otString  :
					STRING_VALUE(object) = "";
					while( !isEOL && !isEOF ){
						n = recv( sd, &c, sizeof(char), 0 );
				 		if( n < 1 ){
							isEOF = true;
						}
						else if( c == '\n' ){
							isEOL = true;
						}
						else {
							if( c != '\r' ){
								STRING_VALUE(object) += c;
								STRING_UPCAST(object)->items++;
							}
						}
					}
					object->type->size = read;
				break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly deserialize type %s", object->type->name );
			}
		}
        _return = OB_DOWNCAST( MK_INT_OBJ(read) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'recv' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}

    return _return;
}

HYBRIS_DEFINE_FUNCTION(hsend){
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

    Object *_return = NULL;
    if( HYB_ARGC() >= 2 ){
		int sd           = INT_ARGV(0);
		Object *object   = HYB_ARGV(1);
		unsigned int size, written = 0, i;
		char c;
		if( HYB_ARGC() == 3 ){
			size = INT_ARGV(2);
			switch( object->type->code ){
				case otInteger : written = send( sd, &(INT_VALUE(object)), size, 0 ); break;
				case otFloat   : written = send( sd, &(FLOAT_VALUE(object)), size, 0 );  break;
				case otChar    : written = send( sd, &(CHAR_VALUE(object)), size, 0 );    break;
				case otString  : written = send( sd, STRING_VALUE(object).c_str(), size, 0 ); break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly serialize type %s", object->type->name );
			}
		}
		else{
			switch( object->type->code ){
				case otInteger : written = send( sd, &(INT_VALUE(object)), sizeof(long), 0 ); break;
				case otFloat   : written = send( sd, &(FLOAT_VALUE(object)), sizeof(double), 0 );  break;
				case otChar    : written = send( sd, &(CHAR_VALUE(object)), sizeof(char), 0 );    break;
				case otString  : written = send( sd, STRING_VALUE(object).c_str(), STRING_UPCAST(object)->items, 0 ); break;

				case otVector   :
					for( i = 0; i < VECTOR_UPCAST(object)->items; i++ ){
						Object *element = VECTOR_UPCAST(object)->value[i];
						switch( element->type->code ){
							case otInteger : written += send( sd, &(INT_VALUE(element)), sizeof(long), 0 ); break;
							case otFloat   : written += send( sd, &(FLOAT_VALUE(element)), sizeof(double), 0 );  break;
							case otChar    : written += send( sd, &(CHAR_VALUE(element)), sizeof(char), 0 );    break;
							case otString  : written += send( sd, STRING_VALUE(element).c_str(), STRING_UPCAST(element)->items, 0 ); break;

							default :
                                hyb_throw( H_ET_GENERIC, "can not directly serialize nested type %s", element->type->name );
						}
					}
				break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly serialize type %s", object->type->name );
			}
		}
        _return = OB_DOWNCAST( MK_INT_OBJ(written) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'send' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}

    return _return;
}

HYBRIS_DEFINE_FUNCTION(hclose){
    if( HYB_ARGC() ){
		close( INT_ARGV(0) );
    }
    return OB_DOWNCAST( MK_INT_OBJ(0) );
}
