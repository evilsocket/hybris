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
#include "hybris.h"
#include "parser.hpp"

string h_trace( char *function, vmem_t *stack, int identifiers ){
    string trace( function + string("(") );
    unsigned int i,
                 size( stack->size() ),
                 i_end( size - 1 );

    if( identifiers ){
        for( i = 0; i < size; ++i ){
            trace += " " + string(stack->label(i)) + "=" +  stack->at(i)->svalue() + (i < i_end ? "," : "");
        }
    }
    else{
        for( i = 0; i < size; ++i ){
            trace += " " + stack->at(i)->svalue() + (i < i_end ? "," : "");
        }
    }
    trace += " )";

    return trace;
}

void h_signal_handler( int signo ) {
    if( signo == SIGSEGV ){
        extern h_context_t HCTX;
        HCTX.args.stacktrace = 1;
        hybris_generic_error( "SIGSEGV Signal Catched" );
    }
}

void h_env_init( h_context_t *ctx, int argc, char *argv[] ){
    int i;
    char name[0xFF] = {0};
    Object *o;

    /** initialize command line arguments **/
    o = new Object( static_cast<long>(argc - 1) );
    hybris_vm_add( &ctx->vmem, (char *)"argc", o );
    delete o;
    for( i = 1; i < argc; ++i ){
        sprintf( name, "%d", i - 1 );
        o = new Object(argv[i]);
        hybris_vm_add( &ctx->vmem, name, o );
        delete o;
    }

    /** initialize builtins' constants **/
    /* misc */
    HYBRIS_DEFINE_CONSTANT( ctx, "true",  static_cast<long>(1) );
    HYBRIS_DEFINE_CONSTANT( ctx, "false", static_cast<long>(0) );
    /* fileio.cc::fseek */
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_SET", static_cast<long>(SEEK_SET) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_CUR", static_cast<long>(SEEK_CUR) );
    HYBRIS_DEFINE_CONSTANT( ctx, "SEEK_END", static_cast<long>(SEEK_END) );
    /* fileio.cc::readdir */
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_BLK",   static_cast<long>(DT_BLK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_CHR",   static_cast<long>(DT_CHR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_DIR",   static_cast<long>(DT_DIR)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_FIFO",  static_cast<long>(DT_FIFO) );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_LNK",   static_cast<long>(DT_LNK)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_REG",   static_cast<long>(DT_REG)  );
    HYBRIS_DEFINE_CONSTANT( ctx, "DT_SOCK",  static_cast<long>(DT_SOCK) );
    /* http.cc::http */
    HYBRIS_DEFINE_CONSTANT( ctx, "GET",  static_cast<long>(0) );
    HYBRIS_DEFINE_CONSTANT( ctx, "POST", static_cast<long>(1) );

    for( i = 0; i < ctx->constants.size(); ++i ){
        hybris_vm_add( &ctx->vmem, (char *)ctx->constants[i]->identifier.c_str(), ctx->constants[i]->value );
    }

    /** initialize builtin functions */
    HYBRIS_DEFINE_BUILTIN( ctx, "isint", hisint );
	HYBRIS_DEFINE_BUILTIN( ctx, "isfloat", hisfloat );
	HYBRIS_DEFINE_BUILTIN( ctx, "ischar", hischar );
	HYBRIS_DEFINE_BUILTIN( ctx, "isstring", hisstring );
	HYBRIS_DEFINE_BUILTIN( ctx, "isarray", hisarray );
	HYBRIS_DEFINE_BUILTIN( ctx, "ismap", hismap );
	HYBRIS_DEFINE_BUILTIN( ctx, "isalias", hisalias );
	HYBRIS_DEFINE_BUILTIN( ctx, "typeof", htypeof );
	HYBRIS_DEFINE_BUILTIN( ctx, "sizeof", hsizeof );
	HYBRIS_DEFINE_BUILTIN( ctx, "toint", htoint );
	HYBRIS_DEFINE_BUILTIN( ctx, "tostring", htostring );
	HYBRIS_DEFINE_BUILTIN( ctx, "toxml", htoxml );
	HYBRIS_DEFINE_BUILTIN( ctx, "fromxml", hfromxml );
	HYBRIS_DEFINE_BUILTIN( ctx, "acos", hacos );
    HYBRIS_DEFINE_BUILTIN( ctx, "asin", hasin );
    HYBRIS_DEFINE_BUILTIN( ctx, "atan", hatan );
    HYBRIS_DEFINE_BUILTIN( ctx, "atan2", hatan2 );
    HYBRIS_DEFINE_BUILTIN( ctx, "ceil", hceil );
    HYBRIS_DEFINE_BUILTIN( ctx, "cos", hcos );
    HYBRIS_DEFINE_BUILTIN( ctx, "cosh", hcosh );
    HYBRIS_DEFINE_BUILTIN( ctx, "exp", hexp );
    HYBRIS_DEFINE_BUILTIN( ctx, "fabs", hfabs );
    HYBRIS_DEFINE_BUILTIN( ctx, "floor", hfloor );
    HYBRIS_DEFINE_BUILTIN( ctx, "fmod", hfmod );
    HYBRIS_DEFINE_BUILTIN( ctx, "log", hlog );
    HYBRIS_DEFINE_BUILTIN( ctx, "log10", hlog10 );
    HYBRIS_DEFINE_BUILTIN( ctx, "pow", hpow );
    HYBRIS_DEFINE_BUILTIN( ctx, "sin", hsin );
    HYBRIS_DEFINE_BUILTIN( ctx, "sinh", hsinh );
    HYBRIS_DEFINE_BUILTIN( ctx, "sqrt", hsqrt );
    HYBRIS_DEFINE_BUILTIN( ctx, "tan", htan );
    HYBRIS_DEFINE_BUILTIN( ctx, "tanh", htanh );
	HYBRIS_DEFINE_BUILTIN( ctx, "array", harray );
	HYBRIS_DEFINE_BUILTIN( ctx, "elements", helements );
	HYBRIS_DEFINE_BUILTIN( ctx, "pop", hpop );
	HYBRIS_DEFINE_BUILTIN( ctx, "remove", hremove );
	HYBRIS_DEFINE_BUILTIN( ctx, "contains", hcontains );
	HYBRIS_DEFINE_BUILTIN( ctx, "struct", hmap );
	HYBRIS_DEFINE_BUILTIN( ctx, "map", hmap );
	HYBRIS_DEFINE_BUILTIN( ctx, "mapelements", hmapelements );
	HYBRIS_DEFINE_BUILTIN( ctx, "mappop", hmappop );
	HYBRIS_DEFINE_BUILTIN( ctx, "unmap", hunmap );
	HYBRIS_DEFINE_BUILTIN( ctx, "ismapped", hismapped );
	HYBRIS_DEFINE_BUILTIN( ctx, "strlen", hstrlen );
	HYBRIS_DEFINE_BUILTIN( ctx, "strfind", hstrfind );
	HYBRIS_DEFINE_BUILTIN( ctx, "substr", hsubstr );
	HYBRIS_DEFINE_BUILTIN( ctx, "strreplace", hstrreplace );
	HYBRIS_DEFINE_BUILTIN( ctx, "strsplit", hstrsplit );
    HYBRIS_DEFINE_BUILTIN( ctx, "matrix",  hmatrix );
    HYBRIS_DEFINE_BUILTIN( ctx, "columns", hcolumns );
    HYBRIS_DEFINE_BUILTIN( ctx, "rows",    hrows );
	#ifdef PCRE_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "rex_match", hrex_match );
	HYBRIS_DEFINE_BUILTIN( ctx, "rex_matches", hrex_matches );
	HYBRIS_DEFINE_BUILTIN( ctx, "rex_replace", hrex_replace );
	#endif
	HYBRIS_DEFINE_BUILTIN( ctx, "print", hprint );
	HYBRIS_DEFINE_BUILTIN( ctx, "println", hprintln );
	HYBRIS_DEFINE_BUILTIN( ctx, "input", hinput );
	HYBRIS_DEFINE_BUILTIN( ctx, "exec", hexec );
	HYBRIS_DEFINE_BUILTIN( ctx, "fork", hfork );
	HYBRIS_DEFINE_BUILTIN( ctx, "getpid", hgetpid );
	HYBRIS_DEFINE_BUILTIN( ctx, "wait", hwait );
	HYBRIS_DEFINE_BUILTIN( ctx, "popen", hpopen );
	HYBRIS_DEFINE_BUILTIN( ctx, "pclose", hpclose );
    HYBRIS_DEFINE_BUILTIN( ctx, "exit", hexit );
	HYBRIS_DEFINE_BUILTIN( ctx, "var_names", hvar_names );
	HYBRIS_DEFINE_BUILTIN( ctx, "var_values", hvar_values );
	HYBRIS_DEFINE_BUILTIN( ctx, "user_functions", huser_functions );
	HYBRIS_DEFINE_BUILTIN( ctx, "core_functions", hcore_functions );
	HYBRIS_DEFINE_BUILTIN( ctx, "dyn_functions",  hdyn_functions );
	#ifndef _LP64
	HYBRIS_DEFINE_BUILTIN( ctx, "call", hcall );
	HYBRIS_DEFINE_BUILTIN( ctx, "dllopen", hdllopen );
    HYBRIS_DEFINE_BUILTIN( ctx, "dlllink", hdlllink );
    HYBRIS_DEFINE_BUILTIN( ctx, "dllcall", hdllcall );
    HYBRIS_DEFINE_BUILTIN( ctx, "dllclose", hdllclose );
	#endif
	HYBRIS_DEFINE_BUILTIN( ctx, "ticks", hticks );
	HYBRIS_DEFINE_BUILTIN( ctx, "usleep", husleep );
	HYBRIS_DEFINE_BUILTIN( ctx, "sleep", hsleep );
	HYBRIS_DEFINE_BUILTIN( ctx, "time", htime );
	HYBRIS_DEFINE_BUILTIN( ctx, "strtime", hstrtime );
	HYBRIS_DEFINE_BUILTIN( ctx, "strdate", hstrdate );
	HYBRIS_DEFINE_BUILTIN( ctx, "fopen", hfopen );
	HYBRIS_DEFINE_BUILTIN( ctx, "fseek", hfseek );
	HYBRIS_DEFINE_BUILTIN( ctx, "ftell", hftell );
	HYBRIS_DEFINE_BUILTIN( ctx, "fsize", hfsize );
	HYBRIS_DEFINE_BUILTIN( ctx, "fread", hfread );
	HYBRIS_DEFINE_BUILTIN( ctx, "fgets", hfgets );
	HYBRIS_DEFINE_BUILTIN( ctx, "fwrite", hfwrite );
	HYBRIS_DEFINE_BUILTIN( ctx, "fclose", hfclose );
	HYBRIS_DEFINE_BUILTIN( ctx, "file", hfile );
    HYBRIS_DEFINE_BUILTIN( ctx, "readdir", hreaddir );
	HYBRIS_DEFINE_BUILTIN( ctx, "settimeout", hsettimeout );
	HYBRIS_DEFINE_BUILTIN( ctx, "connect", hconnect );
	HYBRIS_DEFINE_BUILTIN( ctx, "server", hserver );
	HYBRIS_DEFINE_BUILTIN( ctx, "accept", haccept );
	HYBRIS_DEFINE_BUILTIN( ctx, "recv", hrecv );
	HYBRIS_DEFINE_BUILTIN( ctx, "send", hsend );
	HYBRIS_DEFINE_BUILTIN( ctx, "close", hclose );
	#ifdef HTTP_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "http",     hhttp );
	HYBRIS_DEFINE_BUILTIN( ctx, "http_get", hhttp_get );
	HYBRIS_DEFINE_BUILTIN( ctx, "http_post", hhttp_post );
    #endif
	#ifdef XML_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "xml_load", hxml_load );
	HYBRIS_DEFINE_BUILTIN( ctx, "xml_parse", hxml_parse );
    #endif
    HYBRIS_DEFINE_BUILTIN( ctx, "urlencode", hurlencode );
	HYBRIS_DEFINE_BUILTIN( ctx, "urldecode", hurldecode );
    HYBRIS_DEFINE_BUILTIN( ctx, "base64encode", hbase64encode );
	HYBRIS_DEFINE_BUILTIN( ctx, "base64decode", hbase64decode );
    #ifdef MT_SUPPORT
	HYBRIS_DEFINE_BUILTIN( ctx, "pthread_create", hpthread_create );
    HYBRIS_DEFINE_BUILTIN( ctx, "pthread_exit",   hpthread_exit );
    HYBRIS_DEFINE_BUILTIN( ctx, "pthread_join",   hpthread_join );
    #endif

    /* save interpreter directory */
    getcwd( ctx->args.rootpath, 0xFF );

    #ifdef MT_SUPPORT
    ctx->th_mutex = PTHREAD_MUTEX_INITIALIZER;
    #endif

    /* initialize tree executor */
    ctx->executor = new Executor( ctx );

    signal( SIGSEGV, h_signal_handler );
}

void h_env_release( h_context_t *ctx, int onerror /*= 0*/ ){
    #ifdef MT_SUPPORT
    pthread_mutex_lock( &ctx->th_mutex );
        if( ctx->th_pool.size() > 0 ){
            printf( "[WARNING] Hard killing remaining running threads ... " );
            for( int pool_i = 0; pool_i < ctx->th_pool.size(); ++pool_i ){
                pthread_kill( ctx->th_pool[pool_i], SIGTERM );
            }
            ctx->th_pool.clear();
            printf( "done .\n" );
        }
    pthread_mutex_unlock( &ctx->th_mutex );
    #endif

    for( int i = 0; i < ctx->builtins.size(); ++i ){
        delete ctx->builtins.at(i);
    }

    ctx->builtins.clear();

    delete ctx->executor;

    hybris_vm_release( &ctx->vmem );
    hybris_vc_release( &ctx->vcode );
}

int h_file_exists( char *filename ){
    FILE *fp = fopen( filename, "r" );
    if( fp ){
        fclose(fp);
        return 1;
    }
    else{
        return 0;
    }
}

int h_changepath( h_context_t *ctx ){
	/* compute source path and chdir to it */
	char *ptr = strrchr( ctx->args.source, '/' );
	if( ptr != NULL ){
		unsigned int pos = ptr - ctx->args.source + 1;
		char path[0xFF]  = {0};
		strncpy( path, ctx->args.source, pos );
		return chdir(path);
	}
	return 0;
}


