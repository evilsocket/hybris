#include "../common.h"
#include "../vmem.h"
#include "../builtin.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

HYBRIS_BUILTIN(hticks){
    timeval ts;
    gettimeofday(&ts,0);
    return new Object( (int)(ts.tv_sec * 1000 + (ts.tv_usec / 1000)) );
}

HYBRIS_BUILTIN(husleep){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'usleep' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );

	struct timespec ts;

    ts.tv_sec  = (long int)data->at(0)->xint / 1000000;
    ts.tv_nsec = ts.tv_sec * 1000;
    nanosleep(&ts,&ts);

	return new Object( (int)0 );
}

HYBRIS_BUILTIN(hsleep){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'sleep' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );

	struct timespec ts;

    ts.tv_sec  = (long int)data->at(0)->xint / 1000;
    ts.tv_nsec = ts.tv_sec * 1000;
    nanosleep(&ts,&ts);

	return new Object( (int)0 );
}

HYBRIS_BUILTIN(htime){
	Object *map = new Object();
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	map->map( new Object("sec"),   new Object((int)ti->tm_sec) );
	map->map( new Object("min"),   new Object((int)ti->tm_min) );
	map->map( new Object("hour"),  new Object((int)ti->tm_hour) );
	map->map( new Object("mday"),  new Object((int)ti->tm_mday) );
	map->map( new Object("month"), new Object((int)ti->tm_mon + 1) );
	map->map( new Object("year"),  new Object((int)ti->tm_year + 1900) );
	map->map( new Object("wday"),  new Object((int)ti->tm_wday + 1) );
	map->map( new Object("yday"),  new Object((int)ti->tm_yday + 1) );

	return map;
}

HYBRIS_BUILTIN(hstrtime){
	char stime[0xFF] = {0};
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	sprintf( stime, "%d:%d:%d", ti->tm_hour, ti->tm_min, ti->tm_sec );
	return new Object(stime);
}

HYBRIS_BUILTIN(hstrdate){
	char sdate[0xFF] = {0};
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	sprintf( sdate, "%d/%d/%d", ti->tm_mon + 1, ti->tm_mday, ti->tm_year + 1900 );
	return new Object(sdate);
}
