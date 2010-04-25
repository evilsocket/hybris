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
import std.os.process;
include std.io.File;

class Pipe extends File {
	protected pname, pargs;
	
	public method Pipe( name, args, mode ){
		me->File(0)
		me->fileName = name;
		me->pname    = name;
		me->pargs    = args;
		me->mode     = mode;
	}

	public method Pipe( name, mode ){
		me->Pipe( name, "", mode );
	}

	public method Pipe(){
		me->Pipe("","");
	}

	public method close (){
		if( me->file ){
			pclose( me->file );	
			me->file = null;		
		}
	}

	private method __expire() {
		me->close();
	}

	public method setProcessName( name ){
		me->close();
		me->pname = name;
	}

	public method setProcessArgs( args ){
		me->close();
		me->pargs = args;
	}

	public method setProcessMode( mdoe ){
		me->close();
		me->mode = mode;
	}

	public method open(){
		if( me->pname == "" || me->mode == "" ){
			return false;		
		}
		me->file = popen( me->pname." ".me->pargs, me->mode );

		return (me->file != 0);
	}
}	
