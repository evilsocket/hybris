function foo( a, b ){
	print( "\tfoo::a = ".a."\n\tfoo::b = ".b."\n" );	
}
function bar( n ){
	print( "\tbar::n = ".n."\n");	
}

a = 1;
b = "test";
names  = var_names();
values = var_values();
print( "VARIABLES : (".elements(names).")\n" );
for( i = 0; i < elements(names); i++ ){
	print( "\t".names[i]." => ".values[i]."\n" );	
}

cores = core_functions();
print( "\nCORE FUNCTIONS (".elements(cores)."): \n" );
foreach( core of cores ){
	print( "\t".core."\n" );	
}

users = user_functions();
print( "\nUSER FUNCTIONS (".elements(users).") : \n" );
foreach( user of users ){
	print( "\t".user."\n" );	
}

call( "println"  );
call( "print",   "CALLs : \n" );
call( "foo",     "sono"." A", 'B' );
call( "print",   "\t---------------\n" );
call( "bar",     123 );
call( "println" );
