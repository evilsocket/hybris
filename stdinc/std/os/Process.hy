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

class Process {
	protected pd, execName, arg;
	protected isPipe;
	
	public method Process ( execName, arg ){
		me->execName = execName;
		me->arg = arg;
		me->isPipe = -1;
	}
	public method Process ( execName ){
		me->execName = execName;
		me->arg = "";
		me->isPipe = -1;
	}
	public method Process (){
		me->execName = "";
		me->arg = "";
		me->isPipe = -1;
	}
	public method close (){
		if ( me->isPipe != -1 ){
			pclose( me->pd );
			me->isPipe = -1;
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
			return -1;
		}
		me->isPipe = -1;
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
			return -1;
		}
		me->pd = popen( execName." ".arg, mode );
		me->isPipe = 1;
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
	public method readAll (){
		if ( me->isPipe == -1) {
			return -1;
		}
		pipe = new File( me->pd );
		return pipe->readAll();
	}
}			
