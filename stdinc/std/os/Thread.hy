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
import std.os.threads;
import std.os.process;

class Thread  {
	protected function_name, id;

	public method Thread( function_name ){
		me.function_name = function_name;
		me.id 			 = 0;
	}
	
	public method start( args ){
		me.id = pthread_create_argv( me.function_name, args );
	
		return (me.id > 0);
	}

	public method exit(){
		pthread_exit();
	}

	public method terminate(){
		return pthread_kill( me.id, SIGTERM );
	}

	public method kill(){
		return pthread_kill( me.id, SIGKILL );
	}

	public method join(){
		return pthread_join( me.id );
	}
}	
