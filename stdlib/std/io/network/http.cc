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
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hhttp_get);
HYBRIS_DEFINE_FUNCTION(hhttp_post);
HYBRIS_DEFINE_FUNCTION(hhttp);

extern "C" named_function_t hybris_module_functions[] = {
	{ "http_get", hhttp_get },
	{ "http_post", hhttp_post },
	{ "http", hhttp },
	{ "", NULL }
};

#define HTTP_GET  0
#define HTTP_POST 1

extern "C" void hybris_module_init( Context * ctx ){
    HYBRIS_DEFINE_CONSTANT( ctx, "GET",  MK_TMP_INT_OBJ(HTTP_GET) );
    HYBRIS_DEFINE_CONSTANT( ctx, "POST", MK_TMP_INT_OBJ(HTTP_POST) );
}

static size_t http_append_callback( void *ptr, size_t size, size_t nmemb, void *data ){
	string *buffer = (string *)data;
	(*buffer) += (char *)ptr;
	return size * nmemb;
}

HYBRIS_DEFINE_FUNCTION(hhttp_get){
	if( HYB_ARGC() < 2 ){
		hyb_throw( H_ET_SYNTAX, "function 'http_get' requires at least 2 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );
	HYB_TYPE_ASSERT( HYB_ARGV(1), otString );
	if( HYB_ARGC() >= 3 ){
		HYB_TYPE_ASSERT( HYB_ARGV(2), otMap );
	}

	CURL  *cd;
	CURLcode res;
	struct curl_slist *headerlist = NULL;
	string hbuffer,
		   buffer,
		   host = STRING_ARGV(0),
		   page = STRING_ARGV(1),
		   url;
	unsigned int dohead = (HYB_ARGC() >= 3 ? ob_lvalue( HYB_ARGV(2) ) : 0),
				 https  = !strncmp( "https://", host.c_str(), 8 );

	curl_global_init(CURL_GLOBAL_ALL);
	cd = curl_easy_init();

	if( strncmp( "http://", host.c_str(), 7 ) != 0 && !https ){
		url = "http://"	+ host;
	}
	else{
		url = host;
	}
	if( url[ url.size() - 1 ] != '/' && page[0] != '/' ){
		url += "/" + page;
	}
	else{
		url += page;
	}

	curl_easy_setopt( cd, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( cd, CURLOPT_WRITEFUNCTION, http_append_callback );
	curl_easy_setopt( cd, CURLOPT_WRITEDATA, (void *)&buffer );

	if( dohead ){
		curl_easy_setopt( cd, CURLOPT_WRITEHEADER, &hbuffer );
	}

	if( https ){
		curl_easy_setopt( cd, CURLOPT_SSL_VERIFYPEER, 0L );
		curl_easy_setopt( cd, CURLOPT_SSL_VERIFYHOST, 0L );
	}

	if( HYB_ARGC() > 3 ){
		HYB_TYPE_ASSERT( HYB_ARGV(3), otMap );
		unsigned int i;
		string header;
		MapObject *headers = MAP_ARGV(3);

		for( i = 0; i < headers->items; ++i ){
			string name  = ob_svalue( headers->keys[i] ),
				   value = ob_svalue( headers->values[i] );
			header       = name + ": " + value;
						   headerlist = curl_slist_append( headerlist, header.c_str() );
		}
		curl_easy_setopt( cd, CURLOPT_HTTPHEADER, headerlist );
	}

	res = curl_easy_perform(cd);

	curl_easy_cleanup(cd);

	curl_global_cleanup();

	if( headerlist != NULL ){
		curl_slist_free_all(headerlist);
	}

	if( !dohead ){
		return OB_DOWNCAST( MK_STRING_OBJ(buffer.c_str()) );
	}
	else{
		VectorObject *array = MK_VECTOR_OBJ();

		ob_cl_push_reference( OB_DOWNCAST(array), OB_DOWNCAST( MK_STRING_OBJ(hbuffer.c_str() ) ) );
		ob_cl_push_reference( OB_DOWNCAST(array), OB_DOWNCAST( MK_STRING_OBJ(buffer.c_str() ) ) );

		return OB_DOWNCAST( array );
	}
}

HYBRIS_DEFINE_FUNCTION(hhttp_post){
	if( HYB_ARGC() < 3 ){
		hyb_throw( H_ET_SYNTAX, "function 'http_post' requires at least 3 parameters (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );
	HYB_TYPE_ASSERT( HYB_ARGV(1), otString );
	HYB_TYPE_ASSERT( HYB_ARGV(2), otMap );
	if( HYB_ARGC() >= 3 ){
		HYB_TYPE_ASSERT( HYB_ARGV(3), otInteger );
	}

	CURL  *cd;
	CURLcode res;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist    *headerlist = NULL;
	MapObject *post = MAP_ARGV(2);
	string hbuffer,
		   buffer,
		   host = STRING_ARGV(0),
		   page = STRING_ARGV(1),
		   url;
	unsigned int i,
				 dohead = (HYB_ARGC() >= 3 ? ob_lvalue( HYB_ARGV(3) ) : 0),
				 https  = !strncmp( "https://", host.c_str(), 8 );

	curl_global_init(CURL_GLOBAL_ALL);
	cd = curl_easy_init();

	if( strncmp( "http://", host.c_str(), 7 ) != 0 && !https ){
		url = "http://"	+ host;
	}
	else{
		url = host;
	}
	if( url[ url.size() - 1 ] != '/' && page[0] != '/' ){
		url += "/" + page;
	}
	else{
		url += page;
	}

	curl_easy_setopt( cd, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( cd, CURLOPT_WRITEFUNCTION, http_append_callback );
	curl_easy_setopt( cd, CURLOPT_WRITEDATA, (void *)&buffer );

	if( dohead ){
		curl_easy_setopt( cd, CURLOPT_WRITEHEADER, &hbuffer );
	}

	if( https ){
		curl_easy_setopt( cd, CURLOPT_SSL_VERIFYPEER, 0L );
		curl_easy_setopt( cd, CURLOPT_SSL_VERIFYHOST, 0L );
	}

	if( HYB_ARGC() >= 4 ){
		HYB_TYPE_ASSERT( HYB_ARGV(4), otMap );
		string header;
		MapObject *headers = MAP_ARGV(4);

		for( i = 0; i < headers->items; i++ ){
			string name  = ob_svalue(headers->keys[i]),
                   value = ob_svalue(headers->values[i]);
			header     = name + ": " + value;
			headerlist = curl_slist_append( headerlist, header.c_str() );
		}
		curl_easy_setopt( cd, CURLOPT_HTTPHEADER, headerlist );
	}

	for( i = 0; i < post->items; i++ ){
		string name  = ob_svalue( post->keys[i] ),
               value = ob_svalue( post->values[i] );

		curl_formadd( &formpost, &lastptr,
		 		      CURLFORM_COPYNAME, name.c_str(),
               		  CURLFORM_COPYCONTENTS, value.c_str(),
              		  CURLFORM_END );
	}
	curl_easy_setopt( cd, CURLOPT_HTTPPOST, formpost );

	res = curl_easy_perform(cd);

	curl_easy_cleanup(cd);

	curl_global_cleanup();

	if( formpost != NULL ){
		curl_formfree(formpost);
	}
	if( headerlist != NULL ){
		curl_slist_free_all(headerlist);
	}

	if( !dohead ){
		return OB_DOWNCAST( MK_STRING_OBJ(buffer.c_str()) );
	}
	else{
		VectorObject *array = MK_VECTOR_OBJ();

		ob_cl_push_reference( OB_DOWNCAST(array), OB_DOWNCAST( MK_STRING_OBJ(hbuffer.c_str() ) ) );
		ob_cl_push_reference( OB_DOWNCAST(array), OB_DOWNCAST( MK_STRING_OBJ(buffer.c_str() ) ) );

		return OB_DOWNCAST( array );
	}
}

HYBRIS_DEFINE_FUNCTION(hhttp){
    if( HYB_ARGC() < 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'http' requires at least 1 parameter (called with %d)", HYB_ARGC() );
	}
    HYB_TYPE_ASSERT( HYB_ARGV(0), otInteger );

    vmem_t hdata;
    int    method = INT_ARGV(0),
           i;

    if( method == HTTP_GET && HYB_ARGC() < 3 ){
        hyb_throw( H_ET_SYNTAX, "function 'http' requires at least 2 parameters if method=GET (called with %d)", HYB_ARGC() );
    }
    else if( method == HTTP_POST && HYB_ARGC() < 4 ){
        hyb_throw( H_ET_SYNTAX, "function 'http' requires at least 3 parameters if method=POST (called with %d)", HYB_ARGC() );
    }

    for( i = 1; i < HYB_ARGC(); ++i ){
        hdata.insert( HANONYMOUSIDENTIFIER, HYB_ARGV(i) );
    }

    if( method == HTTP_GET ){
        return hhttp_get( ctx, &hdata );
    }
    else if( method == HTTP_POST ){
        return hhttp_post( ctx, &hdata );
    }
    else{
        hyb_throw( H_ET_SYNTAX, "function 'http', unknown method" );
    }
}
