/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "common.h"
#include "vmem.h"
#include "builtin.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

HYBRIS_BUILTIN(hticks){
    timeval ts;
    gettimeofday(&ts,0);
    return new Object( static_cast<int>(ts.tv_sec * 1000 + (ts.tv_usec / 1000)) );
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

	return new Object( static_cast<int>(0) );
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

	return new Object( static_cast<int>(0) );
}

HYBRIS_BUILTIN(htime){
	Object *map = new Object();
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	map->map( new Object("sec"),   new Object(static_cast<int>(ti->tm_sec)) );
	map->map( new Object("min"),   new Object(static_cast<int>(ti->tm_min)) );
	map->map( new Object("hour"),  new Object(static_cast<int>(ti->tm_hour)) );
	map->map( new Object("mday"),  new Object(static_cast<int>(ti->tm_mday)) );
	map->map( new Object("month"), new Object(static_cast<int>(ti->tm_mon + 1)) );
	map->map( new Object("year"),  new Object(static_cast<int>(ti->tm_year + 1900)) );
	map->map( new Object("wday"),  new Object(static_cast<int>(ti->tm_wday + 1)) );
	map->map( new Object("yday"),  new Object(static_cast<int>(ti->tm_yday + 1)) );

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
