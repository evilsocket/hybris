print( "TESTING URL EXTRACTION :\n\n" );
html  = http_get( "www.google.com", "/search?hl=en&safe=off&pwst=1&q=evilsocket&start=10&sa=N" );
links = rex_matches( "/href=\"(http\://.+?)(?:#.*?)?\"/i", html );
foreach( link of links ){
	if( rex_match( "(/search\?)|google", link ) == 0 ){
		print( "\tLINK : ".link."\n" );		
	}
}

print( "\nTESTING COOKIE EXTRACTION ON POST :\n\n" );
headers = map( "Referer"    -> "http://www.evilsocket.net",
               "User-Agent" -> "muauauau" );
               
data    = map( "username" -> "evilsocket",
               "password" -> "ti_piacerebbe",
               "Login"    -> "Login" );
               
array   = http_post( "www.evilsocket.net", "/", data, 1, headers );

cookie  = rex_matches( "/Set\-Cookie\: (.+) path\=\//i", array[0] );
cookie  = cookie[0];

println( "\t".cookie );

print( "\nTESTING HTTPS :\n\n" );
           
html = http_get( "https://gna.org", "/projects/tetrinux/" );
println(html);


