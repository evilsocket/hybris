/*
 * This file is part of the Hybris programming language interpreter.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * Hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hticks);
HYBRIS_DEFINE_FUNCTION(husleep);
HYBRIS_DEFINE_FUNCTION(hsleep);
HYBRIS_DEFINE_FUNCTION(htime);
HYBRIS_DEFINE_FUNCTION(hstrtime);
HYBRIS_DEFINE_FUNCTION(hstrdate);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "ticks", hticks },
	{ "usleep", husleep },
	{ "sleep", hsleep },
	{ "time", htime },
	{ "strtime", hstrtime },
	{ "strdate", hstrdate },
	{ "", NULL }
};

static Object *__dateTime_type = H_UNDEFINED;

extern "C" void hybris_module_init( vm_t * vm ){
    char *dateTime_attributes[] = { "sec", "min", "hour", "mday", "month", "year", "wday", "yday" };

    __dateTime_type = HYBRIS_DEFINE_STRUCTURE( vm, "DateTime", 8, dateTime_attributes );
}

HYBRIS_DEFINE_FUNCTION(hticks){
    timeval ts;
    gettimeofday(&ts,0);

    return ob_dcast( gc_new_integer( ts.tv_sec * 1000 + (ts.tv_usec / 1000) ) );
}

HYBRIS_DEFINE_FUNCTION(husleep){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'usleep' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otInteger, "usleep" );

	struct timespec ts;

    ts.tv_sec  = int_argv(0) / 1000000;
    ts.tv_nsec = ts.tv_sec * 1000;
    nanosleep(&ts,&ts);

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(hsleep){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'sleep' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_argv_type_assert( 0, otInteger, "sleep" );

	struct timespec ts;

    ts.tv_sec  = int_argv(0) / 1000;
    ts.tv_nsec = ts.tv_sec * 1000;
    nanosleep(&ts,&ts);

	return H_DEFAULT_RETURN;
}

HYBRIS_DEFINE_FUNCTION(htime){
	Object *dtime = ob_clone(__dateTime_type);
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	ob_set_attribute_reference( dtime, "sec",   ob_dcast( gc_new_integer(ti->tm_sec) ) );
	ob_set_attribute_reference( dtime, "min",   ob_dcast( gc_new_integer(ti->tm_min) ) );
	ob_set_attribute_reference( dtime, "hour",  ob_dcast( gc_new_integer(ti->tm_hour) ) );
	ob_set_attribute_reference( dtime, "mday",  ob_dcast( gc_new_integer(ti->tm_mday) ) );
	ob_set_attribute_reference( dtime, "month", ob_dcast( gc_new_integer(ti->tm_mon + 1) ) );
	ob_set_attribute_reference( dtime, "year",  ob_dcast( gc_new_integer(ti->tm_year + 1900) ) );
	ob_set_attribute_reference( dtime, "wday",  ob_dcast( gc_new_integer(ti->tm_wday + 1) ) );
	ob_set_attribute_reference( dtime, "yday",  ob_dcast( gc_new_integer(ti->tm_yday + 1) ) );

	return dtime;
}

HYBRIS_DEFINE_FUNCTION(hstrtime){
	char stime[0xFF] = {0};
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	sprintf( stime, "%d:%d:%d", ti->tm_hour, ti->tm_min, ti->tm_sec );

	return ob_dcast( gc_new_string(stime) );
}

HYBRIS_DEFINE_FUNCTION(hstrdate){
	char sdate[0xFF] = {0};
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	sprintf( sdate, "%d/%d/%d", ti->tm_mon + 1, ti->tm_mday, ti->tm_year + 1900 );

	return ob_dcast( gc_new_string(sdate) );
}
