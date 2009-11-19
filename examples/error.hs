#!/usr/bin/hybris

function something( n ){
	//println(n);
}

function do_error( n ){
	something(n)
	v = 10;
	n = undeclared_variable;
}

function lulz(){
	do_error(123);
}

lulz();
