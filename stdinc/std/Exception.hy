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

class Exception {
	protected file, line, message;
	
	public method Exception( file, line, message ){
		me->file    = file;
		me->line    = line;
		me->message = message;
	}	

	public method Exception( message ){
		me->Exception( 0, 0, message );
	}
	
	public method message(){
		return me->message;
	}

	public method __to_string(){
		if( me->file ){	
			svalue = "[".me->file.":".me->line."] ".me->message;
		}
		else{
			svalue = me->message;	
		}
		return svalue;
	}

	public method print(){
		println( me );
	}

	method __size(){
		return 1;
	}
}

