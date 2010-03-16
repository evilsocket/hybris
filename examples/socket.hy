site = "www.evilsocket.net";
port = 80;
page = "/";

sd = connect( site, port, 1000000 );
if( sd <= 0 ){
	println( "Error connecting to ".site." on port ".port." ." );	
	return -1;
}

send( sd, "GET ".page." HTTP/1.1\r\n".
          "Host: ".site."\r\n".
          "User-Agent: x-client\r\n\r\n" );

c = ' ';
while( recv( sd, c ) > 0 ){
	print( c );	
}
println();

close(sd);

