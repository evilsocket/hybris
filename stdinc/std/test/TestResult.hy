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

class TestResult {
	protected tests;

	public method TestResult(){
		me.reset();
	}	

	public method reset(){
		me.tests = [:];
	}	

	public method appendSuccess( testName, methodName ){
		if( me.tests.has(testName) == false ){
			me.tests[testName] = [ "success" : [], "errors" : [] ];
		}
		me.tests[testName]["success"][] = methodName;
	}

	public method appendError( testName, methodName, error ){
		if( me.tests.has(testName) == false ){
			me.tests[testName] = [ "success" : [], "errors" : [] ];
		}
		me.tests[testName]["errors"][] = [ "method" : methodName, "error" : error ];
	}

	public method __to_string(){
		repr = "";
			   
		foreach( name -> result of me.tests ){
			repr += "  <" + name + ">\n";

			foreach( error of result["errors"] ){
				repr += "    <failure method='" + error["method"].replace( "'", "\'" ) + 
							   "' error='" + error["error"].message().replace( "'", "\'" ) + "'/>\n";
			}
			foreach( success of result["success"] ){
				repr += "    <success method='" + success.replace( "'", "\'" ) + "'/>\n";
			}

			repr += "  </" + name + ">\n";
		}
		
		return repr;	
	}	
}
