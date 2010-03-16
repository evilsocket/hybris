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
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

static size_t http_append_callback( void *ptr, size_t size, size_t nmemb, void *data ){
	string *buffer = (string *)data;
	(*buffer) += (char *)ptr;
	return size * nmemb;
}

HYBRIS_BUILTIN(hhttp_get){
	if( data->size() < 2 ){
		hybris_syntax_error( "function 'http_get' requires at least 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );
	if( data->size() >= 3 ){
		htype_assert( data->at(2), H_OT_MAP );
	}

	CURL  *cd;
	CURLcode res;
	struct curl_slist *headerlist = NULL;
	string hbuffer,
		   buffer,
		   host = data->at(0)->xstring,
		   page = data->at(1)->xstring,
		   url;
	unsigned int dohead = (data->size() >= 3 ? data->at(2)->lvalue() : 0),
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

	if( data->size() > 3 ){
		htype_assert( data->at(3), H_OT_MAP );
		unsigned int i;
		string header;
		Object *headers = data->at(3);

		for( i = 0; i < headers->xmap.size(); i++ ){
			Object *name  = headers->xmap[i]->toString(),
				   *value = headers->xarray[i]->toString();
			header     = name->xstring + ": " + value->xstring;
			headerlist = curl_slist_append( headerlist, header.c_str() );
			delete name;
			delete value;
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
		return new Object( (char *)buffer.c_str() );
	}
	else{
		Object *array = new Object();
		array->push( new Object( (char *)hbuffer.c_str() ) );
		array->push( new Object( (char *)buffer.c_str() ) );
		return array;
	}
}

HYBRIS_BUILTIN(hhttp_post){
	if( data->size() < 3 ){
		hybris_syntax_error( "function 'http_post' requires at least 3 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );
	htype_assert( data->at(2), H_OT_MAP );
	if( data->size() >= 3 ){
		htype_assert( data->at(3), H_OT_INT );
	}

	CURL  *cd;
	CURLcode res;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist    *headerlist = NULL;
	Object *post = data->at(2);
	string hbuffer,
		   buffer,
		   host = data->at(0)->xstring,
		   page = data->at(1)->xstring,
		   url;
	unsigned int i,
				 dohead = (data->size() >= 3 ? data->at(3)->lvalue() : 0),
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

	if( data->size() >= 4 ){
		htype_assert( data->at(4), H_OT_MAP );
		string header;
		Object *headers = data->at(4);

		for( i = 0; i < headers->xmap.size(); i++ ){
			Object *name  = headers->xmap[i]->toString(),
					*value = headers->xarray[i]->toString();
			header     = name->xstring + ": " + value->xstring;
			headerlist = curl_slist_append( headerlist, header.c_str() );
			delete name;
			delete value;
		}
		curl_easy_setopt( cd, CURLOPT_HTTPHEADER, headerlist );
	}

	for( i = 0; i < post->xmap.size(); i++ ){
		Object *name  = post->xmap[i]->toString(),
				*value = post->xarray[i]->toString();

		curl_formadd( &formpost, &lastptr,
		 		      CURLFORM_COPYNAME, name->xstring.c_str(),
               		  CURLFORM_COPYCONTENTS, value->xstring.c_str(),
              		  CURLFORM_END );
		delete name;
		delete value;
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
		return new Object( (char *)buffer.c_str() );
	}
	else{
		Object *array = new Object();
		array->push( new Object( (char *)hbuffer.c_str() ) );
		array->push( new Object( (char *)buffer.c_str() ) );
		return array;
	}
}

