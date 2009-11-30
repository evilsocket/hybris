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
#ifndef _HBUILTIN_H_
#   define _HBUILTIN_H_

#include "common.h"
#include "vmem.h"
#include <vector>
#include <string>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>

using std::string;
using std::vector;

/* builtin function pointer prototype */
typedef Object * (*function_t)( vmem_t * );

/* helper macro to define a builtin function */
#define HYBRIS_BUILTIN(name) Object *name( vmem_t *data )
/* helper macro to define a builtin constant */
#define HYBRIS_CONSTANT( name, value ) { name, new Object(value) }

/* builtins definition list item structure */
typedef struct {
    string      identifier;
    function_t function;
}
builtin_t;

/* builtins' constants definition structure */
typedef struct {
    string identifier;
    Object *value;
}
builtin_constant_t;

/* module initializer function pointer prototype */
typedef void (*initializer_t)( vmem_t *vm, vcode_t *vc );

/* module structure definition */
typedef struct{
    string               name;
    initializer_t       initializer;
    vector<builtin_t *> functions;
}
module_t;

/* type.cc */
HYBRIS_BUILTIN(hisint);
HYBRIS_BUILTIN(hisfloat);
HYBRIS_BUILTIN(hischar);
HYBRIS_BUILTIN(hisstring);
HYBRIS_BUILTIN(hisarray);
HYBRIS_BUILTIN(hismap);
HYBRIS_BUILTIN(hisalias);
HYBRIS_BUILTIN(htypeof);
HYBRIS_BUILTIN(hsizeof);
HYBRIS_BUILTIN(htoint);
HYBRIS_BUILTIN(htostring);
HYBRIS_BUILTIN(htoxml);
HYBRIS_BUILTIN(hfromxml);
/* math.cc */
HYBRIS_BUILTIN(hacos);
HYBRIS_BUILTIN(hasin);
HYBRIS_BUILTIN(hatan);
HYBRIS_BUILTIN(hatan2);
HYBRIS_BUILTIN(hceil);
HYBRIS_BUILTIN(hcos);
HYBRIS_BUILTIN(hcosh);
HYBRIS_BUILTIN(hexp);
HYBRIS_BUILTIN(hfabs);
HYBRIS_BUILTIN(hfloor);
HYBRIS_BUILTIN(hfmod);
HYBRIS_BUILTIN(hlog);
HYBRIS_BUILTIN(hlog10);
HYBRIS_BUILTIN(hpow);
HYBRIS_BUILTIN(hsin);
HYBRIS_BUILTIN(hsinh);
HYBRIS_BUILTIN(hsqrt);
HYBRIS_BUILTIN(htan);
HYBRIS_BUILTIN(htanh);
/* array.cc */
HYBRIS_BUILTIN(harray);
HYBRIS_BUILTIN(helements);
HYBRIS_BUILTIN(hpop);
HYBRIS_BUILTIN(hremove);
HYBRIS_BUILTIN(hcontains);
/* map.cc */
HYBRIS_BUILTIN(hmap);
HYBRIS_BUILTIN(hmapelements);
HYBRIS_BUILTIN(hmappop);
HYBRIS_BUILTIN(hunmap);
HYBRIS_BUILTIN(hismapped);
/* string.cc */
HYBRIS_BUILTIN(hstrlen);
HYBRIS_BUILTIN(hstrfind);
HYBRIS_BUILTIN(hsubstr);
HYBRIS_BUILTIN(hstrreplace);
HYBRIS_BUILTIN(hstrsplit);
/* matrix.cc */
HYBRIS_BUILTIN(hmatrix);
HYBRIS_BUILTIN(hcolumns);
HYBRIS_BUILTIN(hrows);

#ifdef PCRE_SUPPORT
/* pcre.cc */
HYBRIS_BUILTIN(hrex_match);
HYBRIS_BUILTIN(hrex_matches);
HYBRIS_BUILTIN(hrex_replace);
#endif

Object *hrex_operator( Object *o, Object *regexp );
/* conio.cc */
HYBRIS_BUILTIN(hprint);
HYBRIS_BUILTIN(hprintln);
HYBRIS_BUILTIN(hinput);
/* process.cc */
HYBRIS_BUILTIN(hexec);
HYBRIS_BUILTIN(hfork);
HYBRIS_BUILTIN(hgetpid);
HYBRIS_BUILTIN(hwait);
HYBRIS_BUILTIN(hpopen);
HYBRIS_BUILTIN(hpclose);
HYBRIS_BUILTIN(hexit);
/* reflection.cc */
HYBRIS_BUILTIN(hvar_names);
HYBRIS_BUILTIN(hvar_values);
HYBRIS_BUILTIN(huser_functions);
HYBRIS_BUILTIN(hcore_functions);
#ifndef _LP64
HYBRIS_BUILTIN(hcall);
#endif
/* dll.cc */
#ifndef _LP64
HYBRIS_BUILTIN(hdllopen);
HYBRIS_BUILTIN(hdlllink);
HYBRIS_BUILTIN(hdllcall);
HYBRIS_BUILTIN(hdllclose);
#endif
/* time.cc */
HYBRIS_BUILTIN(hticks);
HYBRIS_BUILTIN(husleep);
HYBRIS_BUILTIN(hsleep);
HYBRIS_BUILTIN(htime);
HYBRIS_BUILTIN(hstrtime);
HYBRIS_BUILTIN(hstrdate);
/* fileio.cc */
HYBRIS_BUILTIN(hfopen);
HYBRIS_BUILTIN(hfseek);
HYBRIS_BUILTIN(hftell);
HYBRIS_BUILTIN(hfsize);
HYBRIS_BUILTIN(hfread);
HYBRIS_BUILTIN(hfgets);
HYBRIS_BUILTIN(hfwrite);
HYBRIS_BUILTIN(hfclose);
HYBRIS_BUILTIN(hfile);
HYBRIS_BUILTIN(hreaddir);
/* netio.cc */
HYBRIS_BUILTIN(hsettimeout);
HYBRIS_BUILTIN(hconnect);
HYBRIS_BUILTIN(hserver);
HYBRIS_BUILTIN(haccept);
HYBRIS_BUILTIN(hrecv);
HYBRIS_BUILTIN(hsend);
HYBRIS_BUILTIN(hclose);

#ifdef HTTP_SUPPORT
/* http.cc */
HYBRIS_BUILTIN(hhttp_get);
HYBRIS_BUILTIN(hhttp_post);
#endif

#ifdef XML_SUPPORT
/* xml.cc */
HYBRIS_BUILTIN(hxml_load);
HYBRIS_BUILTIN(hxml_parse);
#endif

/* encode.cc */
HYBRIS_BUILTIN(hurlencode);
HYBRIS_BUILTIN(hurldecode);
HYBRIS_BUILTIN(hbase64encode);
HYBRIS_BUILTIN(hbase64decode);
/* pthreads.cc */
HYBRIS_BUILTIN(hpthread_create);
HYBRIS_BUILTIN(hpthread_exit);
HYBRIS_BUILTIN(hpthread_join);

static builtin_constant_t HSTATICCONSTANTS[] = {
    /* fileio.cc::fseek */
    HYBRIS_CONSTANT( "SEEK_SET", static_cast<long>(SEEK_SET) ),
    HYBRIS_CONSTANT( "SEEK_CUR", static_cast<long>(SEEK_CUR) ),
    HYBRIS_CONSTANT( "SEEK_END", static_cast<long>(SEEK_END) ),
    /* fileio.cc::readdir */
    HYBRIS_CONSTANT( "DT_BLK",   static_cast<long>(DT_BLK)  ),
    HYBRIS_CONSTANT( "DT_CHR",   static_cast<long>(DT_CHR)  ),
    HYBRIS_CONSTANT( "DT_DIR",   static_cast<long>(DT_DIR)  ),
    HYBRIS_CONSTANT( "DT_FIFO",  static_cast<long>(DT_FIFO) ),
    HYBRIS_CONSTANT( "DT_LNK",   static_cast<long>(DT_LNK)  ),
    HYBRIS_CONSTANT( "DT_REG",   static_cast<long>(DT_REG)  ),
    HYBRIS_CONSTANT( "DT_SOCK",  static_cast<long>(DT_SOCK) )
};

#define NCONSTANTS sizeof(HSTATICCONSTANTS) / sizeof(HSTATICCONSTANTS[0])

static builtin_t HSTATICBUILTINS[] = {
	{ "isint", hisint },
	{ "isfloat", hisfloat },
	{ "ischar", hischar },
	{ "isstring", hisstring },
	{ "isarray", hisarray },
	{ "ismap", hismap },
	{ "isalias", hisalias },
	{ "typeof", htypeof },
	{ "sizeof", hsizeof },
	{ "toint", htoint },
	{ "tostring", htostring },
	{ "toxml", htoxml },
	{ "fromxml", hfromxml },
	{ "acos", hacos },
    { "asin", hasin },
    { "atan", hatan },
    { "atan2", hatan2 },
    { "ceil", hceil },
    { "cos", hcos },
    { "cosh", hcosh },
    { "exp", hexp },
    { "fabs", hfabs },
    { "floor", hfloor },
    { "fmod", hfmod },
    { "log", hlog },
    { "log10", hlog10 },
    { "pow", hpow },
    { "sin", hsin },
    { "sinh", hsinh },
    { "sqrt", hsqrt },
    { "tan", htan },
    { "tanh", htanh },
	{ "array", harray },
	{ "elements", helements },
	{ "pop", hpop },
	{ "remove", hremove },
	{ "contains", hcontains },
	{ "struct", hmap },
	{ "map", hmap },
	{ "mapelements", hmapelements },
	{ "mappop", hmappop },
	{ "unmap", hunmap },
	{ "ismapped", hismapped },
	{ "strlen", hstrlen },
	{ "strfind", hstrfind },
	{ "substr", hsubstr },
	{ "strreplace", hstrreplace },
	{ "strsplit", hstrsplit },

    { "matrix",  hmatrix },
    { "columns", hcolumns },
    { "rows",    hrows },

	#ifdef PCRE_SUPPORT
	{ "rex_match", hrex_match },
	{ "rex_matches", hrex_matches },
	{ "rex_replace", hrex_replace },
	#endif

	{ "print", hprint },
	{ "println", hprintln },
	{ "input", hinput },
	{ "exec", hexec },
	{ "fork", hfork },
	{ "getpid", hgetpid },
	{ "wait", hwait },
	{ "popen", hpopen },
	{ "pclose", hpclose },
    { "exit", hexit },
	{ "var_names", hvar_names },
	{ "var_values", hvar_values },
	{ "user_functions", huser_functions },
	{ "core_functions", hcore_functions },
	#ifndef _LP64
	{ "call", hcall },
	{ "dllopen", hdllopen },
    { "dlllink", hdlllink },
    { "dllcall", hdllcall },
    { "dllclose", hdllclose },
	#endif
	{ "ticks", hticks },
	{ "usleep", husleep },
	{ "sleep", hsleep },
	{ "time", htime },
	{ "strtime", hstrtime },
	{ "strdate", hstrdate },
	{ "fopen", hfopen },
	{ "fseek", hfseek },
	{ "ftell", hftell },
	{ "fsize", hfsize },
	{ "fread", hfread },
	{ "fgets", hfgets },
	{ "fwrite", hfwrite },
	{ "fclose", hfclose },
	{ "file", hfile },
    { "readdir", hreaddir },
	{ "settimeout", hsettimeout },
	{ "connect", hconnect },
	{ "server", hserver },
	{ "accept", haccept },
	{ "recv", hrecv },
	{ "send", hsend },
	{ "close", hclose },

	#ifdef HTTP_SUPPORT
	{ "http_get", hhttp_get },
	{ "http_post", hhttp_post },
    #endif

	#ifdef XML_SUPPORT
	{ "xml_load", hxml_load },
	{ "xml_parse", hxml_parse },
    #endif

    { "urlencode", hurlencode },
	{ "urldecode", hurldecode },
    { "base64encode", hbase64encode },
	{ "base64decode", hbase64decode },

	{ "pthread_create", hpthread_create },
    { "pthread_exit",   hpthread_exit },
    { "pthread_join",   hpthread_join }
};

#define NBUILTINS sizeof(HSTATICBUILTINS) / sizeof(HSTATICBUILTINS[0])

static vector<module_t *> HDYNAMICMODULES;

void       hmodule_load( char *module );
function_t hfunction_search( char *identifier );

#endif
