#!/usr/bin/hybris

print( "\nTEST CONTROLLO EMAIL :\n\n" );
mails = array( "evilsocket@gmail.com", "lamer@foo" );
foreach( mail of mails ){
	if( mail ~= "/^[a-zA-Z0-9._-]+@[a-zA-Z0-9-]+\.[a-zA-Z.]{2,5}$/i" ){
		print( mail." è un email valida .\n" );	
	}
	else{
		print( mail." NON è un email valida .\n" );	
	}
}

print( "\nTEST ESTRAZIONE LINK (in connessione ...) :\n\n" );
html  = http_get( "www.google.it" , "/" );
links = (html ~= "/a[^>]+href=[\"']([^\"']+)[\"']/i");
print( "Trovati ".elements(links)." links :\n\n" );
foreach( link of links ){
	print( "\t".link."\n" );	
}

