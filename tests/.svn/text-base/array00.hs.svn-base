a     = array( 0, 1.0 *1, array( 2, "due", 2.0 ), 3 );
elems = elements(a);

println( "L'array 'a' ha ".elems." elementi :\n" );
for( i = 0; i < elems; i++ ){
	print( "elemento ".i." : " );
	println( a[i] );	
}

println( "\na - 2° elemento : \n" );
remove( a, 1 );
elems = elements(a);
for( i = 0; i < elems; i++ ){
	print( "elemento ".i." : " );
	println( a[i] );	
}

println( "\nTest foreach : \n" );
foreach( item of a ){
	print("item = ");
	print(item);	
	println( "\t".typeof(item) );
}

println( "\nSvuoto 'a' :\n" );
for( i = 0; i < elems; i++ ){
	pop(a);
	println(a);	
}

matrice = array( array( 0, 1 ), array( 2, 3 ) );
print( "\nMATRICE : " );
println( matrice );



/*
println(a);
println( a[2] );

b = array();
println(b);

b[] = 1;
b[] = 2;
println(b);

b[] = a;
println(b);

a[1] = b;
b[] = b;
println(b);
*/
