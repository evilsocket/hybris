function ok( n ){
	println(n);	
}

function error(a){
	println(is_not_defined);	
}

function wrap(){
	error(2);	
}

function letssee( n, f ){
	ok(n);
	wrap();
}

letssee(123, "ciao");
