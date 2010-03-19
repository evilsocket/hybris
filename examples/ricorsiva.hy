function recurse( n ){
	println( "n = ".n );
	if( n > 0 ){
		//println("if ok");
		recurse( n - 1 );	
	}	
}

for( i = 0; i <= 10; i++ ){
	//println("for ok");
	recurse(i);
	println();	
}
