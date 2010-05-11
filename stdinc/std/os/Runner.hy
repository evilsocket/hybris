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
include std.os.Runnable;
include std.os.Thread;

function __std_os_RunnerDispatcher( mref, argv ){
	return mref.call(argv);	
}

class Runner extends Thread {
	private cref;

	method Runner( cref ){
		me.Thread( "__std_os_RunnerDispatcher" );
		if( contains( methods(cref), "run" ) == -1 ){
			throw new Exception( __FILE__, __LINE__, typeof(cref) + " does not implement Runnable interface or does not override the run method." );
		}
		me.cref = cref;
	}	

	method go( argv ){
		me.start( array( me.cref.run, argv ) );
	}

	method __to_string(){	
		return "Runner<" + typeof(me.cref) + ">";
	}
}

