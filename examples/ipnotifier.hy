lastip = "";

while(1){
	pd = popen( "ifconfig ppp0", "r" );

	while( (line = fgets(pd)) != 0 ){
		matches = rex_matches( "/addr:(.+)[\s]+/isU", line );
		if( matches ){
			ip = matches[0];
			if( ip != lastip ){
				print( "ip changed to ".ip."\n" );
				lastip = ip;
			}
		}
	}

	pclose(pd);
	sleep( 10 * 1000 );
}
