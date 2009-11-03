dll  = dllopen( "libc-2.10.1.so" );
if( dll == 0 ){
	print( "error loading library\n" );
	exit();
}

printf = dlllink( dll, "printf" );
if( printf == 0 ){
	print( "error linking symbol\n" );
	exit();
}
dllclose(dll);

for( i = 0; i < 10; i++ ){
	printf( "%d %s\n", i, "ciao" );
}

pointer = printf;

pointer( "hello da pointer\n" );

pointer( "0x%X\n", pointer );
