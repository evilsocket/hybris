b = binary( 10, 12, 85, 123, 'c' );
println("b è grande ".sizeof(b)." bytes." );

foreach( c of b ){
	println(c);
}

for( i = 0; i < sizeof(b); i++ ){
	println( "b[".i."] = ".toint(b[i])." (".b[i].")");
}
