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
import std.lang.map;

class Map {
	private __m;

	public method Map(){
		me.__m = map();
	}

	public method __size(){
		return me.__m.size();
	}

	public method __to_string(){
		ks = array();
		vs = array();

		foreach( k -> v of me.__m ){
			ks[] = k;
			vs[] = v;
		}		
		return "{ " + ks.join( ", " ) + " } . { " + vs.join( ", " ) + " }";
	}

	public method __attribute( name ){
		if( me.__m.has(name) == false){
			me.__m[name] = null;
		}
		return me.__m[name];
	}

	public method __attribute( name, value ){
		me.__m[name] = value;
	}	

	operator [] ( key ){	
		return me.__m[key];
	}

	operator []< ( key, object ){
		me.__m[key] = object;
	}
}

