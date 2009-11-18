#!/usr/bin/hybris


function really_bad_one( v ){
	a = v[100];
}

function screw_tha_script( v ){
	v[] = 10;
	really_bad_one(v);
}

println( "Prepare for the sigsegv !!! :D" );

too_small = array( 1, 2, 3 );

screw_tha_script( too_small );

