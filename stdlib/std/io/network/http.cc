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
HYBRIS_DEFINE_FUNCTION(hhttp_download);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "http_get",      hhttp_get,      H_REQ_ARGC(2,3,4), { H_REQ_TYPES(otString), H_REQ_TYPES(otString), H_REQ_TYPES(otBoolean), H_REQ_TYPES(otMap) } },
	{ "http_post",     hhttp_post,     H_REQ_ARGC(3,4,5), { H_REQ_TYPES(otString), H_REQ_TYPES(otString), H_REQ_TYPES(otMap), H_REQ_TYPES(otBoolean), H_REQ_TYPES(otMap) } },
	{ "http_download", hhttp_download, H_REQ_ARGC(2,3),   { H_REQ_TYPES(otString), H_REQ_TYPES(otHandle), H_REQ_TYPES(otAlias) } },
	{ "", NULL }
};

typedef struct {
    Node   *handler;
    vmem_t *data;
}
http_progress_callback_data_t;

static vm_t *__vm;

extern "C" void hybris_module_init( vm_t * vm ){
	__vm = vm;

    curl_global_init(CURL_GLOBAL_ALL);
}

static size_t http_append_callback( void *ptr, size_t size, size_t nmemb, void *data ){
	string *buffer = (string *)data;
	(*buffer) += (char *)ptr;
	return size * nmemb;
}

static size_t http_download_callback( void *ptr, size_t size, size_t nmemb, FILE *stream){
	return fwrite( ptr, size, nmemb, stream );
}

static size_t http_progress_callback( http_progress_callback_data_t *http_data, double dltotal, double dlnow, double ultotal, double ulnow ){
	vmem_t  stack;
	Object *h_dltotal,
		   *h_dlnow;

	h_dltotal = (Object *)gc_new_float(dltotal);
	h_dlnow   = (Object *)gc_new_float(dlnow);

	stack.push( h_dltotal );
	stack.push( h_dlnow );

	vm_exec_threaded_call( __vm, http_data->handler, http_data->data, &stack );

	return 0;
}

HYBRIS_DEFINE_FUNCTION(hhttp_get){
	CURL  *cd;
	CURLcode res;
	struct curl_slist *headerlist = NULL;
	string hbuffer,
		   buffer,
		   host,
		   page,
		   url;
	bool   dohead = false,
		   https  = !strncmp( "https://", host.c_str(), 8 );
	Map   *headers = NULL;

	vm_parse_argv( "ssbM", &host, &page, &dohead, &headers );

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

	if( headers ){
		unsigned int i;
		string header;

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

	if( headerlist != NULL ){
		curl_slist_free_all(headerlist);
	}

	if( !dohead ){
		return (Object *)gc_new_string(buffer.c_str());
	}
	else{
		Vector *array = gc_new_vector();

		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(hbuffer.c_str() ) ) );
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(buffer.c_str() ) ) );

		return ob_dcast( array );
	}
}

HYBRIS_DEFINE_FUNCTION(hhttp_post){
	CURL  *cd;
	CURLcode res;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist    *headerlist = NULL;
	string hbuffer,
		   buffer,
		   host,
		   page,
		   url;
	Map   *post,
		  *headers = NULL;
	size_t i;
	bool   dohead = false,
		   https  = !strncmp( "https://", host.c_str(), 8 );

	vm_parse_argv( "ssMbM", &host, &page, &post, &dohead, &headers );

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

	if( headers ){
		string header;
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

	if( formpost != NULL ){
		curl_formfree(formpost);
	}
	if( headerlist != NULL ){
		curl_slist_free_all(headerlist);
	}

	if( !dohead ){
		return ob_dcast( gc_new_string(buffer.c_str()) );
	}
	else{
		Vector *array = gc_new_vector();

		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(hbuffer.c_str() ) ) );
		ob_cl_push_reference( ob_dcast(array), ob_dcast( gc_new_string(buffer.c_str() ) ) );

		return ob_dcast( array );
	}
}

HYBRIS_DEFINE_FUNCTION(hhttp_download){
	string  file;
	Handle *handle;
	Alias  *alias = NULL;

	vm_parse_argv( "sHA", &file, &handle, &alias );

	http_progress_callback_data_t *http_data = NULL;
	FILE    *fp  = (FILE *)handle->value;
	CURL    *curl;
	CURLcode res;

	if(!fp){
		hyb_error( H_ET_SYNTAX, "invalid file handle given" );
	}

	curl = curl_easy_init();

	curl_easy_setopt( curl, CURLOPT_URL, file.c_str() );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, fp );
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, http_download_callback );

	if( alias ){
		http_data = new http_progress_callback_data_t;

		http_data->handler = (Node *)alias->value;
		http_data->data    = data;

		curl_easy_setopt( curl, CURLOPT_NOPROGRESS, 0L );
		curl_easy_setopt( curl, CURLOPT_PROGRESSFUNCTION, http_progress_callback );
		curl_easy_setopt( curl, CURLOPT_PROGRESSDATA, http_data );
	}

	res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);

	if( http_data != NULL ){
		delete http_data;
	}

	return (Object *)gc_new_integer(res);
}
