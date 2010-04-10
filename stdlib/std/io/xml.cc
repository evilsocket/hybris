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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hxml_load);
HYBRIS_DEFINE_FUNCTION(hxml_parse);

extern "C" named_function_t hybris_module_functions[] = {
	{ "xml_load", hxml_load },
	{ "xml_parse", hxml_parse },
	{ "", NULL }
};

static Object *__xmlNode_type = H_UNDEFINED;

extern "C" void hybris_module_init( Context * ctx ){
    char *xmlNode_attributes[] = { "name", "attributes", "data", "children" };

    __xmlNode_type = HYBRIS_DEFINE_STRUCTURE( ctx, "xmlNode", 4, xmlNode_attributes );
}

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
    Object *h_xmlNode = MK_CLONE_OBJ(__xmlNode_type);
	if( node->type == XML_ELEMENT_NODE ){
	    /* check for empty names (usually indentation) */
		if( xml_isinvalid((char *)node->name) == 0 ){
			Object *h_xmlAttributes = MK_TMP_COLLECTION_OBJ(),
                   *h_xmlChildren   = MK_TMP_COLLECTION_OBJ();

            /* set node name */
			h_xmlNode->setAttribute( "name", MK_TMP_STRING_OBJ(node->name) );

			/* fill attributes array */
			for( xmlAttr *a = node->properties; a; a = a->next ){
				const xmlChar *value = xmlNodeGetContent(a->children);
				h_xmlAttributes->map( MK_TMP_STRING_OBJ(a->name), MK_TMP_STRING_OBJ(value) );
				xmlFree((void*)value);
			}
            /* set attributes */
            h_xmlNode->setAttribute( "attributes", h_xmlAttributes );

			/* start children evaluation */
			for( xmlNode *child = node->children; child; child = child->next ){
				/* child element */
				if( child->type == XML_ELEMENT_NODE ){
                    /* create a map for the child if it doesn't exist yet */
					Object childname((char *)child->name);
					if( h_xmlChildren->mapFind(&childname) == -1 ){
						h_xmlChildren->map( MK_TMP_STRING_OBJ(child->name), MK_TMP_COLLECTION_OBJ() );
					}
					/* push the child into the array */
					h_xmlChildren->at((char *)child->name)->push_ref( xml_traverse(child) );
				}
				/* node raw content */
				else{
					const xmlChar *content = xmlNodeGetContent(child);
					if( xml_isinvalid((char *)content) == 0 ){
						/* add the content */
						h_xmlNode->setAttribute( "data", MK_TMP_STRING_OBJ(content) );
						xmlFree((void*)content);
					}
				}
			}
			/* set children */
			h_xmlNode->setAttribute( "children", h_xmlChildren );
		}
	}

	return h_xmlNode;
}

HYBRIS_DEFINE_FUNCTION(hxml_load){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'xml_load' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

	string   filename = (const char *)(*HYB_ARGV(0));
	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	doc = xmlReadFile( filename.c_str(), NULL, 0);
	if( doc == NULL ){
		hyb_throw( H_ET_GENERIC, "error loading or parsing '%s'", filename.c_str() );
	}

	Object *node = xml_traverse( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return node;
}

HYBRIS_DEFINE_FUNCTION(hxml_parse){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'xml_parse' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), H_OT_STRING );

	string   xml  = (const char *)(*HYB_ARGV(0));
	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	doc = xmlReadMemory( xml.c_str(), xml.size(), NULL, NULL, 0);
	if( doc == NULL ){
		hyb_throw( H_ET_GENERIC, "error parsing xml buffer" );
	}

	Object *node = xml_traverse( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return node;
}
