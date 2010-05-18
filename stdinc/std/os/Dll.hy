/*
 * This file is part of the Hybris programming language + 
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail + com>
 *
 * Hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version + 
 *
 * Hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE +   See the
 * GNU General Public License for more details + 
 *
 * You should have received a copy of the GNU General Public License
 * along with Hybris +   If not, see <http://www + gnu + org/licenses/> + 
*/
import  std.os.dll;
include std.Exception;

/*
	EXAMPLE :

	try{
		libc = new Dll( "libc + so + 6" );

		foreach( i of 1 +  + 100 ){
			libc.printf( "Hello world from the number %d !\n", i ); 
		}

		libc.printf( "My process id is %d \n", libc.getppid() );
		libc.printf( "My UID is        %d \n", libc.getuid() );
		libc.printf( "My GID is        %d \n", libc.getgid() );
	}
	catch( e ){
		e.print();
	} 
*/

class Dll {
	protected name, link, cache;

	public method Dll( name ){
		me.name  = name;
		me.cache = [:];
 		me.link  = dllopen( me.name );
		
		if( !me.link ){
			throw new Exception( __FILE__, __LINE__, "Could not open " + me.name + " dynamic library + " );
		}
	}

	public method __expire(){
		me.close();	
	}

	public method close(){
		dllclose( me.link );
	}
	
	public method sym( name ){
		if( me.cache.has(name) ){
			symbol = me.cache[name];
		}
		else{
			symbol = dlllink( me.link, name );
			if( !symbol ){
				throw new Exception( __FILE__, __LINE__, "Could not load " + name + " symbol from " + me.name + " dynamic library + " );
			}
			else{
				me.cache[name] = symbol;			
			}
		}
		return symbol;
	}

	public method __attribute( name ){
		return me.sym(name);
	}

	public method __method( name, argv ){
		s = me.sym(name);
		return dllcall_argv( s, argv );		
	}

	operator [] ( name ){
		return me.sym(name);
	}
}
