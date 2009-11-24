m = matrix( 3,	3, 		 // rows, columns
			2,  8,    4, // values  
			5,  6,  0.1, 
			23, 2, 1.23 );

println(m);

for( x = 0; x < columns(m); x++ ){
	for( y = 0; y < rows(m); y++ ){
		println( "m[".x."][".y."] = ".m[x][y] );
	}
}

a = m * m;

println(a);

m *= m;

println(m);
