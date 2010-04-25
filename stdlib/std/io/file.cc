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

HYBRIS_EXPORTED_FUNCTIONS() {
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

extern "C" void hybris_module_init( VM * vmachine ){
    HYBRIS_DEFINE_CONSTANT( vmachine, "SEEK_SET", gc_new_integer(SEEK_SET) );
    HYBRIS_DEFINE_CONSTANT( vmachine, "SEEK_CUR", gc_new_integer(SEEK_CUR) );
    HYBRIS_DEFINE_CONSTANT( vmachine, "SEEK_END", gc_new_integer(SEEK_END) );
    HYBRIS_DEFINE_CONSTANT( vmachine, "DT_BLK",   gc_new_integer(DT_BLK)  );
    HYBRIS_DEFINE_CONSTANT( vmachine, "DT_CHR",   gc_new_integer(DT_CHR)  );
    HYBRIS_DEFINE_CONSTANT( vmachine, "DT_DIR",   gc_new_integer(DT_DIR)  );
    HYBRIS_DEFINE_CONSTANT( vmachine, "DT_FIFO",  gc_new_integer(DT_FIFO) );
    HYBRIS_DEFINE_CONSTANT( vmachine, "DT_LNK",   gc_new_integer(DT_LNK)  );
    HYBRIS_DEFINE_CONSTANT( vmachine, "DT_REG",   gc_new_integer(DT_REG)  );
    HYBRIS_DEFINE_CONSTANT( vmachine, "DT_SOCK",  gc_new_integer(DT_SOCK) );
}

HYBRIS_DEFINE_FUNCTION(hfopen){
	ob_type_assert( ob_argv(0), otString );
	ob_type_assert( ob_argv(1), otString );

    Object *_return = NULL;
    if( ob_argc() == 2 ){
        _return = ob_dcast( PTR_TO_INT_OBJ( fopen( string_argv(0).c_str(), string_argv(1).c_str() ) ) );
    }
	else{
		hyb_error( H_ET_SYNTAX, "function 'fopen' requires 2 parameters (called with %d)", ob_argc() );
	}
    return _return;
}

HYBRIS_DEFINE_FUNCTION(hfseek){
	if( ob_argc() != 3 ){
		hyb_error( H_ET_SYNTAX, "function 'fseek' requires 3 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger );
	ob_type_assert( ob_argv(1), otInteger );
	ob_type_assert( ob_argv(2), otInteger );

	if( int_argv(0) == 0 ){
		return H_DEFAULT_ERROR;
	}

	return ob_dcast( gc_new_integer( fseek( (FILE *)int_argv(0), int_argv(1), int_argv(2) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hftell){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'ftell' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger );

	if( int_argv(0) == 0 ){
		return H_DEFAULT_ERROR;
	}

    return ob_dcast( gc_new_integer( ftell( (FILE *)int_argv(0) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hfsize){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'fsize' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otInteger, otString );
	int size = 0, pos;
	FILE *fp;

	if( ob_argv(0)->type->code == otInteger ){
		if( int_argv(0) == 0 ){
			return H_DEFAULT_ERROR;
		}

	    fp  = (FILE *)int_argv(0);
		pos = ftell(fp);

		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fseek( fp, pos, SEEK_SET );
	}
	else{
		fp = fopen( string_argv(0).c_str(), "r" );
		if( fp == NULL ){
			hyb_error( H_ET_GENERIC, "'%s' no such file or directory", string_argv(0).c_str() );
		}
		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fclose(fp);
	}

	return ob_dcast( gc_new_integer(size) );
}

HYBRIS_DEFINE_FUNCTION(hfread){
	if( ob_argc() < 2 ){
		hyb_error( H_ET_SYNTAX, "function 'fread' requires 2 or 3 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger );

	if( int_argv(0) == 0 ){
		return H_DEFAULT_ERROR;
	}

	FILE *fp = (FILE *)int_argv(0);
	int fd = fileno(fp);
	size_t size = 0;
	Object *object   = ob_argv(1);

	/* explicit size declaration */
	if( ob_argc() == 3 ){
		size = int_argv(2);
	}

	return ob_from_fd( object, fd, size );
}

HYBRIS_DEFINE_FUNCTION(hfwrite){
	if( ob_argc() < 2 ){
		hyb_error( H_ET_SYNTAX, "function 'fwrite' requires 2 or 3 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger );

	if( int_argv(0) == 0 ){
		return H_DEFAULT_ERROR;
	}

	FILE *fp = (FILE *)int_argv(0);
	int fd = fileno(fp);
	size_t size = 0;
	Object *object   = ob_argv(1);

	/* explicit size declaration */
	if( ob_argc() == 3 ){
		size = int_argv(2);
	}

	return ob_to_fd( object, fd, size );
}

HYBRIS_DEFINE_FUNCTION(hfgets){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'fgets' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger );

	if( int_argv(0) == 0 ){
		return H_DEFAULT_ERROR;
	}

	char line[0xFFFF] = {0};

	if( fgets( line, 0xFFFF, (FILE *)int_argv(0) ) ){
		return ob_dcast( gc_new_string(line) );
	}
	else{
		return H_DEFAULT_RETURN;
	}
}

HYBRIS_DEFINE_FUNCTION(hfclose){
	ob_type_assert( ob_argv(0), otInteger );
	if( int_argv(0) == 0 ){
		return H_DEFAULT_ERROR;
	}
    if( ob_argc() ){
		fclose( (FILE *)int_argv(0) );
    }
    return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hfile){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'file' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

	FILE *fp = fopen( string_argv(0).c_str(), "rt" );
	if( !fp ){
		hyb_error( H_ET_GENERIC, "could not open '%s' for reading", string_argv(0).c_str() );
	}

	string buffer;
	while( !feof(fp) ){
		buffer += fgetc(fp);
	}
	fclose(fp);

	buffer[ buffer.size() - 2 ] = 0x00;

    return ob_dcast( gc_new_string(buffer.c_str()) );
}

void readdir_recurse( char *root, char *dir, VectorObject *vector ){
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
        hyb_error( H_ET_GENERIC, "could not open directory '%s' for reading", path );
    }

    while( (ent = readdir(dirh)) != NULL ){
        MapObject *file = gc_new_map();
        string name  = "";
        if( path[strlen(path) - 1] != '/' && ent->d_name[0] != '/' ){
			name = string(path) + "/" + string(ent->d_name);
		}
		else{
			name = string(path) + string(ent->d_name);
		}
        ob_cl_set_reference( ob_dcast(file), ob_dcast( gc_new_string("name") ), ob_dcast( gc_new_string(name.c_str()) ) );
        ob_cl_set_reference( ob_dcast(file), ob_dcast( gc_new_string("type") ), ob_dcast( gc_new_integer(ent->d_type) ) );

		ob_cl_push_reference( ob_dcast(vector), ob_dcast(file) );

        if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) != 0 && strcmp( ent->d_name, "." ) != 0 ){
            readdir_recurse( path, ent->d_name, vector );
        }
    }

    closedir(dirh);
}

HYBRIS_DEFINE_FUNCTION(hreaddir){
    if( ob_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "function 'readdir' requires at least 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

    DIR           *dir;
    struct dirent *ent;

    if( (dir = opendir( string_argv(0).c_str() )) == NULL ) {
        hyb_error( H_ET_GENERIC, "could not open directory '%s' for reading", string_argv(0).c_str() );
    }

    VectorObject *files 	= gc_new_vector();
	int           recursive = ( ob_argc() > 1 && ob_lvalue(ob_argv(1)) );
    while( (ent = readdir(dir)) != NULL ){
    	MapObject *file = gc_new_map();

    	ob_cl_set_reference( ob_dcast(file), ob_dcast( gc_new_string("name") ), ob_dcast( gc_new_string(ent->d_name) ) );
    	ob_cl_set_reference( ob_dcast(file), ob_dcast( gc_new_string("type") ), ob_dcast( gc_new_integer(ent->d_type) ) );

    	ob_cl_push_reference( ob_dcast(files), ob_dcast(file) );

        if( recursive ){
            if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) != 0 && strcmp( ent->d_name, "." ) != 0 ){
                readdir_recurse( (char *)string_argv(0).c_str(), ent->d_name, files );
            }
        }
    }

    closedir(dir);

    return ob_dcast(files);
}

