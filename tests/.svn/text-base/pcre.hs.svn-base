println( "TEST REPLACE :\n" );
string = "caro babbo sei un babbione bastardo";
replace = rex_replace( "/b+/i", string, "." );
println( "Originale : ".string );
println( "Replace   : ".replace );

println( "\nTEST CONTROLLO EMAIL :\n" );
mails = array( "evilsocket@gmail.com", "lamer@foo" );
foreach( mail of mails ){
	if( rex_match( "/^[a-zA-Z0-9._-]+@[a-zA-Z0-9-]+\.[a-zA-Z.]{2,5}$/i", mail ) ){
		println( mail." è un email valida ." );	
	}
	else{
		println( mail." NON è un email valida ." );	
	}
}

println( "\nTEST ESTRAZIONE LINK (in connessione ...) :\n" );
html  = http_get( "www.google.it" , "/" );
links = rex_matches( "href=\"(.+?)(?:#.*?)?\"", html );
println( "Trovati ".elements(links)." links :\n" );
foreach( link of links ){
	println( "\t".link );	
}

