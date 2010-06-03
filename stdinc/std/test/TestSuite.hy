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
include std.test.TestUnit;

class TestSuite {
	protected units;
	protected results;
	protected name;

	public method TestSuite( name ){
		me.name    = name;		
		me.units   = [];
		me.results = [];
	}	

	public method add( unit ){
		me.units[] = unit;
	}

	public method run(){
		foreach( unit of me.units ){
			me.results[] = unit.run();
		}
	}

	public method __to_string(){
		repr = "<" + me.name + ">\n";
			   
		foreach( result of me.results ){
			repr += result;
		}

		repr += "</" + me.name + ">";
		
		return repr;	
	}	
}
