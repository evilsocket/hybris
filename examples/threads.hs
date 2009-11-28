function thread_worker( number ){
	println( "Hello from a thread number ".number );
	println( "Thread ".number." finished ." );
 
	pthread_exit();
}
 
tids = array();
for( i = 0; i < 10; i++ ){
	tids[] = pthread_create( "thread_worker", i );
}

foreach( tid of tids ){
	println( "Waiting for thread ".tid." to finish ..." );
	pthread_join( tid );
}
