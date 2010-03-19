pid = fork();
if( pid < 0 ){
	println( "error forking" );	
}
else if( pid == 0 ){
	println( "hello from ".getpid() );	
}
else{
	wait(0);
}	

pd = popen( "tracepath www.google.it", "r" );

buffer = ' ';
line   = "";
while( fread( pd, buffer ) ){
	if( buffer == '\n' ){ 
		matches = rex_matches( "\((.*)\)", line );
		if( elements(matches) ){
			println( "HOP : ".matches[0] );
		}
		line = "";
	}
	else{
		line = line.buffer;
	}
}

pclose(pd);
