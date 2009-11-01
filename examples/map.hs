m = map( "zero" -> 0, "uno" -> 2 - 1, "due" -> 2.0, "tre" -> array( 0, 1, 2, 3 ) );
print(m);

println( m["due"] );

m[144] = "stocazzo";

print(m);

unmap(m,"uno");

print(m);

m[12.4] = "dodici virgola quattro";

m["lol"] = map( "Ciao" -> "mao",
				"Bau"  -> array( m ) );

print(m);

x = toxml(m);

print(x);

print( fromxml(x) );

/*
foreach( key -> value of m ){
	if( isarray(value) || ismap(value) ){
		print( key." : " );
		print(value);
	}
	else{
		print( key." : ".value."\n" );	
	}
}
*/
