a = array( 0, 1, array( '2', '4', '6', '8' ), 3, "quattro" );
m = map( "zero" -> 0, "uno" -> a, "due" -> 2.0 );

println(m);

xml = toxml( m );

println(xml);

copy = fromxml( xml );

println(copy);
