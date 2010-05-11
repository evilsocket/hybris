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
import std.io.network.socket;

class Socket {
	protected sd;

	public method Socket( domain, type ){
		me.sd = socket( domain, type );
	}
	
	public method Socket( sd ){
		me.sd = sd;
	}
	
	public method Socket(){
		me.sd = null;
	}

	private method __expire(){
		me.close();
	}

	public method bind( address, port ){
		return bind( me.sd, address, port );
	}

	public method bind( port ){
		return bind( me.sd, "0.0.0.0", port );
	}

	public method listen( backlog ){
		return listen( me.sd, backlog );
	}

	public method listen(){
		return listen( me.sd );
	}

	public method accept(){
		return new Socket( accept(me.sd) );
	}

	public method sockname( address, port ){
		return getsockname( me.sd, address, port );
	}

	public method peername( address, port ){
		return getpeername( me.sd, address, port );
	}

	public method close(){
		close( me.sd );
	}

	public method setTimeout( tm ){
		return settimeout( me.sd, tm );
	}

	public method write( data ){
		return send( me.sd, data );
	}

	public method read(){
		byte = ' ';
		if( recv( me.sd, byte ) > 0 ){
			return byte;
		}
		else{
			return -1;
		}
	}
 
	public method read( size ){
		byte   = ' ';
		buffer = "";
		for( i = 0; i < size; i++ ){
			if( recv( me.sd, byte ) > 0 ){
				buffer += byte;
			}
			else{
				return buffer;
			}
		}
		return buffer;
	}	

	public method readline(){
		byte = ' ';
		line = "";
		do{
			if( recv( me.sd, byte ) > 0 ){
				line += byte;
			}
			else{
				return -1;
			}
		}
		while( byte != '\n' && byte );
		
		return line;
	}
}


