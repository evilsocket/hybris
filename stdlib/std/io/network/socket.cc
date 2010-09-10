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

typedef struct _SocketObject {
	int sd;
	int family;
	int type;
	int protocol;

	_SocketObject( int _sd, int _family, int _type, int _protocol ) :
		sd(_sd),
		family(_family),
		type(_type),
		protocol(_protocol) {

	}
}
SocketObject;

#define MK_SOCK(s,f,t,p) gc_new_handle( new SocketObject( s, f, t, p ) )

HYBRIS_DEFINE_FUNCTION(hsocket);
HYBRIS_DEFINE_FUNCTION(hbind);
HYBRIS_DEFINE_FUNCTION(hlisten);
HYBRIS_DEFINE_FUNCTION(haccept);
HYBRIS_DEFINE_FUNCTION(hgetsockname);
HYBRIS_DEFINE_FUNCTION(hgetpeername);
HYBRIS_DEFINE_FUNCTION(hsettimeout);
HYBRIS_DEFINE_FUNCTION(hconnect);
HYBRIS_DEFINE_FUNCTION(hserver);
HYBRIS_DEFINE_FUNCTION(hrecv);
HYBRIS_DEFINE_FUNCTION(hsend);
HYBRIS_DEFINE_FUNCTION(hclose);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "socket", 	 hsocket,      H_REQ_ARGC(2),   { H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "bind", 		 hbind,        H_REQ_ARGC(2,3), { H_REQ_TYPES(otHandle), H_REQ_TYPES(otString), H_REQ_TYPES(otInteger) } },
	{ "listen", 	 hlisten,      H_REQ_ARGC(1,2), { H_REQ_TYPES(otHandle), H_REQ_TYPES(otInteger) } },
	{ "accept", 	 haccept,      H_REQ_ARGC(1),   { H_REQ_TYPES(otHandle) } },
	{ "getsockname", hgetsockname, H_REQ_ARGC(3),   { H_REQ_TYPES(otHandle), H_REQ_TYPES(otReference), H_REQ_TYPES(otReference) } },
	{ "getpeername", hgetpeername, H_REQ_ARGC(3),   { H_REQ_TYPES(otHandle), H_REQ_TYPES(otReference), H_REQ_TYPES(otReference) } },
	{ "settimeout",  hsettimeout,  H_REQ_ARGC(2),   { H_REQ_TYPES(otHandle), H_REQ_TYPES(otInteger) } },
	{ "connect",     hconnect, 	   H_REQ_ARGC(2,3), { H_REQ_TYPES(otString), H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "server",      hserver,      H_REQ_ARGC(1,2), { H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "recv", 	     hrecv,        H_REQ_ARGC(2,3), { H_REQ_TYPES(otHandle), H_ANY_TYPE, H_REQ_TYPES(otInteger) } },
	{ "send", 		 hsend,        H_REQ_ARGC(2,3), { H_REQ_TYPES(otHandle), H_ANY_TYPE, H_REQ_TYPES(otInteger) } },
	{ "close", 		 hclose,       H_REQ_ARGC(1),   { H_REQ_TYPES(otHandle) } },
	{ "", NULL }
};

extern "C" void hybris_module_init( vm_t * vm ){
	/*
	 * Socket address families.
	 */
	HYBRIS_DEFINE_CONSTANT( vm, "AF_UNSPEC", gc_new_integer(AF_UNSPEC) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_LOCAL", gc_new_integer(AF_LOCAL) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_UNIX", gc_new_integer(AF_UNIX) );
#ifndef __APPLE__
	HYBRIS_DEFINE_CONSTANT( vm, "AF_FILE", gc_new_integer(AF_FILE) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_INET", gc_new_integer(AF_INET) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_AX25", gc_new_integer(AF_AX25) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_IPX", gc_new_integer(AF_IPX) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_APPLETALK", gc_new_integer(AF_APPLETALK) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_NETROM", gc_new_integer(AF_NETROM) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_BRIDGE", gc_new_integer(AF_BRIDGE) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_ATMPVC", gc_new_integer(AF_ATMPVC) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_X25", gc_new_integer(AF_X25) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_INET6", gc_new_integer(AF_INET6) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_ROSE", gc_new_integer(AF_ROSE) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_DECnet", gc_new_integer(AF_DECnet) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_NETBEUI", gc_new_integer(AF_NETBEUI) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_SECURITY", gc_new_integer(AF_SECURITY) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_KEY", gc_new_integer(AF_KEY) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_NETLINK", gc_new_integer(AF_NETLINK) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_ROUTE", gc_new_integer(AF_ROUTE) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_PACKET", gc_new_integer(AF_PACKET) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_ASH", gc_new_integer(AF_ASH) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_ECONET", gc_new_integer(AF_ECONET) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_ATMSVC", gc_new_integer(AF_ATMSVC) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_RDS", gc_new_integer(AF_RDS) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_SNA", gc_new_integer(AF_SNA) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_IRDA", gc_new_integer(AF_IRDA) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_PPPOX", gc_new_integer(AF_PPPOX) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_WANPIPE", gc_new_integer(AF_WANPIPE) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_LLC", gc_new_integer(AF_LLC) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_CAN", gc_new_integer(AF_CAN) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_TIPC", gc_new_integer(AF_TIPC) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_BLUETOOTH", gc_new_integer(AF_BLUETOOTH) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_IUCV", gc_new_integer(AF_IUCV) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_RXRPC", gc_new_integer(AF_RXRPC) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_ISDN", gc_new_integer(AF_ISDN) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_PHONET", gc_new_integer(AF_PHONET) );
	HYBRIS_DEFINE_CONSTANT( vm, "AF_IEEE802154", gc_new_integer(AF_IEEE802154) );
#endif
	HYBRIS_DEFINE_CONSTANT( vm, "AF_MAX", gc_new_integer(AF_MAX) );
	/*
	 * Socket types.
	 */
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_STREAM", gc_new_integer(SOCK_STREAM) );
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_DGRAM", gc_new_integer(SOCK_DGRAM) );
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_RAW", gc_new_integer(SOCK_RAW) );
#ifndef __APPLE__
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_RDM", gc_new_integer(SOCK_RDM) );
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_SEQPACKET", gc_new_integer(SOCK_SEQPACKET) );
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_DCCP", gc_new_integer(SOCK_DCCP) );
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_PACKET", gc_new_integer(SOCK_PACKET) );
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_CLOEXEC", gc_new_integer(SOCK_CLOEXEC) );
	HYBRIS_DEFINE_CONSTANT( vm, "SOCK_NONBLOCK", gc_new_integer(SOCK_NONBLOCK) );
#endif
}

HYBRIS_DEFINE_FUNCTION(hsocket){
	int domain,
		type;

	vm_parse_argv( "ii", &domain, &type );

	/*
	 * Protocol is choosen automatically.
	 */
	int	proto  = 0,
		sd     = socket ( domain, type, proto );

	return (sd > 0 ? ob_dcast( MK_SOCK( sd, domain, type, proto ) ) : H_DEFAULT_ERROR);
}

HYBRIS_DEFINE_FUNCTION(hbind){
	Handle *handle;
	string address;
	int    port = 0;

	vm_parse_argv( "Hsi", &handle, &address, &port );

	SocketObject *sobj = (SocketObject *)handle->value;

	int res     = -1;

	struct sockaddr_in saddr;

	if( port < 0 || port > 0xffff ){
		hyb_error( H_ET_GENERIC, "allowed port interval is 0-65535, given %d", port );
	}

	hostent * host = gethostbyname( address.c_str() );
	if( !host ){
		hyb_error( H_ET_GENERIC, "invalid address given '%s'", address.c_str() );
	}

	bzero( &saddr, sizeof(saddr) );
	saddr.sin_port   = htons(port);
	saddr.sin_family = sobj->family;
	bcopy( host->h_addr, &(saddr.sin_addr.s_addr), host->h_length );

	res = bind( sobj->sd, (struct sockaddr *)&saddr, sizeof(saddr) );

	return ob_dcast( gc_new_integer(res) );
}

HYBRIS_DEFINE_FUNCTION(hlisten){
	Handle *handle;
	int     backlog = 10;

	vm_parse_argv( "Hi", &handle, &backlog );

	int sd  = ((SocketObject *)handle->value)->sd,
		res = -1;

	res = listen( sd, backlog );

	return (Object *)gc_new_integer(res);
}

HYBRIS_DEFINE_FUNCTION(haccept){
	Handle *handle;

	vm_parse_argv( "H", &handle );

	SocketObject *sobj = (SocketObject *)handle->value;
	int			  csd  = -1;

	csd = accept( sobj->sd, NULL, NULL );

	if( csd <= 0 ){
		return (Object *)gc_new_boolean(false);
	}
	else{
		return  ob_dcast( MK_SOCK( csd, sobj->family, sobj->type, sobj->protocol ) );
	}
}

HYBRIS_DEFINE_FUNCTION(hgetsockname){
	Handle    *handle;
	Reference *raddress,
	  		  *rport;

	vm_parse_argv( "HRR", &handle, &raddress, &rport );

	SocketObject *sobj = (SocketObject *)handle->value;
	char		  ipstr[INET6_ADDRSTRLEN];
	int 		  port;
	socklen_t 	  len  = sizeof(struct sockaddr_storage);
	struct sockaddr_storage addr;

	if( getsockname( sobj->sd, (struct sockaddr*)&addr, &len ) != 0 ){
		return (Object *)gc_new_boolean(false);
	}

	/*
	 * IPv4
	 */
	if( addr.ss_family == AF_INET ){
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop( AF_INET, &s->sin_addr, ipstr, INET6_ADDRSTRLEN );
	}
	/*
	 * IPv6
	 */
	else {
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop( AF_INET6, &s->sin6_addr, ipstr, INET6_ADDRSTRLEN );
	}

	ob_assign( (Object *)raddress, (Object *)gc_new_string(ipstr) );
	ob_assign( (Object *)rport,    (Object *)gc_new_integer(port) );

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hgetpeername){
	Handle    *handle;
	Reference *raddress,
	  		  *rport;

	vm_parse_argv( "HRR", &handle, &raddress, &rport );

	SocketObject *sobj = (SocketObject *)handle->value;
	char		  ipstr[INET6_ADDRSTRLEN];
	int 		  port;
	socklen_t 	  len  = sizeof(struct sockaddr_storage);
	struct sockaddr_storage addr;

	if( getpeername( sobj->sd, (struct sockaddr*)&addr, &len ) != 0 ){
		return (Object *)gc_new_boolean(false);
	}

	/*
	 * IPv4
	 */
	if( addr.ss_family == AF_INET ){
	    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	    port = ntohs(s->sin_port);
	    inet_ntop( AF_INET, &s->sin_addr, ipstr, INET6_ADDRSTRLEN );
	}
	/*
	 * IPv6
	 */
	else {
	    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
	    port = ntohs(s->sin6_port);
	    inet_ntop( AF_INET6, &s->sin6_addr, ipstr, INET6_ADDRSTRLEN );
	}

	ob_assign( (Object *)raddress, (Object *)gc_new_string(ipstr) );
	ob_assign( (Object *)rport,    (Object *)gc_new_integer(port) );

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hsettimeout){
	Handle *handle;
	int		timeout;

	vm_parse_argv( "Hi", &handle, &timeout );

	SocketObject *sobj = (SocketObject *)handle->value;

	struct timeval tout = { 0 , timeout };

	setsockopt( sobj->sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
	setsockopt( sobj->sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hconnect){
	char *servername;
	int	  port,
		  timeout = -1;

	vm_parse_argv( "pii", &servername, &port, &timeout );

	int sd = socket( AF_INET, SOCK_STREAM, 0 );
	if( sd <= 0 ){
		return (Object *)gc_new_boolean(false);
	}
	if( timeout != -1 ){
		struct timeval tout = { 0 , timeout };

		setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
		setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
	}

	struct sockaddr_in server;
	hostent * host = gethostbyname( servername );
	if(!host){
		hyb_error( H_ET_GENERIC, "invalid address given '%s'", servername );
	}

	bzero( &server, sizeof(server) );
	server.sin_family = AF_INET;
	server.sin_port   = htons(port);
	bcopy( host->h_addr, &(server.sin_addr.s_addr), host->h_length );

	if( connect( sd, (struct sockaddr*)&server, sizeof(server) ) != 0 ){
		return (Object *)gc_new_boolean(false);
	}

    return (Object *)MK_SOCK( sd, AF_INET, SOCK_STREAM, 0 );
}

HYBRIS_DEFINE_FUNCTION(hserver){
	int port,
		timeout = -1;

	vm_parse_argv( "ii", &port, &timeout );

	int sd = socket( AF_INET, SOCK_STREAM, 0 );
	if( sd <= 0 ){
		return (Object *)gc_new_boolean(false);
	}
	if( timeout ){
		struct timeval tout = { 0 , timeout };

		setsockopt( sd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout) );
		setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout) );
	}

	struct    sockaddr_in servaddr;

	bzero( &servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	if( bind( sd, (struct sockaddr *)&servaddr, sizeof(servaddr) ) < 0 ) {
		return (Object *)gc_new_boolean(false);
	}
	if( listen( sd, 1024 ) < 0 ) {
		return (Object *)gc_new_boolean(false);
	}

    return (Object *)MK_SOCK( sd, AF_INET, SOCK_STREAM, 0 );
}

HYBRIS_DEFINE_FUNCTION(hrecv){
	Handle *handle;
	Object *object;
	size_t  size   = 0;

	vm_parse_argv( "HOi", &handle, &object, &size );

	int sd  = ((SocketObject *)handle->value)->sd;

	return ob_from_fd( object, sd, size );
}

HYBRIS_DEFINE_FUNCTION(hsend){
	Handle *handle;
	Object *object;
	size_t  size   = 0;

	vm_parse_argv( "HOi", &handle, &object, &size );

	int sd  = ((SocketObject *)handle->value)->sd;

	return ob_to_fd( object, sd, size );
}

HYBRIS_DEFINE_FUNCTION(hclose){
	Handle *handle;

	vm_parse_argv( "H", &handle );

	if( handle->value ){
		close( ((SocketObject *)handle->value)->sd );

		handle->value = NULL;
	}

    return H_DEFAULT_RETURN;
}
