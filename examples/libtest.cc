#include <stdio.h>

typedef struct {
	int number;
	char str[12];
	double dbl;
}
test_struct_t;

extern "C" int test( int number, char *string ){
	return printf( "number : %d\nstring : %s\n", number, string );	
}

extern "C" void print_struct( test_struct_t * ts ){
	
	printf( "print_struct( 0x%X )\n", ts );
	printf( "test_struct_t::number : %d\n"
			"test_struct_t::str    : %s\n"
			"test_struct_t::dbl    : %lf\n", ts->number, ts->str, ts->dbl );
}
