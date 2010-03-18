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
#include "common.h"
#include "vmem.h"
#include "builtin.h"

int xml_isinvalid( char *str ){
	if( str != NULL ){
		int len = strlen(str), i;
		for( i = 0; i < len; i++ ){
			if( !isspace(str[i]) ){
				return 0;
			}
		}
	}
	return 1;
}

Object *xml_traverse( xmlNode *node ){
	if( node->type == XML_ELEMENT_NODE ){
		/* check for empty names (usually indentation) */
		if( xml_isinvalid((char *)node->name) == 0 ){
			Object *hnode = new Object();
			/* create attributes array */
			hnode->map( new Object((char *)"<attributes>"), new Object() );
			Object *attributes = hnode->at((char *)"<attributes>");
			for( xmlAttr *a = node->properties; a; a = a->next ){
				const xmlChar *value = xmlNodeGetContent(a->children);
				attributes->map( new Object((char *)a->name), new Object((char *)value) );
				xmlFree((void*)value);
			}
			/* start children evaluation */
			for( xmlNode *child = node->children; child; child = child->next ){
				/* child element */
				if( child->type == XML_ELEMENT_NODE ){
					/* create a map for the child if it doesn't exist yet */
					Object childname((char *)child->name);
					if( hnode->mapFind(&childname) == -1 ){
						hnode->map( new Object((char *)child->name), new Object() );
					}
					/* push the child into the array */
					hnode->at((char *)child->name)->push( xml_traverse(child) );
				}
				/* node raw content */
				else{
					const xmlChar *content = xmlNodeGetContent(child);
					if( xml_isinvalid((char *)content) == 0 ){
						/* add the content */
						hnode->map( new Object((char *)"<data>"), new Object((char *)content) );
						xmlFree((void*)content);
					}
				}
			}
			return hnode;
		}
	}
	return new Object((char *)"");
}

HYBRIS_BUILTIN(hxml_load){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'xml_load' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

	string   filename = data->at(0)->xstring;
	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	doc = xmlReadFile( filename.c_str(), NULL, 0);
	if( doc == NULL ){
		hybris_generic_error( "error loading or parsing '%s'", filename.c_str() );
	}

	Object *hmap = xml_traverse( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return hmap;
}

HYBRIS_BUILTIN(hxml_parse){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'xml_parse' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

	string   xml  = data->at(0)->xstring;
	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	doc = xmlReadMemory( xml.c_str(), xml.size(), NULL, NULL, 0);
	if( doc == NULL ){
		hybris_generic_error( "error parsing xml buffer" );
	}

	Object *hmap = xml_traverse( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return hmap;
}
