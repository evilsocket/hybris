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
#include <hybris.h>
#include <sys/stat.h>
#include <sys/mount.h>

HYBRIS_DEFINE_FUNCTION(hmknod);
HYBRIS_DEFINE_FUNCTION(hmkfifo);
HYBRIS_DEFINE_FUNCTION(hmount);
HYBRIS_DEFINE_FUNCTION(humount2);
HYBRIS_DEFINE_FUNCTION(humount);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "mknod",   hmknod,   H_REQ_ARGC(3), { H_REQ_TYPES(otString), H_REQ_TYPES(otInteger), H_REQ_TYPES(otInteger) } },
	{ "mkfifo",  hmkfifo,  H_REQ_ARGC(2), { H_REQ_TYPES(otString), H_REQ_TYPES(otInteger) } },
	{ "mount",   hmount,   H_REQ_ARGC(4), { H_REQ_TYPES(otString), H_REQ_TYPES(otString), H_REQ_TYPES(otString), H_REQ_TYPES(otInteger) } },
	{ "umount2", humount2, H_REQ_ARGC(2), { H_REQ_TYPES(otString), H_REQ_TYPES(otInteger) } },
	{ "umount",  humount,  H_REQ_ARGC(1), { H_REQ_TYPES(otString)} },
	{ "", NULL }
};

extern "C" void hybris_module_init( vm_t * vm ){
	HYBRIS_DEFINE_CONSTANT( vm, "S_IFIFO", gc_new_integer(S_IFIFO) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IFCHR", gc_new_integer(S_IFCHR) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IFDIR", gc_new_integer(S_IFDIR) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IFBLK", gc_new_integer(S_IFBLK) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IFREG", gc_new_integer(S_IFREG) );

	HYBRIS_DEFINE_CONSTANT( vm, "S_ISUID", gc_new_integer(S_ISUID) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_ISGID", gc_new_integer(S_ISGID) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IRWXU", gc_new_integer(S_IRWXU) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IRUSR", gc_new_integer(S_IRUSR) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IWUSR", gc_new_integer(S_IWUSR) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IXUSR", gc_new_integer(S_IXUSR) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IRWXG", gc_new_integer(S_IRWXG) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IRGRP", gc_new_integer(S_IRGRP) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IWGRP", gc_new_integer(S_IWGRP) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IXGRP", gc_new_integer(S_IXGRP) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IRWXO", gc_new_integer(S_IRWXO) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IROTH", gc_new_integer(S_IROTH) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IWOTH", gc_new_integer(S_IWOTH) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IXOTH", gc_new_integer(S_IXOTH) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_ISVTX", gc_new_integer(S_ISVTX) );

	HYBRIS_DEFINE_CONSTANT( vm, "MS_RDONLY", gc_new_integer(MS_RDONLY) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_NOSUID", gc_new_integer(MS_NOSUID) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_NODEV", gc_new_integer(MS_NODEV) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_NOEXEC", gc_new_integer(MS_NOEXEC) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_SYNCHRONOUS", gc_new_integer(MS_SYNCHRONOUS) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_REMOUNT", gc_new_integer(MS_REMOUNT) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_MANDLOCK", gc_new_integer(MS_MANDLOCK) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_WRITE", gc_new_integer(S_WRITE) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_APPEND", gc_new_integer(S_APPEND) );
	HYBRIS_DEFINE_CONSTANT( vm, "S_IMMUTABLE", gc_new_integer(S_IMMUTABLE) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_NOATIME", gc_new_integer(MS_NOATIME) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_NODIRATIME", gc_new_integer(MS_NODIRATIME) );
	HYBRIS_DEFINE_CONSTANT( vm, "MS_BIND", gc_new_integer(MS_BIND) );

	HYBRIS_DEFINE_CONSTANT( vm, "MNT_FORCE",  gc_new_integer(MNT_FORCE) );
	HYBRIS_DEFINE_CONSTANT( vm, "MNT_DETACH", gc_new_integer(MNT_DETACH) );
	HYBRIS_DEFINE_CONSTANT( vm, "MNT_EXPIRE", gc_new_integer(MNT_EXPIRE) );
}

HYBRIS_DEFINE_FUNCTION(hmknod){
    char  *path;
    mode_t mode;
    dev_t  dev;

    vm_parse_argv( "pii", &path, &mode, &dev );

    return (Object *)gc_new_integer( mknod( path, mode, dev ) );
}

HYBRIS_DEFINE_FUNCTION(hmkfifo){
	char  *path;
	mode_t mode;

	vm_parse_argv( "pi", &path, &mode );

	return (Object *)gc_new_integer( mkfifo( path, mode ) );
}

HYBRIS_DEFINE_FUNCTION(hmount){
	char 		 *special_file,
				 *dir,
				 *fstype;
	unsigned long options;

	vm_parse_argv( "pppl", &special_file, &dir, &fstype, &options );

	return (Object *)gc_new_integer( mount( special_file, dir, fstype, options, NULL ) );
}

HYBRIS_DEFINE_FUNCTION(humount2){
	char *file;
	int	  flags;

	vm_parse_argv( "pi", &file, &flags );

	return (Object *)gc_new_integer( umount2( file, flags ) );
}

HYBRIS_DEFINE_FUNCTION(humount){
	char *file;

	vm_parse_argv( "p", &file );

	return (Object *)gc_new_integer( umount( file ) );
}
