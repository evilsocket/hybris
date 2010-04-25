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

HYBRIS_EXPORTED_FUNCTIONS() {
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
    Object *h_xmlNode = ob_clone(__xmlNode_type);
	if( node->type == XML_ELEMENT_NODE ){
	    /* check for empty names (usually indentation) */
		if( xml_isinvalid((char *)node->name) == 0 ){
			MapObject *h_xmlAttributes = gc_new_map(),
					  *h_xmlChildren   = gc_new_map();

            /* set node name */
            ob_set_attribute_reference( h_xmlNode, "name", ob_dcast( gc_new_string(node->name) ) );

			/* fill attributes map */
			for( xmlAttr *a = node->properties; a; a = a->next ){
				const xmlChar *value = xmlNodeGetContent(a->children);

				ob_cl_set_reference( ob_dcast(h_xmlAttributes),
									 ob_dcast( gc_new_string(a->name) ),
									 ob_dcast( gc_new_string(value) ) );

				xmlFree((void*)value);
			}
            /* set attributes */
            ob_set_attribute_reference( h_xmlNode, "attributes", ob_dcast(h_xmlAttributes) );

			/* start children evaluation */
			for( xmlNode *child = node->children; child; child = child->next ){
				/* child element */
				if( child->type == XML_ELEMENT_NODE ){
                    /* create a map for the child if it doesn't exist yet */
					Object *childname = ob_dcast( gc_new_string( (char *)child->name ) ),
						   *vchildren = H_UNDEFINED;

					if( map_find( ob_dcast(h_xmlChildren), childname ) == -1 ){
						vchildren = ob_dcast(gc_new_vector());
						ob_cl_set_reference( ob_dcast(h_xmlChildren),
											 childname,
											 vchildren );
					}
					else{
						vchildren = ob_cl_at( ob_dcast(h_xmlChildren), childname );
					}
					/* push the child into the array */
					ob_cl_push_reference( vchildren, xml_traverse(child) );
				}
				/* node raw content */
				else{
					const xmlChar *content = xmlNodeGetContent(child);
					if( xml_isinvalid((char *)content) == 0 ){
						/* add the content */
						ob_set_attribute_reference( h_xmlNode, "data", ob_dcast(gc_new_string(content)) );
						xmlFree((void*)content);
					}
				}
			}
			/* set children */
			ob_set_attribute_reference( h_xmlNode, "children", ob_dcast(h_xmlChildren) );
		}
	}

	return h_xmlNode;
}

HYBRIS_DEFINE_FUNCTION(hxml_load){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'xml_load' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

	string   filename = string_argv(0);
	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	doc = xmlReadFile( filename.c_str(), NULL, 0);
	if( doc == NULL ){
		hyb_error( H_ET_GENERIC, "error loading or parsing '%s'", filename.c_str() );
	}

	Object *node = xml_traverse( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return node;
}

HYBRIS_DEFINE_FUNCTION(hxml_parse){
	if( ob_argc() != 1 ){
		hyb_error( H_ET_SYNTAX, "function 'xml_parse' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString );

	string   xml  = string_argv(0);
	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	doc = xmlReadMemory( xml.c_str(), xml.size(), NULL, NULL, 0);
	if( doc == NULL ){
		hyb_error( H_ET_GENERIC, "error parsing xml buffer" );
	}

	Object *node = xml_traverse( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return node;
}
