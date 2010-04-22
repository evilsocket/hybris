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
import std.os.threads;

class ServerSocket extends Socket {
	protected port, acceptor;
	
	public method ServerSocket( port, acceptor_thread_name ){
		me->Socket(0);
		me->port = port;
		me->acceptor = acceptor_thread_name;
	}

	public method start(){
		me->sd = server( me->port );
		if( me->sd <= 0 ){
			return false;
		}

		while( (csd = accept(me->sd)) > 0 ){
			pthread_create( me->acceptor, new Socket(csd) );
		}

		return true;
	}
}


