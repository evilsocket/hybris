foo = 123;
name = "foo";

println( "\nname  = $name" );
println( name." = ".$name );
println( $("fo".'o') );

println( "\nCi sono ".argc." parametri sulla linea di comando ." );
println( "Il nome dello script è '".$0."' .\n" );

for( i = 0; i < argc; i++ ){
	println( "arg[".i."] = ".$i.( $i == "dio" ? " <--- PORCO !" : "" ) );
}

println();
