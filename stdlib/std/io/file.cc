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
	{ "fopen",   hfopen,   H_REQ_ARGC(2),   { H_REQ_TYPES(otString), H_REQ_TYPES(otString) } },
	{ "fseek",   hfseek,   H_REQ_ARGC(3),   { H_REQ_TYPES(otHandle), H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "ftell",   hftell,   H_REQ_ARGC(1),   { H_REQ_TYPES(otHandle) } },
	{ "fsize",   hfsize,   H_REQ_ARGC(1),   { H_REQ_TYPES(otHandle,otString) } },
	{ "fread",   hfread,   H_REQ_ARGC(2,3), { H_REQ_TYPES(otHandle), H_ANY_TYPE, H_REQ_TYPES(otInteger) } },
	{ "fwrite",  hfwrite,  H_REQ_ARGC(2,3), { H_REQ_TYPES(otHandle), H_ANY_TYPE, H_REQ_TYPES(otInteger) } },
	{ "fgets",   hfgets,   H_REQ_ARGC(1),   { H_REQ_TYPES(otHandle) } },
	{ "fclose",  hfclose,  H_REQ_ARGC(1),   { H_REQ_TYPES(otHandle) } },
	{ "file",    hfile,    H_REQ_ARGC(1),   { H_REQ_TYPES(otString) } },
	{ "readdir", hreaddir, H_REQ_ARGC(1,2), { H_REQ_TYPES(otString), H_REQ_TYPES(otBoolean) } },
	{ "", NULL }
};

extern "C" void hybris_module_init( vm_t * vm ){
	HYBRIS_DEFINE_CONSTANT( vm, "STDOUT_FILENO", gc_new_integer(STDOUT_FILENO) );
	HYBRIS_DEFINE_CONSTANT( vm, "STDERR_FILENO", gc_new_integer(STDERR_FILENO) );
	HYBRIS_DEFINE_CONSTANT( vm, "STDIN_FILENO",  gc_new_integer(STDIN_FILENO) );

    HYBRIS_DEFINE_CONSTANT( vm, "SEEK_SET", gc_new_integer(SEEK_SET) );
    HYBRIS_DEFINE_CONSTANT( vm, "SEEK_CUR", gc_new_integer(SEEK_CUR) );
    HYBRIS_DEFINE_CONSTANT( vm, "SEEK_END", gc_new_integer(SEEK_END) );
    HYBRIS_DEFINE_CONSTANT( vm, "DT_BLK",   gc_new_integer(DT_BLK)  );
    HYBRIS_DEFINE_CONSTANT( vm, "DT_CHR",   gc_new_integer(DT_CHR)  );
    HYBRIS_DEFINE_CONSTANT( vm, "DT_DIR",   gc_new_integer(DT_DIR)  );
    HYBRIS_DEFINE_CONSTANT( vm, "DT_FIFO",  gc_new_integer(DT_FIFO) );
    HYBRIS_DEFINE_CONSTANT( vm, "DT_LNK",   gc_new_integer(DT_LNK)  );
    HYBRIS_DEFINE_CONSTANT( vm, "DT_REG",   gc_new_integer(DT_REG)  );
    HYBRIS_DEFINE_CONSTANT( vm, "DT_SOCK",  gc_new_integer(DT_SOCK) );
}

HYBRIS_DEFINE_FUNCTION(hfopen){
    return (Object *)( gc_new_handle( fopen( string_argv(0).c_str(), string_argv(1).c_str() ) ) );;
}

HYBRIS_DEFINE_FUNCTION(hfseek){
	if( handle_argv(0) == NULL ){
		return H_DEFAULT_ERROR;
	}

	return (Object *)( gc_new_integer( fseek( (FILE *)handle_argv(0), int_argv(1), int_argv(2) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hftell){
	if( handle_argv(0) == NULL ){
		return H_DEFAULT_ERROR;
	}

    return (Object *)( gc_new_integer( ftell( (FILE *)handle_argv(0) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hfsize){
	int size = 0, pos;
	FILE *fp;

	if( ob_argv(0)->type->code == otHandle ){
		if( handle_argv(0) == NULL ){
			return H_DEFAULT_ERROR;
		}

	    fp  = (FILE *)handle_argv(0);
		pos = ftell(fp);

		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fseek( fp, pos, SEEK_SET );
	}
	else{
		fp = fopen( string_argv(0).c_str(), "r" );
		if( fp == NULL ){
			return vm_raise_exception( "'%s' no such file or directory", string_argv(0).c_str() );
		}
		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fclose(fp);
	}

	return (Object *)( gc_new_integer(size) );
}

HYBRIS_DEFINE_FUNCTION(hfread){
	if( handle_argv(0) == NULL ){
		return H_DEFAULT_ERROR;
	}

	FILE *fp = (FILE *)handle_argv(0);
	int fd = fileno(fp);
	size_t size = 0;
	Object *object = ob_argv(1);

	/* explicit size declaration */
	if( ob_argc() == 3 ){
		size = int_argv(2);
	}

	return ob_from_fd( object, fd, size );
}

HYBRIS_DEFINE_FUNCTION(hfwrite){
	if( handle_argv(0) == NULL ){
		return H_DEFAULT_ERROR;
	}

	FILE *fp = (FILE *)handle_argv(0);
	int fd = fileno(fp);
	size_t size = 0;
	Object *object = ob_argv(1);

	/* explicit size declaration */
	if( ob_argc() == 3 ){
		size = int_argv(2);
	}

	return ob_to_fd( object, fd, size );
}

HYBRIS_DEFINE_FUNCTION(hfgets){
	if( handle_argv(0) == NULL ){
		return H_DEFAULT_ERROR;
	}

	char line[0xFFFF] = {0};

	if( fgets( line, 0xFFFF, (FILE *)handle_argv(0) ) ){
		return (Object *)( gc_new_string(line) );
	}
	else{
		return H_DEFAULT_RETURN;
	}
}

HYBRIS_DEFINE_FUNCTION(hfclose){
	if( handle_argv(0) == NULL ){
		return H_DEFAULT_ERROR;
	}

	fclose( (FILE *)handle_argv(0) );
	/*
	 * Make sure the handle is set to NULL to prevent SIGSEGV
	 * when file functions try to use this file handle.
	 */
	ob_ref_ucast( ob_argv(0) )->value = NULL;

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hfile){
	FILE *fp = fopen( string_argv(0).c_str(), "rt" );
	if( !fp ){
		return vm_raise_exception( "could not open '%s' for reading", string_argv(0).c_str() );
	}

	string buffer;
	while( !feof(fp) ){
		buffer += fgetc(fp);
	}
	fclose(fp);

	buffer[ buffer.size() - 2 ] = 0x00;

    return (Object *)( gc_new_string(buffer.c_str()) );
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
    	vm_raise_exception( "could not open directory '%s' for reading", path );
    	return;
    }

    while( (ent = readdir(dirh)) != NULL ){
        Object *file = (Object *)gc_new_map();
        string name  = "";
        if( path[strlen(path) - 1] != '/' && ent->d_name[0] != '/' ){
			name = string(path) + "/" + string(ent->d_name);
		}
		else{
			name = string(path) + string(ent->d_name);
		}
        ob_cl_set_reference( file, (Object *)gc_new_string("name"), (Object *)gc_new_string(name.c_str()) );
        ob_cl_set_reference( file, (Object *)gc_new_string("type"), (Object *)gc_new_integer(ent->d_type) );

		ob_cl_push_reference( vector, file );

        if( ent->d_type == DT_DIR &&
        	strcmp( ent->d_name, ".." ) != 0 &&
        	strcmp( ent->d_name, "." ) != 0 &&
			strstr( path, "/.." ) == NULL &&
			strstr( path, "./" ) == NULL ){
            readdir_recurse( path, ent->d_name, vector );
        }
    }

    closedir(dirh);
}

HYBRIS_DEFINE_FUNCTION(hreaddir){
    DIR           *dir;
    struct dirent *ent;

    if( (dir = opendir( string_argv(0).c_str() )) == NULL ) {
    	return vm_raise_exception( "could not open directory '%s' for reading", string_argv(0).c_str() );
    }

    Object *files = (Object *)gc_new_vector();
	bool          recursive = ( ob_argc() > 1 && ob_lvalue(ob_argv(1)) );
    while( (ent = readdir(dir)) != NULL ){
    	Object *file = (Object *)gc_new_map();

    	ob_cl_set_reference( file, (Object *)gc_new_string("name"), (Object *)gc_new_string(ent->d_name) );
    	ob_cl_set_reference( file, (Object *)gc_new_string("type"), (Object *)gc_new_integer(ent->d_type) );

    	ob_cl_push_reference( files, (Object *)file );

        if( recursive ){
            if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) != 0 && strcmp( ent->d_name, "." ) != 0 ){
                readdir_recurse( (char *)string_argv(0).c_str(), ent->d_name, files );
            }
        }
    }

    closedir(dir);

    return files;
}

