charset = 'a' .. 'z';
numbers = 1 .. 10;

foreach( c of charset ){
	print( c." " );
}	
print("\n");

foreach( n of numbers ){
	print( n." " );
}
print("\n");

foreach( c2 of 'a' .. 'z' ){
	print( c2." " );
}
print("\n");

to = 10;

foreach( n2 of 1 .. to ){
	print( n2." " );
}
print("\n");

println( "to = ".to );
