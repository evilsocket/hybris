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
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_SET", MK_TMP_INT_OBJ(SEEK_SET) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_CUR", MK_TMP_INT_OBJ(SEEK_CUR) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_END", MK_TMP_INT_OBJ(SEEK_END) );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_BLK",   MK_TMP_INT_OBJ(DT_BLK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_CHR",   MK_TMP_INT_OBJ(DT_CHR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_DIR",   MK_TMP_INT_OBJ(DT_DIR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_FIFO",  MK_TMP_INT_OBJ(DT_FIFO) );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_LNK",   MK_TMP_INT_OBJ(DT_LNK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_REG",   MK_TMP_INT_OBJ(DT_REG)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_SOCK",  MK_TMP_INT_OBJ(DT_SOCK) );
}

HYBRIS_DEFINE_FUNCTION(hfopen){
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );
	HYB_TYPE_ASSERT( HYB_ARGV(1), otString );

    Object *_return = NULL;
    if( HYB_ARGC() == 2 ){
        _return = OB_DOWNCAST( PTR_TO_INT_OBJ( fopen( STRING_ARGV(0).c_str(), STRING_ARGV(1).c_str() ) ) );
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
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );
	HYB_TYPE_ASSERT( HYB_ARGV(1), otInteger );
	HYB_TYPE_ASSERT( HYB_ARGV(3), otInteger );

	return OB_DOWNCAST( MK_INT_OBJ( fseek( (FILE *)INT_ARGV(0), INT_ARGV(1), INT_ARGV(2) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hftell){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'ftell' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

    return OB_DOWNCAST( MK_INT_OBJ( ftell( (FILE *)INT_ARGV(0) ) ) );
}

HYBRIS_DEFINE_FUNCTION(hfsize){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fsize' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPES_ASSERT( HYB_ARGV(0), otInteger, otString );
	int size = 0, pos;
	FILE *fp;

	if( HYB_ARGV(0)->type->code == otInteger ){
	    fp  = (FILE *)INT_ARGV(0);
		pos = ftell(fp);

		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fseek( fp, pos, SEEK_SET );
	}
	else{
		fp = fopen( STRING_ARGV(0).c_str(), "r" );
		if( fp == NULL ){
			hyb_throw( H_ET_GENERIC, "'%s' no such file or directory", STRING_ARGV(0).c_str() );
		}
		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fclose(fp);
	}

	return OB_DOWNCAST( MK_INT_OBJ(size) );
}

HYBRIS_DEFINE_FUNCTION(hfread){
	if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'fread' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	FILE *fp = (FILE *)INT_ARGV(0);
	int fd = fileno(fp);
	size_t size = 0;
	Object *object   = HYB_ARGV(1);

	/* explicit size declaration */
	if( HYB_ARGC() == 3 ){
		size = INT_ARGV(2);
	}

	return ob_from_fd( object, fd, size );
}

HYBRIS_DEFINE_FUNCTION(hfwrite){
	if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'fwrite' requires 2 or 3 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	FILE *fp = (FILE *)INT_ARGV(0);
	int fd = fileno(fp);
	size_t size = 0;
	Object *object   = HYB_ARGV(1);

	/* explicit size declaration */
	if( HYB_ARGC() == 3 ){
		size = INT_ARGV(2);
	}

	return ob_to_fd( object, fd, size );
}

HYBRIS_DEFINE_FUNCTION(hfgets){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fgets' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	char line[0xFFFF] = {0};

	if( fgets( line, 0xFFFF, (FILE *)INT_ARGV(0) ) ){
		return OB_DOWNCAST( MK_STRING_OBJ(line) );
	}
	else{
		return OB_DOWNCAST( MK_INT_OBJ(0) );
	}
}

HYBRIS_DEFINE_FUNCTION(hfclose){
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );
    if( HYB_ARGC() ){
		fclose( (FILE *)INT_ARGV(0) );
    }
    return OB_DOWNCAST( MK_INT_OBJ(0) );
}

HYBRIS_DEFINE_FUNCTION(hfile){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'file' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

	FILE *fp = fopen( STRING_ARGV(0).c_str(), "rt" );
	if( !fp ){
		hyb_throw( H_ET_GENERIC, "could not open '%s' for reading", STRING_ARGV(0).c_str() );
	}

	string buffer;
	while( !feof(fp) ){
		buffer += fgetc(fp);
	}
	fclose(fp);

	buffer[ buffer.size() - 2 ] = 0x00;

    return OB_DOWNCAST( MK_STRING_OBJ(buffer.c_str()) );
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
        hyb_throw( H_ET_GENERIC, "could not open directory '%s' for reading", path );
    }

    while( (ent = readdir(dirh)) != NULL ){
        MapObject *file = MK_MAP_OBJ();
        string name  = "";
        if( path[strlen(path) - 1] != '/' && ent->d_name[0] != '/' ){
			name = string(path) + "/" + string(ent->d_name);
		}
		else{
			name = string(path) + string(ent->d_name);
		}
        ob_cl_set_reference( OB_DOWNCAST(file), OB_DOWNCAST( MK_STRING_OBJ("name") ), OB_DOWNCAST( MK_STRING_OBJ(name.c_str()) ) );
        ob_cl_set_reference( OB_DOWNCAST(file), OB_DOWNCAST( MK_STRING_OBJ("type") ), OB_DOWNCAST( MK_INT_OBJ(ent->d_type) ) );

		ob_cl_push_reference( OB_DOWNCAST(vector), OB_DOWNCAST(file) );

        if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) != 0 && strcmp( ent->d_name, "." ) != 0 ){
            readdir_recurse( path, ent->d_name, vector );
        }
    }

    closedir(dirh);
}

HYBRIS_DEFINE_FUNCTION(hreaddir){
    if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'readdir' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

    DIR           *dir;
    struct dirent *ent;

    if( (dir = opendir( STRING_ARGV(0).c_str() )) == NULL ) {
        hyb_throw( H_ET_GENERIC, "could not open directory '%s' for reading", STRING_ARGV(0).c_str() );
    }

    VectorObject *files 	= MK_VECTOR_OBJ();
	int           recursive = ( HYB_ARGC() > 1 && ob_lvalue(HYB_ARGV(1)) );
    while( (ent = readdir(dir)) != NULL ){
    	MapObject *file = MK_MAP_OBJ();

    	ob_cl_set_reference( OB_DOWNCAST(file), OB_DOWNCAST( MK_STRING_OBJ("name") ), OB_DOWNCAST( MK_STRING_OBJ(ent->d_name) ) );
    	ob_cl_set_reference( OB_DOWNCAST(file), OB_DOWNCAST( MK_STRING_OBJ("type") ), OB_DOWNCAST( MK_INT_OBJ(ent->d_type) ) );

    	ob_cl_push_reference( OB_DOWNCAST(files), OB_DOWNCAST(file) );

        if( recursive ){
            if( ent->d_type == DT_DIR && strcmp( ent->d_name, ".." ) != 0 && strcmp( ent->d_name, "." ) != 0 ){
                readdir_recurse( (char *)STRING_ARGV(0).c_str(), ent->d_name, files );
            }
        }
    }

    closedir(dir);

    return OB_DOWNCAST(files);
}

