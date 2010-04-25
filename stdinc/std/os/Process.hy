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
import std.os.process;
include std.io.File;

class Process extends File {
	protected execName, arg;
	protected isPipe;
	
	public method Process ( execName, arg ){
		me->File(0)
		me->fileName = execName;
		me->execName = execName;
		me->arg      = arg;
		me->isPipe   = false;
	}

	public method Process ( execName ){
		me->Process( execName, "" );
	}

	public method Process (){
		me->Process("");
	}

	public method close (){
		if ( me->isPipe ){
			pclose( me->file );
			me->isPipe = false;
		}
	}

	private method __expire() {
		me->close();
	}

	public method setexecName ( execName ){
		me->close();
		me->execName = execName;
	}

	public method setArg ( arg ){
		me->close();
		me->execName = arg;
	}

	private method _exec( execName, arg ){
		if ( execName == "" ) {
			return false;
		}
		me->isPipe = false;
		exec( execName." ".arg );
	}

	public method exec(){
		me->_exec( me->execName, me->arg );
	}

	public method exec ( execName, arg ){
		me->_exec( execName, arg );
	}

	private method _pipeOpen( execName, arg, mode ){
		if ( ( execName == "") | ( mode == "") ) {
			return false;
		}
		me->mode   = mode;
		me->file   = popen( execName." ".arg, mode );
		me->isPipe = true;
	}

	public method pipeOpen ( mode ){
		me->_pipeOpen( me->execName, me->arg, mode);
	}

	public method pipeOpen ( execName, mode ){
		me->_pipeOpen( execName, me->arg, mode );
	}

	public method pipeOpen ( execName, arg, mode ){
		me->_pipeOpen( execName, arg, mode );
	}
}			
