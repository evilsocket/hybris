#!/usr/bin/hybris

println( "\nTEST CONTROLLO EMAIL :\n" );
mails = array( "evilsocket@gmail.com", "lamer@foo" );
foreach( mail of mails ){
	if( mail ~= "/^[a-zA-Z0-9._-]+@[a-zA-Z0-9-]+\.[a-zA-Z.]{2,5}$/i" ){
		println( mail." è un email valida ." );	
	}
	else{
		println( mail." NON è un email valida ." );	
	}
}

println( "\nTEST ESTRAZIONE LINK (in connessione ...) :\n" );
html  = http_get( "www.google.it" , "/" );
links = (html ~= "/a[^>]+href=[\"']([^\"']+)[\"']/i");
println( "Trovati ".elements(links)." links :\n" );
foreach( link of links ){
	println( "\t".link );	
}

