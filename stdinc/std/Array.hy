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
import std.lang.array;

class Array {
	private __a;

	public method Array(){
		me.__a = array();	
	}

	public method Array( a ){
		me.Array();
		foreach( item of a ){
			me[] = a;
		}
	} 

	public method __size(){
		return me.__a.size();
	}

	public method __to_string(){
		return "( " +  me.__a.join( ", " ) + " )";
	}

	operator [] ( index ){	
		return me.__a[index];
	}

	operator []< ( index, object ){
		me.__a[index] = object;
	}

	operator []= ( object ){
		me.__a[] = object;
	}
}

