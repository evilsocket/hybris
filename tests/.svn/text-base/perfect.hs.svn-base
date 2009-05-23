function isdivisor( n, div ){
	return ((n % div) == 0);
}

tofind = 3;
found  = 0;
n      = 2;

input( "Quanti numeri perfetti desideri trovare ? ", tofind );

while( found < tofind ){
	sum  = 1;
	loop = (n / 2) + 1;
	for( div = 2; div < loop; div++ ){
		if( isdivisor( n, div ) ){
			sum = sum + div;	
		}
	}

	if( sum == n ){
		found++;
		println( n." è un numero perfetto (trovati ".found." numeri perfetti su ".tofind.") ");	
	}

	n++;
}


