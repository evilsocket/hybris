start = ticks();
print( "usleep per un secondo\n" );
usleep( 1 * 1000000 );
print( "sleep per un secondo\n" );
sleep( 1 * 1000 );
println(time());
print( "time  : ".strtime()."\n" );
print( "date  : ".strdate()."\n" );
print( "\npassati ".(ticks() - start)." ms\n" );
