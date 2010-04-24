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

import std.io.file;

class File {
	
	protected file, fileName;

	public method File ( file_name, mode){
		me->file = fopen ( file_name, mode);
	}

	public method File ( file ){
		me->file = file;
	}

	private method __expire() {
		me->close();
	}
	
	public method close (){
		fclose( file );
	}
	
	public method readLine (){
		return line = fgets( me->file );
	}

	public method getFileName (){
		return fileName;
	}

	public method getSize (){
		return fsize(fileName);
	}
	
	public method readAll (){
		text = "";
		line = "";
		while ( ( line = fgets(fp) ) != 0 ){
			text .= line;
		}
		return text;
	}

	public method read (){
		byte = ' ';
		if ( fread( me->file, byte) > 0 ) {
			return byte;
		}
		else {
			return -1;
		}
	}

	public method read ( bytes ) {
		word = "";
		byte = ' ';
		if ( fread( me->file, byte, bytes) > 0 ) {
			word .= byte;
		}
		else {
			return -1;
		}
		return word;
	}

	public method read ( seek, seekType ){
		if ( me->seek() == 0 ) {
			return -1;
		}

		return  me->read();
	}

	public method read ( bytes, seek, seekType ){
		if ( me->seek() == 0 ) {
			return -1;
		}

		return  me->read( bytes );
	}
	public method seek ( position, mode ){
		return fseek( fp, position, mode );
	}

	
}
