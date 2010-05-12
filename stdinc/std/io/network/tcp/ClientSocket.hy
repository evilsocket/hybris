/*
 * This file is part of the Hybris programming language.
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
include std.io.network.tcp.Socket;

class ClientSocket extends Socket {
	protected host, port;
	
	public method ClientSocket( host, port ){
		me.Socket();		
		me.host = host;
		me.port = port;
	}

	public method ClientSocket(){
		me.ClientSocket( false, false );		
	}

	public method setHost( host ){
		me.host = host;
	}

	public method setPort( port ){
		me.port = port;
	}

	public method connect( host, port ){
		me.host = host;
		me.port = port;
		return me.connect();
	}

	public method connect(){
		me.sd = connect( me.host, me.port );
		return me.sd;
	}
}


