function recurse( n ){
	println( "n = ".n );
	if( n > 0 ){
		recurse( n - 1 );	
	}	
}

for( i = 0; i <= 10; i++ ){
	recurse(i);
	println();	
}
