/*
 * This file is part of the Hybris programming language.
 *
 * Copyleft of Francesco Morucci aka merlok <merlok@ihteam.net>
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
import  std.os.process;
import  std.os.process;
include std.io.File;

class Process {
	protected pname, pargs;
	
	public method Process( name, args ){
		me.pname = name;
		me.pargs = args;
	}

	public method Process(){
		me.Pipe("","");
	}

	public method setProcessName( name ){
		me.pname = name;
	}

	public method setProcessArgs( args ){
		me.pargs = args;
	}

	public method exec(){
		if( me.pname == "" ){
			return false;		
		}

		return exec( me.pname + " " + me.pargs );
	}
}		
