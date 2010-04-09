/*
 * This file is part of the Hybris programming language interpreter.
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
#include <sys/types.h>
#include <dirent.h>
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hfopen);
HYBRIS_DEFINE_FUNCTION(hfseek);
HYBRIS_DEFINE_FUNCTION(hftell);
HYBRIS_DEFINE_FUNCTION(hfsize);
HYBRIS_DEFINE_FUNCTION(hfread);
HYBRIS_DEFINE_FUNCTION(hfwrite);
HYBRIS_DEFINE_FUNCTION(hfgets);
HYBRIS_DEFINE_FUNCTION(hfclose);
HYBRIS_DEFINE_FUNCTION(hfile);
HYBRIS_DEFINE_FUNCTION(hreaddir);

extern "C" named_function_t hybris_module_functions[] = {
	{ "fopen", hfopen },
	{ "fseek", hfseek },
	{ "ftell", hftell },
	{ "fsize", hfsize },
	{ "fread", hfread },
	{ "fwrite", hfwrite },
	{ "fgets", hfgets },
	{ "fclose", hfclose },
	{ "file", hfile },
	{ "readdir", hreaddir },
	{ "", NULL }
};

extern "C" void hybris_module_init( Context * ctx ){
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_SET", static_cast<long>(SEEK_SET) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_CUR", static_cast<long>(SEEK_CUR) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_END", static_cast<long>(SEEK_END) );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_BLK",   static_cast<long>(DT_BLK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_CHR",   static_cast<long>(DT_CHR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_DIR",   static_cast<long>(DT_DIR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_FIFO",  static_cast<long>(DT_FIFO) );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_LNK",   static_cast<long>(DT_LNK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_REG",   static_cast<long>(DT_REG)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_SOCK",  static_cast<long>(DT_SOCK) );
}

HYBRIS_DEFINE_FUNCTION(hfopen){
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );
	HYB_TYPE_ASSERT( HYB_ARGV(1), H_OT_STRING );

    Object *_return = NULL;
    if( HYB_ARGC() == 2 ){
        _return = new Object( reinterpret_cast<long>( fopen( HYB_ARGV(0)->value.m_string.c_str(), HYB_ARGV(1)->value.m_string.c_str() ) ) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'fopen' requires 2 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hfseek){
	if( HYB_ARGC() != 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'fseek' requires 3 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );
	HYB_TYPE_ASSERT( HYB_ARGV(1), H_OT_INT );
	HYB_TYPE_ASSERT( HYB_ARGV(3), H_OT_INT );

	return new Object( static_cast<long>( fseek( (FILE *)HYB_ARGV(0)->value.m_integer, HYB_ARGV(1)->value.m_integer, HYB_ARGV(2)->value.m_integer ) ) );
}

HYBRIS_DEFINE_FUNCTION(hftell){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'ftell' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );

	return new Object( static_cast<long>( ftell( (FILE *)HYB_ARGV(0)->value.m_integer ) ) );
}

HYBRIS_DEFINE_FUNCTION(hfsize){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fsize' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), H_OT_INT, H_OT_STRING );
	int size = 0;
	if( HYB_ARGV(0)->type == H_OT_INT ){
		int pos = ftell((FILE *)HYB_ARGV(0)->value.m_integer);
		fseek( (FILE *)HYB_ARGV(0)->value.m_integer, 0, SEEK_END );
		size = ftell( (FILE *)HYB_ARGV(0)->value.m_integer );
		fseek( (FILE *)HYB_ARGV(0)->value.m_integer, pos, SEEK_SET );
	}
	else{
		FILE *fp = fopen( HYB_ARGV(0)->value.m_string.c_str(), "r" );
		if( fp == NULL ){
			hyb_throw( H_ET_GENERIC, "'%s' no such file or directory", HYB_ARGV(0)->value.m_string.c_str() );
		}
		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fclose(fp);
	}

	return new Object( static_cast<long>(size) );
}

HYBRIS_DEFINE_FUNCTION(hfread){
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );

    Object *_return = NULL;
    if( HYB_ARGC() >= 2 ){
		FILE *fp          = (FILE *)HYB_ARGV(0)->value.m_integer;
		if( !fp ){
			hyb_throw( H_ET_GENERIC, "invalid file descriptor" );
		}
		Object *object   = HYB_ARGV(1);
		unsigned int size, read = 0, i;
		char c;
		/* explicit size declaration */
		if( HYB_ARGC() == 3 ){
			size = HYB_ARGV(2)->value.m_integer;
			switch( object->type ){
				case H_OT_INT    : read = fread( &object->value.m_integer,   1, size, fp ); break;
				case H_OT_FLOAT  : read = fread( &object->value.m_double, 1, size, fp ); break;
				case H_OT_CHAR   : read = fread( &object->value.m_char,  1, size, fp ); break;
				case H_OT_STRING :
					for( i = 0; i < size; i++ ){
						read += fread( &c, 1, sizeof(char), fp );
						object->value.m_string += c;
					}
				break;
				case H_OT_ARRAY  : hyb_throw( H_ET_GENERIC, "can not directly deserialize an array type" ); break;
			}
			object->size = size;
		}
		/* handle size by type */
		else{
			switch( object->type ){
				case H_OT_INT    : object->size = read = fread( &object->value.m_integer,   1, sizeof(long), fp ); break;
				case H_OT_FLOAT  : object->size = read = fread( &object->value.m_double, 1, sizeof(double), fp ); break;
				case H_OT_CHAR   : object->size = read = fread( &object->value.m_char,  1, sizeof(char), fp ); break;
				case H_OT_STRING :
					while( (c = fgetc(fp)) != '\n' && c != '\r' && c != 0x00 ){
						object->value.m_string += c;
						read++;
					}
					object->size = read;
				break;
				case H_OT_ARRAY  : hyb_throw( H_ET_GENERIC, "can not directly deserialize an array type" ); break;
			}
		}
        _return = new Object( static_cast<long>(read) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'fread' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hfwrite){
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );

    Object *_return = NULL;
    if( HYB_ARGC() >= 2 ){
		FILE *fp          = (FILE *)HYB_ARGV(0)->value.m_integer;
		if( !fp ){
			hyb_throw( H_ET_GENERIC, "invalid file descriptor" );
		}
		Object *object   = HYB_ARGV(1);
		unsigned int size, written = 0, i;
		char c;
		if( HYB_ARGC() == 3 ){
			size = HYB_ARGV(2)->value.m_integer;
			switch( object->type ){
				case H_OT_INT    : written = fwrite( &object->value.m_integer,   1, size, fp ); break;
				case H_OT_FLOAT  : written = fwrite( &object->value.m_double, 1, size, fp ); break;
				case H_OT_CHAR   : written = fwrite( &object->value.m_char,  1, size, fp ); break;
				case H_OT_STRING : written = fwrite( object->value.m_string.c_str(), 1, size, fp ); break;
				case H_OT_ARRAY  : hyb_throw( H_ET_GENERIC, "can not directly serialize an array type when specifying size" ); break;
			}
		}
		else{
			switch( object->type ){
				case H_OT_INT    : written = fwrite( &object->value.m_integer,   1, sizeof(long), fp ); break;
				case H_OT_FLOAT  : written = fwrite( &object->value.m_double, 1, sizeof(double), fp ); break;
				case H_OT_CHAR   : written = fwrite( &object->value.m_char,  1, sizeof(char), fp ); break;
				case H_OT_STRING : written = fwrite( object->value.m_string.c_str(), 1, object->size, fp ); break;
				case H_OT_ARRAY  :
					for( i = 0; i < object->size; i++ ){
						Object *element = object->value.m_array[i];
						switch( element->type ){
							case H_OT_INT    : written += fwrite( &element->value.m_integer,   1, sizeof(long), fp ); break;
							case H_OT_FLOAT  : written += fwrite( &element->value.m_double, 1, sizeof(double), fp ); break;
							case H_OT_CHAR   : written += fwrite( &element->value.m_char,  1, sizeof(char), fp ); break;
							case H_OT_STRING : written += fwrite( element->value.m_string.c_str(), 1, element->size, fp ); break;
							case H_OT_ARRAY  : hyb_throw( H_ET_GENERIC, "can not directly serialize nested arrays" ); break;
						}
					}
				break;
			}
		}
        _return = new Object( static_cast<long>(written) );
    }
	else{
		hyb_throw( H_ET_SYNTAX, "function 'fwrite' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hfgets){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fgets' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );

	char line[0xFFFF] = {0};

	if( fgets( line, 0xFFFF, (FILE *)HYB_ARGV(0)->value.m_integer ) ){
		return new Object(line);
	}
	else{
		return new Object(static_cast<long>(0));
	}
}

HYBRIS_DEFINE_FUNCTION(hfclose){
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_INT );
    if( HYB_ARGC() ){
		fclose( (FILE *)HYB_ARGV(0)->value.m_integer );
    }
    return new Object(static_cast<long>(0));
}

HYBRIS_DEFINE_FUNCTION(hfile){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'file' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

	FILE *fp = fopen( HYB_ARGV(0)->value.m_string.c_str(), "rt" );
	if( !fp ){
		hyb_throw( H_ET_GENERIC, "could not open '%s' for reading", HYB_ARGV(0)->value.m_string.c_str() );
	}

	string buffer;
	while( !feof(fp) ){
		buffer += fgetc(fp);
	}
	fclose(fp);

	buffer[ buffer.size() - 2 ] = 0x00;

    return new Object((char *)buffer.c_str());
}

void readdir_recurse( char *root, char *dir, Object *vector ){
    char 		   path[0xFF] = {0};
    DIR           *dirh;
    struct dirent *ent;

    if( root[strlen(root) - 1] != '/' && dir[0] != '/' ){
		sprintf( path, "%s/%s", root, dir );
    }
    else{
		sprintf( path, "%s%s", root, dir );
	}

    if( (dirh = opendir(path)) == NULL ) {
        hyb_throw( H_ET_GENERIC, "could not open directory '%s' for reading", path );
    }

    while( (ent = readdir(dirh)) != NULL ){
        Object *file = new Object();
        string name  = "";
        if( path[strlen(path) - 1] != '/' && ent->d_name[0] != '/' ){
			name = string(path) + "/" + string(ent->d_name);
		}
		else{
			name = string(path) + string(ent->d_name);
		}
        file->map( new Object((char *)"name"), new Object((char *)name.c_str()) );
        file->map( new Object((char *)"type"), new Object(static_cast<long>(ent->d_type)) );
        vector->push(file);
        if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) != 0 && strcmp( ent->d_name, "." ) != 0 ){
            readdir_recurse( path, ent->d_name, vector );
        }
    }

    closedir(dirh);
}

HYBRIS_DEFINE_FUNCTION(hreaddir){
    if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'file' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

    DIR           *dir;
    struct dirent *ent;

    if( (dir = opendir( HYB_ARGV(0)->value.m_string.c_str() )) == NULL ) {
        hyb_throw( H_ET_GENERIC, "could not open directory '%s' for reading", HYB_ARGV(0)->value.m_string.c_str() );
    }

    Object *files 	  = new Object();
	int     recursive = ( HYB_ARGC() > 1 && HYB_ARGV(1)->lvalue() );
    while( (ent = readdir(dir)) != NULL ){
        Object *file = new Object();
        file->map( new Object((char *)"name"), new Object((char *)ent->d_name) );
        file->map( new Object((char *)"type"), new Object(static_cast<long>(ent->d_type)) );
        files->push(file);
        if( recursive ){
            if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) != 0 && strcmp( ent->d_name, "." ) != 0 ){
                readdir_recurse( (char *)HYB_ARGV(0)->value.m_string.c_str(), ent->d_name, files );
            }
        }
    }

    closedir(dir);
    return files;
}

