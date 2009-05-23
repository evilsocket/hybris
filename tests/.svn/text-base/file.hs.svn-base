fp = fopen( "test.dat", "w+b" );
data = array( 10.2, "ciao mamma", 12 + 3 );
fwrite( fp, data );
fclose(fp);
println(data);


fp = fopen( "test.dat", "rb" );
if( !fp ){
	print( "Impossibile aprire il file !\n" );	
}
float  = 0.0;
string = "";
int    = 0;
fread( fp, float );
fread( fp, string );
fread( fp, int );

print( "float : ".float."\nstring : ".string."\nint : ".int."\n" );

fclose(fp);

print( "test.dat is ".fsize("test.dat")." bytes long .\n" );

println( file("test.xml") );
