function thread_worker( number, string ){
	println( "Hello from a thread, number = ".number.", string = ".string );
	pthread_exit();
}

pthread_create( "thread_worker", 1012, "ciao" );

while( 1 ){ 
	sleep(1); 
}
