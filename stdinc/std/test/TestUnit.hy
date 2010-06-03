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
include std.Exception;
include std.test.TestResult;
import  std.lang.type;
import  std.lang.reflection;

class TestUnit {
	public method run(){ 
		result = new TestResult();
		foreach( method of methods(me) ){
			if( method ~= "/^test[a-z0-9_]+$/i" ){
				try{
					call_method( me, method, [] );

					result.appendSuccess( typeof(me), method );
				}
				catch( error ){
					result.appendError( typeof(me), method, error );
				}	
			}			
		}
		return result;
	}

	public method fail( msg ){
		throw new Exception(msg);
	}
	
	public method failIf( expr, msg ){
		if( expr ){
			me.fail(msg);
		}
	}

	public method failUnless( expr, msg ){
		if( !expr ){
			me.fail(msg);
		}
	}

	public method assert( expr, msg ){
		me.failUnless( expr, msg );
	}

	public method failUnlessEqual( first, second, msg ){
		if( first != second ){
			me.fail( (msg ? msg : first + " != " + second) );
		}
	}

	public method assertEqual( first, second, msg ){
		me.failUnlessEqual( first, second, msg );
	}

	public method failIfEqual( first, second, msg ){
		if( first == second ){
			me.fail( (msg ? msg : first + " == " + second) );
		}
	}

	public method assertNotEqual( first, second, msg ){
		me.failIfEqual( first, second, msg );
	}
}
