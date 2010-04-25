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
import std.io.console;

class Console  {
	
	public method print( data ){
		return print(data);
	}

	public method println( data ){
		return println(data);
	}

	public method input( var ){
		return input(var);
	}

	public method input( msg, var ){
		return input( msg, var );
	}

	operator << ( data ){
		me->print(data);
		return me;
	}

	operator >> ( var ){
		return me->input(var);
	}
}	
/*
 * Global console object.
 */
console = new Console();

