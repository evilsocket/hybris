#include "common.h"
#include "vmem.h"
#include "builtin.h"
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

HYBRIS_BUILTIN(hsettimeout){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'settimeout' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );
	htype_assert( data->at(1), H_OT_INT );

	struct timeval tout = { 0 , data->at(1)->xint };

	setsockopt( data->at(0)->xint, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
	setsockopt( data->at(0)->xint, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );

	return new Object(0);
}

HYBRIS_BUILTIN(hconnect){
	htype_assert( data->at(0), H_OT_STRING );

	Object *_return = NULL;
	if( data->size() >= 2 ){
		int sd = socket( AF_INET, SOCK_STREAM, 0 );
		if( sd <= 0 ){
			return new Object(-1);
		}
		if( data->size() == 3 ){
			htype_assert( data->at(2), H_OT_INT );
			struct timeval tout = { 0 , data->at(2)->xint };

			setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
			setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
		}

		struct sockaddr_in server;
		hostent * host = gethostbyname(data->at(0)->xstring.c_str());
		if( host ){
			bzero( &server, sizeof(server) );
			server.sin_family      = AF_INET;
			server.sin_port        = htons(data->at(1)->xint);
			bcopy( host->h_addr, &(server.sin_addr.s_addr), host->h_length );
		}

		if( connect( sd, (struct sockaddr*)&server, sizeof(server) ) != 0 ){
			return new Object(-1);
		}

		_return = new Object( sd );
    }
	else{
		hybris_syntax_error( "function 'connect' requires at least 2 parameters (called with %d)", data->size() );
	}
    return _return;
}

HYBRIS_BUILTIN(hserver){
	htype_assert( data->at(0), H_OT_INT );

	Object *_return = NULL;
	if( data->size() >= 1 ){
		int sd = socket( AF_INET, SOCK_STREAM, 0 );
		if( sd <= 0 ){
			return new Object(-1);
		}
		if( data->size() == 2 ){
			htype_assert( data->at(1), H_OT_INT );
			struct timeval tout = { 0 , data->at(1)->xint };

			setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
			setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
		}

		short int port = data->at(0)->xint;
		struct    sockaddr_in servaddr;
			
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family      = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port        = htons(port);
		
		if ( bind( sd, (struct sockaddr *)&servaddr, sizeof(servaddr) ) < 0 ) {
			return new Object(-1);
		}
		
		if ( listen( sd, 1024 ) < 0 ) {
			return new Object(-1);
		}

		_return = new Object( sd );
    }
	else{
		hybris_syntax_error( "function 'server' requires at least 1 parameters (called with %d)", data->size() );
	}
    return _return;
}

HYBRIS_BUILTIN(haccept){
	htype_assert( data->at(0), H_OT_INT );
	
	Object *_return = NULL;
	if( data->size() >= 1 ){
		int sd = data->at(0)->xint,
		    csd;
		    
		if( (csd = accept( sd, NULL, NULL) ) < 0 ) {
			return new Object(-1);
		}
		
		_return = new Object( csd );
	}
	else{
		hybris_syntax_error( "function 'accept' requires 1 parameters (called with %d)", data->size() );
	}
    return _return;
}

HYBRIS_BUILTIN(hrecv){
	htype_assert( data->at(0), H_OT_INT );

    Object *_return = NULL;
    if( data->size() >= 2 ){
		int sd            = data->at(0)->xint;
		Object *object   = data->at(1);
		unsigned int size, read = 0, i;
		char c;
		/* explicit size declaration */
		if( data->size() == 3 ){
			size = data->at(2)->xint;
			switch( object->xtype ){
				case H_OT_INT    : read = recv( sd, &object->xint, size, 0 ); break;
				case H_OT_FLOAT  : read = recv( sd, &object->xfloat, size, 0 ); break;
				case H_OT_CHAR   : read = recv( sd, &object->xchar, size, 0 ); break;
				case H_OT_STRING :
					for( i = 0; i < size; i++ ){
						if( recv( sd, &c, sizeof(char), 0 ) > 0 ){
							read++ ;
							object->xstring += c;
						}
						else{
							read = 0;
							break;
						}
					}
				break;
				case H_OT_ARRAY  : hybris_generic_error( "can not directly deserialize an array type" ); break;
			}
			object->xsize = size;
		}
		/* handle size by type */
		else{
			switch( object->xtype ){
				case H_OT_INT    : object->xsize = read = recv( sd, &object->xint,   sizeof(int), 0 ); break;
				case H_OT_FLOAT  : object->xsize = read = recv( sd, &object->xfloat, sizeof(double), 0 ); break;
				case H_OT_CHAR   : object->xsize = read = recv( sd, &object->xchar,  sizeof(char), 0 ); break;
				case H_OT_STRING :
					do{
						if( recv( sd, &c, sizeof(char), 0 ) > 0 ){
							if( strchr( "\n\r\x00", c ) == NULL ){
								read++;
								object->xstring += c;
							}
						}
						else{
							read = 0;
							break;
						}
					}
					while( strchr( "\n\r\x00", c ) == NULL );
					object->xsize = read;
				break;
				case H_OT_ARRAY  : hybris_generic_error( "can not directly deserialize an array type" ); break;
			}
		}
        _return = new Object( (int)read );
    }
	else{
		hybris_syntax_error( "function 'recv' requires 2 or 3 parameters (called with %d)", data->size() );
	}

    return _return;
}

HYBRIS_BUILTIN(hsend){
	htype_assert( data->at(0), H_OT_INT );

    Object *_return = NULL;
    if( data->size() >= 2 ){
		int sd            = data->at(0)->xint;
		Object *object   = data->at(1);
		unsigned int size, written = 0, i;
		char c;
		if( data->size() == 3 ){
			size = data->at(2)->xint;
			switch( object->xtype ){
				case H_OT_INT    : written = send( sd, &object->xint,   size, 0 ); break;
				case H_OT_FLOAT  : written = send( sd, &object->xfloat, size, 0 ); break;
				case H_OT_CHAR   : written = send( sd, &object->xchar,  size, 0 ); break;
				case H_OT_STRING : written = send( sd, object->xstring.c_str(), size, 0 ); break;
				case H_OT_ARRAY  : hybris_generic_error( "can not directly serialize an array type when specifying size" ); break;
			}
		}
		else{
			switch( object->xtype ){
				case H_OT_INT    : written = send( sd, &object->xint,   sizeof(int), 0 ); break;
				case H_OT_FLOAT  : written = send( sd, &object->xfloat, sizeof(double), 0 ); break;
				case H_OT_CHAR   : written = send( sd, &object->xchar,  sizeof(char), 0 ); break;
				case H_OT_STRING : written = send( sd, object->xstring.c_str(), object->xsize, 0 ); break;
				case H_OT_ARRAY  :
					for( i = 0; i < object->xsize; i++ ){
						Object *element = object->xarray[i];
						switch( element->xtype ){
							case H_OT_INT    : written += send( sd, &element->xint,   sizeof(int), 0 ); break;
							case H_OT_FLOAT  : written += send( sd, &element->xfloat, sizeof(double), 0 ); break;
							case H_OT_CHAR   : written += send( sd, &element->xchar,  sizeof(char), 0 ); break;
							case H_OT_STRING : written += send( sd, element->xstring.c_str(), element->xsize, 0 ); break;
							case H_OT_ARRAY  : hybris_generic_error( "can not directly serialize nested arrays" ); break;
						}
					}
				break;
			}
		}
        _return = new Object( (int)written );
    }
	else{
		hybris_syntax_error( "function 'send' requires 2 or 3 parameters (called with %d)", data->size() );
	}
    return _return;
}

HYBRIS_BUILTIN(hclose){
    if( data->size() ){
		close( data->at(0)->xint );
    }
    return new Object(0);
}
