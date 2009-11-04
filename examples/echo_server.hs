#!/usr/bin/hybris

port = 1234;

sd = server( port );
if( sd <= 0 ){
	println( "Error creating server socket on port ".port." ." );	
	exit();
}

while( (client = accept(sd)) > 0 && !exit ){
	if( fork() == 0 ){
		println( "New client on process : ".getpid() );	
		
		line = "";
		while( recv( client, line ) > 0 ){
			send( client, line."\n" );
		}
	}
}

close(sd);


