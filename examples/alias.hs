function simone( a ){
	print( "simone::a = ".a."\n" );	
}

function docall( f, arg ){
	f( arg );
}

simone( 123 );

f = simone;

print( "f = ".f."\n" );

docall( f, "10" );

($"f")( "composta" );

driver = map( "open"  -> simone, 
			  "close" -> docall );
			
driver["open"]( "questa è open" );
driver["close"]( f, "doppio ptr" );

xml = toxml(driver);

println(xml);

copy = fromxml(xml);

println(copy);

copy["open"]( "questa è copy-open" );
