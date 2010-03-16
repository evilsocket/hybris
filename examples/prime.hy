tofind = 100;
found  = 0;
n      = 3;

while( found < tofind ){
	loop  = ceil(n / 2);
	prime = 1;
	for( div = 2; div < loop && prime; div++ ){
		if( (n % div) == 0 ){
			prime = 0;
		}
	}

	if( prime ){
		found++;
		println( n." è primo" );	
	}

	n++;
}



