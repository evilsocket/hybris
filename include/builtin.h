#ifndef _HBUILTIN_H_
#   define _HBUILTIN_H_

#include "common.h"
#include "vmem.h"
#include <vector>
#include <string>
#include <dlfcn.h>

using std::string;
using std::vector;

typedef Object * (*function_t)( vmem_t * );

#define HYBRIS_BUILTIN(name) Object *name( vmem_t *data )

typedef struct {
    string      identifier;
    function_t function;
}
builtin_t;

typedef void (*initializer_t)( vmem_t *vm, vcode_t *vc );

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
/* pcre.cc */
HYBRIS_BUILTIN(hrex_match);
HYBRIS_BUILTIN(hrex_matches);
HYBRIS_BUILTIN(hrex_replace);
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
HYBRIS_BUILTIN(hcall);
/* dll.cc */
HYBRIS_BUILTIN(hdllopen);
HYBRIS_BUILTIN(hdlllink);
HYBRIS_BUILTIN(hdllcall);
HYBRIS_BUILTIN(hdllclose);
/* time.cc */
HYBRIS_BUILTIN(hticks);
HYBRIS_BUILTIN(husleep);
HYBRIS_BUILTIN(hsleep);
HYBRIS_BUILTIN(htime);
HYBRIS_BUILTIN(hstrtime);
HYBRIS_BUILTIN(hstrdate);
/* fileio.cc */
HYBRIS_BUILTIN(hfopen);
HYBRIS_BUILTIN(hfseekset);
HYBRIS_BUILTIN(hfseekcur);
HYBRIS_BUILTIN(hfseekend);
HYBRIS_BUILTIN(hftell);
HYBRIS_BUILTIN(hfsize);
HYBRIS_BUILTIN(hfread);
HYBRIS_BUILTIN(hfgets);
HYBRIS_BUILTIN(hfwrite);
HYBRIS_BUILTIN(hfclose);
HYBRIS_BUILTIN(hfile);
HYBRIS_BUILTIN(hreaddir);
/* time.cc */
HYBRIS_BUILTIN(hsettimeout);
HYBRIS_BUILTIN(hconnect);
HYBRIS_BUILTIN(hrecv);
HYBRIS_BUILTIN(hsend);
HYBRIS_BUILTIN(hclose);
/* http.cc */
HYBRIS_BUILTIN(hhttp_get);
HYBRIS_BUILTIN(hhttp_post);
/* xml.cc */
HYBRIS_BUILTIN(hxml_load);
HYBRIS_BUILTIN(hxml_parse);

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
	{ "rex_match", hrex_match },
	{ "rex_matches", hrex_matches },
	{ "rex_replace", hrex_replace },
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
	{ "call", hcall },
	{ "dllopen", hdllopen },
    { "dlllink", hdlllink },
    { "dllcall", hdllcall },
    { "dllclose", hdllclose },
	{ "ticks", hticks },
	{ "usleep", husleep },
	{ "sleep", hsleep },
	{ "time", htime },
	{ "strtime", hstrtime },
	{ "strdate", hstrdate },
	{ "fopen", hfopen },
	{ "fseekset", hfseekset },
	{ "fseekcur", hfseekcur },
	{ "fseekend", hfseekend },
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
	{ "recv", hrecv },
	{ "send", hsend },
	{ "close", hclose },
	{ "http_get", hhttp_get },
	{ "http_post", hhttp_post },
	{ "xml_load", hxml_load },
	{ "xml_parse", hxml_parse }
};

#define NBUILTINS sizeof(HSTATICBUILTINS) / sizeof(HSTATICBUILTINS[0])

static vector<module_t *> HDYNAMICMODULES;

void        hmodule_load( char *module );
function_t hfunction_search( char *identifier );

#endif
