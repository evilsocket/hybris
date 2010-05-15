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
import  std.io.file;

class Directory {
	protected root, directories, files;

	public method Directory(){
		me.root 		= "";
		me.directories = [];
		me.files       = [];
	}

	public method Directory( root ){
		me.Directory();
		me.root = root;

		items = readdir( me.root, true );
		foreach( item of items ){
			if( item["type"] == DT_DIR ){
				me.directories[] = item["name"];
			}
			else{
				me.files[] = item["name"];
			}
		}
	}

	public method __size(){
		return sizeof(me.files);
	}

	public operator [] ( index ){
		return me.files[index];
	}
}
