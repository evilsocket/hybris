#!/usr/bin/hybris

function client_worker( sd ){
	println( "New client thread started ." );	
	line = "";
	while( recv( sd, line ) > 0 ){
		send( sd, line."\n" );
	}
	pthread_exit();
}

port = 1234;
sd   = server( port );
if( sd <= 0 ){
	println( "Error creating server socket on port ".port." ." );	
	exit();
}

while( (client = accept(sd)) > 0 ){
	pthread_create( "client_worker", client );
}

close(sd);


