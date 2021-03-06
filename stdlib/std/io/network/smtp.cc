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

#define safe_send( sd, b ) if( sd_write(sd, b) <= 0 ){ close(sd); return H_DEFAULT_ERROR; }
#define safe_read( sd, b ) if( (buffer = sd_read(sd)) == "" ){ \
							   close(sd); return H_DEFAULT_ERROR; \
                           } \
                           else if( buffer.find("220") == string::npos && \
                                    buffer.find("235") == string::npos && \
                                    buffer.find("250") == string::npos && \
                                    buffer.find("334") == string::npos && \
                                    buffer.find("354") == string::npos ){ \
                        	   hyb_error( H_ET_WARNING, buffer.c_str() ); \
                           }

HYBRIS_DEFINE_FUNCTION(hsmtp_send);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "smtp_send", hsmtp_send, H_REQ_ARGC(2,3), { H_REQ_TYPES(otString), H_REQ_TYPES(otMap), H_REQ_TYPES(otMap) } },
	{ "", NULL }
};

int sd_create( char *host, int port ){
    int sd = socket( AF_INET, SOCK_STREAM, 0 );

	if( sd <= 0 ){
        return -1;
    }

    struct sockaddr_in server;
    hostent * resol = gethostbyname( host );
    if( resol ){
        bzero( &server, sizeof(server) );
        server.sin_family      = AF_INET;
        server.sin_port        = htons(port);
        bcopy( resol->h_addr, &(server.sin_addr.s_addr), resol->h_length );
    }

    if( connect( sd, (struct sockaddr*)&server, sizeof(server) ) != 0 ){
        return -1;
    }

    return sd;
}

string sd_read( int sd ){
    string buffer("");
    unsigned char c;
    bool isEOL(false), isEOF(false);

    while( !isEOL && !isEOF ){
        if( recv( sd, &c, sizeof(unsigned char), 0 ) < 1 ){
            isEOF = true;
        }
        else if( c == '\n' ){
            isEOL = true;
        }
        else if( c != '\r' ){
            buffer += c;
        }
    }

    return buffer;
}

int sd_write( int sd, string buffer ){
    return write( sd, buffer.c_str(), buffer.size() );
}

string base64( unsigned char *data, unsigned int size ){
    static const char b64_charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    string ret;
    unsigned char block_3[3];
    unsigned char block_4[4];
    unsigned char * str = data;
    int i = 0,
        j = 0;

    while( size-- ){
        block_3[i++] = *(str++);
        if( i == 3 ){
            block_4[0] = (block_3[0] & 0xfc) >> 2;
            block_4[1] = ((block_3[0] & 0x03) << 4) + ((block_3[1] & 0xf0) >> 4);
            block_4[2] = ((block_3[1] & 0x0f) << 2) + ((block_3[2] & 0xc0) >> 6);
            block_4[3] = block_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++){
                ret += b64_charset[block_4[i]];
            }
            i = 0;
        }
    }

    if(i){
        for(j = i; j < 3; j++){
            block_3[j] = '\0';
        }
        block_4[0] = (block_3[0] & 0xfc) >> 2;
        block_4[1] = ((block_3[0] & 0x03) << 4) + ((block_3[1] & 0xf0) >> 4);
        block_4[2] = ((block_3[1] & 0x0f) << 2) + ((block_3[2] & 0xc0) >> 6);
        block_4[3] = block_3[2] & 0x3f;

        for(j = 0; (j < i + 1); j++){
            ret += b64_charset[block_4[j]];
        }
        while((i++ < 3)){
            ret += '=';
        }
    }

    return ret;
}

HYBRIS_DEFINE_FUNCTION(hsmtp_send){
	string server_name;
	Map   *headers,
		  *login_map = NULL;

	vm_parse_argv( "sMM", &server_name, &headers, &login_map );

	int i, j, sd, port = 25;
	string from(""),
           subject(""),
           message(""),
           login_name(""),
           login_pass(""),
           buffer("");
    vector<string> receivers;


	for( i = 0; i < headers->items ; ++i ){
		string  name  = ob_string_val(headers->keys[i]);
		Object *value = headers->values[i];

		if( name == "to" ){
			ob_types_assert( value, otString, otVector, "smtp_send" );
			if( value->type->code == otString ){
				receivers.push_back( ob_string_val(value) );
			}
			else if( value->type->code == otVector ){
				for( j = 0; j < ob_vector_ucast(value)->items; ++j ){
				    ob_type_assert(  ob_vector_ucast(value)->value[j], otString, "smtp_send" );
					receivers.push_back( ob_string_val( ob_vector_ucast(value)->value[j] ) );
				}
			}
		}
		else if ( name == "from" ){
			from = ob_string_val(value);
		}
		else if ( name == "subject" ){
			subject = ob_string_val(value) ;
		}
		else if ( name == "message" ){
			message = ob_string_val(value);
		}
	}

	if ( login_map ){
		for ( i = 0; i < ob_map_ucast(login_map)->items; ++i ){
			string  name  = ob_string_val( login_map->keys[i] ),
					value = ob_string_val( login_map->values[i] );

			if ( name == "username" ){
				login_name = value;
			}
			else if ( name == "password" ){
				login_pass = value;
			}
		}
	}

	sd = sd_create( (char *)server_name.c_str(), port );
	if ( sd == -1 ){
		return H_DEFAULT_ERROR;
	}

	safe_send( sd, "EHLO hybris_mailer\r\n" );

    // read until last '250 .*' reached
	while( (buffer = sd_read(sd)) != "" ){
        if( buffer.find("220") == string::npos &&
            buffer.find("235") == string::npos &&
            buffer.find("250") == string::npos &&
            buffer.find("334") == string::npos &&
            buffer.find("354") == string::npos ){

            hyb_error( H_ET_WARNING, buffer.c_str() );
        }
        else if( buffer.find( "250 " ) != string::npos ){
            break;
        }
    }

	if( login_map ){
	    int auth_len = login_name.size() + login_pass.size() + 2;
	    unsigned char * b64auth = new unsigned char[auth_len];
	    memset( b64auth, 0x00, auth_len );

	    memcpy( &b64auth[1], login_name.c_str(), login_name.size() );
	    memcpy( &b64auth[2 + login_name.size()], login_pass.c_str(), login_pass.size() );

	    string hash = base64( b64auth, auth_len );

	    delete [] b64auth;

	    safe_send( sd, "AUTH PLAIN " + hash + "\r\n" );
	    safe_read( sd, buffer );
	}
	safe_send( sd, "MAIL FROM: <" + from + ">\r\n" );
    safe_read( sd, buffer );
	for( i = 0; i < receivers.size(); ++i ){
	    safe_send( sd, "RCPT TO: <" + receivers[i] + ">\r\n" );
		safe_read( sd, buffer );
	}
	safe_send( sd, "DATA\r\n" );
	safe_read( sd, buffer );

	safe_send( sd, "Subject: " + subject + "\r\n" );
	safe_send( sd, "From: <" + from + ">\r\n" );
    safe_send( sd, "To: <" + receivers[0] + ">\r\n" );
    safe_send( sd, "Content-Type: text/plain\r\n" );
    safe_send( sd, "Mime-Version: 1.0\r\n" );
    safe_send( sd, "X-Mailer: Hybris smtp_send\r\n" );
    safe_send( sd, "Content-Transfer-Encoding: 7bit\r\n\r\n" );

	safe_send( sd, message );

	safe_send( sd, "\r\n.\r\n" );
	safe_read( sd, buffer );
	safe_send( sd, "RSET\r\n" );
	safe_read( sd, buffer );
	safe_send( sd, "QUIT\r\n" );

	close(sd);

	return H_DEFAULT_RETURN;
}
