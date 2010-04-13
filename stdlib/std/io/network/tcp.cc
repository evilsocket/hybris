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
	HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type );
	HYB_TYPE_ASSERT( HYB_ARGV(1), Integer_Type );

	struct timeval tout = { 0 , (long)(*HYB_ARGV(1)) };

	setsockopt( (long)(*HYB_ARGV(0)), SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
	setsockopt( (long)(*HYB_ARGV(0)), SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );

	return MK_INT_OBJ(0);
}

HYBRIS_DEFINE_FUNCTION(hconnect){
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

	Object *_return = NULL;
	if( HYB_ARGC() >= 2 ){
		int sd = socket( AF_INET, SOCK_STREAM, 0 );
		if( sd <= 0 ){
			return MK_INT_OBJ(-1);
		}
		if( HYB_ARGC() == 3 ){
			HYB_TYPE_ASSERT( HYB_ARGV(2), Integer_Type );
			struct timeval tout = { 0 , (long)(*HYB_ARGV(2)) };

			setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
			setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
		}

		struct sockaddr_in server;
		hostent * host = gethostbyname( (const char *)(*HYB_ARGV(0)) );
		if( host ){
			bzero( &server, sizeof(server) );
			server.sin_family      = AF_INET;
			server.sin_port        = htons(HYB_ARGV(1)->value.m_integer);
			bcopy( host->h_addr, &(server.sin_addr.s_addr), host->h_length );
		}

		if( connect( sd, (struct sockaddr*)&server, sizeof(server) ) != 0 ){
			return MK_INT_OBJ(-1);
		}

		_return = MK_INT_OBJ(sd);
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'connect' requires at least 2 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hserver){
	HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type );

	Object *_return = NULL;
	if( HYB_ARGC() >= 1 ){
		int sd = socket( AF_INET, SOCK_STREAM, 0 );
		if( sd <= 0 ){
			return MK_INT_OBJ(-1);
		}
		if( HYB_ARGC() == 2 ){
			HYB_TYPE_ASSERT( HYB_ARGV(1), Integer_Type );
			struct timeval tout = { 0 , (long)(*HYB_ARGV(1)) };

			setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
			setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
		}

		short int port = (long)(*HYB_ARGV(0));
		struct    sockaddr_in servaddr;

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family      = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port        = htons(port);

		if ( bind( sd, (struct sockaddr *)&servaddr, sizeof(servaddr) ) < 0 ) {
			return MK_INT_OBJ(-1);
		}

		if ( listen( sd, 1024 ) < 0 ) {
			return MK_INT_OBJ(-1);
		}

		_return = MK_INT_OBJ(sd);
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'server' requires at least 1 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(haccept){
	HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type );

	Object *_return = NULL;
	if( HYB_ARGC() >= 1 ){
		int sd = (long)(*HYB_ARGV(0)),
		    csd;

		if( (csd = accept( sd, NULL, NULL) ) < 0 ) {
			return MK_INT_OBJ(-1);
		}

		_return = MK_INT_OBJ(csd);
	}
	else{
		hyb_throw( H_ET_SYNTAX, "function 'accept' requires 1 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hrecv){
	HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type );

	bool isEOF	    = false,
		 isEOL 		= false;
    Object *_return = NULL;
    if( HYB_ARGC() >= 2 ){
		int sd           = (long)(*HYB_ARGV(0));
		Object *object   = HYB_ARGV(1);
		unsigned int size, read = 0, i, n;
		char c;
		/* explicit size declaration */
		if( HYB_ARGC() == 3 ){
			size = HYB_ARGV(2)->value.m_integer;
			switch( object->type ){
				case Integer_Type    : read = recv( sd, &object->value.m_integer, size, 0 ); break;
				case Float_Type  : read = recv( sd, &object->value.m_double, size, 0 );  break;
				case H_OT_CHAR   : read = recv( sd, &object->value.m_char, size, 0 );    break;
				case H_OT_STRING :
					for( i = 0; i < size; i++ ){
						if( recv( sd, &c, sizeof(char), 0 ) > 0 ){
							read++ ;
							object->value.m_string += c;
						}
						else{
							read = 0;
							break;
						}
					}
				break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly deserialize type %s", Object::type_name(object) );
			}
			object->size = size;
		}
		/* handle size by type */
		else{
			switch( object->type ){
				case Integer_Type    : object->size = read = recv( sd, &object->value.m_integer,   sizeof(long), 0 ); break;
				case Float_Type  : object->size = read = recv( sd, &object->value.m_double, sizeof(double), 0 );  break;
				case H_OT_CHAR   : object->size = read = recv( sd, &object->value.m_char,  sizeof(char), 0 );     break;
				case H_OT_STRING :
					object->value.m_string = "";
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
								read++;
								object->value.m_string += c;
							}
						}
					}
					object->size = read;
				break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly deserialize type %s", Object::type_name(object) );
			}
		}
        _return = MK_INT_OBJ(read);
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'recv' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}

    return _return;
}

HYBRIS_DEFINE_FUNCTION(hsend){
	HYB_TYPE_ASSERT( HYB_ARGV(0), Integer_Type );

    Object *_return = NULL;
    if( HYB_ARGC() >= 2 ){
		int sd           = (long)(*HYB_ARGV(0));
		Object *object   = HYB_ARGV(1);
		unsigned int size, written = 0, i;
		char c;
		if( HYB_ARGC() == 3 ){
			size = HYB_ARGV(2)->value.m_integer;
			switch( object->type ){
				case Integer_Type    : written = send( sd, &object->value.m_integer,   size, 0 ); break;
				case Float_Type  : written = send( sd, &object->value.m_double, size, 0 ); break;
				case H_OT_CHAR   : written = send( sd, &object->value.m_char,  size, 0 ); break;
				case H_OT_STRING : written = send( sd, object->value.m_string.c_str(), size, 0 ); break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly serialize type %s", Object::type_name(object) );
			}
		}
		else{
			switch( object->type ){
				case Integer_Type    : written = send( sd, &object->value.m_integer,   sizeof(long), 0 ); break;
				case Float_Type  : written = send( sd, &object->value.m_double, sizeof(double), 0 ); break;
				case H_OT_CHAR   : written = send( sd, &object->value.m_char,  sizeof(char), 0 ); break;
				case H_OT_STRING : written = send( sd, object->value.m_string.c_str(), object->size, 0 ); break;
				case H_OT_ARRAY  :
					for( i = 0; i < object->size; i++ ){
						Object *element = object->value.m_array[i];
						switch( element->type ){
							case Integer_Type    : written += send( sd, &element->value.m_integer,   sizeof(long), 0 ); break;
							case Float_Type  : written += send( sd, &element->value.m_double, sizeof(double), 0 ); break;
							case H_OT_CHAR   : written += send( sd, &element->value.m_char,  sizeof(char), 0 ); break;
							case H_OT_STRING : written += send( sd, element->value.m_string.c_str(), element->size, 0 ); break;

							default :
                                hyb_throw( H_ET_GENERIC, "can not directly serialize nested type %s", Object::type_name(element) );
						}
					}
				break;

				default :
                     hyb_throw( H_ET_GENERIC, "can not directly serialize type %s", Object::type_name(object) );
			}
		}
        _return = MK_INT_OBJ(written);
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'send' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hclose){
    if( HYB_ARGC() ){
		close( (long)(*HYB_ARGV(0)) );
    }
    return MK_INT_OBJ(0);
}
