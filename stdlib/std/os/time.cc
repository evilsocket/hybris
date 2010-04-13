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

extern "C" named_function_t hybris_module_functions[] = {
	{ "ticks", hticks },
	{ "usleep", husleep },
	{ "sleep", hsleep },
	{ "time", htime },
	{ "strtime", hstrtime },
	{ "strdate", hstrdate },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hticks){
    timeval ts;
    gettimeofday(&ts,0);

    return MK_INT_OBJ( ts.tv_sec * 1000 + (ts.tv_usec / 1000) );
}

HYBRIS_DEFINE_FUNCTION(husleep){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'usleep' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	struct timespec ts;

    ts.tv_sec  = (long)(*HYB_ARGV(0)) / 1000000;
    ts.tv_nsec = ts.tv_sec * 1000;
    nanosleep(&ts,&ts);

	return MK_INT_OBJ(0);
}

HYBRIS_DEFINE_FUNCTION(hsleep){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sleep' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

	struct timespec ts;

    ts.tv_sec  = (long)(*HYB_ARGV(0)) / 1000;
    ts.tv_nsec = ts.tv_sec * 1000;
    nanosleep(&ts,&ts);

	return MK_INT_OBJ(0);
}

HYBRIS_DEFINE_FUNCTION(htime){
	Object *map = MK_COLLECTION_OBJ();
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	map->map( MK_TMP_STRING_OBJ("sec"),   MK_TMP_INT_OBJ(ti->tm_sec) );
	map->map( MK_TMP_STRING_OBJ("min"),   MK_TMP_INT_OBJ(ti->tm_min) );
	map->map( MK_TMP_STRING_OBJ("hour"),  MK_TMP_INT_OBJ(ti->tm_hour) );
	map->map( MK_TMP_STRING_OBJ("mday"),  MK_TMP_INT_OBJ(ti->tm_mday) );
	map->map( MK_TMP_STRING_OBJ("month"), MK_TMP_INT_OBJ(ti->tm_mon + 1) );
	map->map( MK_TMP_STRING_OBJ("year"),  MK_TMP_INT_OBJ(ti->tm_year + 1900) );
	map->map( MK_TMP_STRING_OBJ("wday"),  MK_TMP_INT_OBJ(ti->tm_wday + 1) );
	map->map( MK_TMP_STRING_OBJ("yday"),  MK_TMP_INT_OBJ(ti->tm_yday + 1) );

	return map;
}

HYBRIS_DEFINE_FUNCTION(hstrtime){
	char stime[0xFF] = {0};
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	sprintf( stime, "%d:%d:%d", ti->tm_hour, ti->tm_min, ti->tm_sec );

	return MK_STRING_OBJ(stime);
}

HYBRIS_DEFINE_FUNCTION(hstrdate){
	char sdate[0xFF] = {0};
	time_t raw;
	struct tm * ti;

	time(&raw);
	ti = localtime(&raw);

	sprintf( sdate, "%d/%d/%d", ti->tm_mon + 1, ti->tm_mday, ti->tm_year + 1900 );

	return MK_STRING_OBJ(sdate);
}
