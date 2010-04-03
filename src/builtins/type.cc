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
#include "context.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

HYBRIS_BUILTIN(hisint){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isint' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>(data->at(0)->type == H_OT_INT) );
}

HYBRIS_BUILTIN(hisfloat){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isfloat' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>(data->at(0)->type == H_OT_FLOAT) );
}

HYBRIS_BUILTIN(hischar){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'ischar' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>(data->at(0)->type == H_OT_CHAR) );
}

HYBRIS_BUILTIN(hisstring){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isstring' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>(data->at(0)->type == H_OT_STRING) );
}

HYBRIS_BUILTIN(hisarray){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isarray' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>(data->at(0)->type == H_OT_ARRAY) );
}

HYBRIS_BUILTIN(hismap){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'ismap' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>(data->at(0)->type == H_OT_MAP) );
}

HYBRIS_BUILTIN(hisalias){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isalias' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>(data->at(0)->type == H_OT_ALIAS) );
}

HYBRIS_BUILTIN(htypeof){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'typeof' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (char *)Object::type_name(data->at(0)) );
}

HYBRIS_BUILTIN(hsizeof){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'sizeof' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( static_cast<long>( data->at(0)->size  ) );
}

HYBRIS_BUILTIN(htoint){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'toint' requires 1 parameter (called with %d)", data->size() );
	}
	return data->at(0)->toInt();
}

HYBRIS_BUILTIN(htostring){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'tostring' requires 1 parameter (called with %d)", data->size() );
	}
	return data->at(0)->toString();
}

/* xml conversion routines */
unsigned int htoi( const char *ptr ){
	unsigned int value(0);
	char ch = *ptr;

    for(;;){
        if (ch >= '0' && ch <= '9')
            value = (value << 4) + (ch - '0');
        else if (ch >= 'A' && ch <= 'F')
            value = (value << 4) + (ch - 'A' + 10);
        else if (ch >= 'a' && ch <= 'f')
            value = (value << 4) + (ch - 'a' + 10);
        else
            return value;

        ch = *(++ptr);
    }
}

Object *xmlNode2Object( xmlNode *node ){
	char *data = (char *)(node->children ? node->children->content : NULL);

	if( strcmp( (char *)node->name, "int" ) == 0 ){
		return new Object( static_cast<long>( atoi(data) ) );
	}
	else if( strcmp( (char *)node->name, "alias" ) == 0 ){
		return new Object( (unsigned int)atoi(data) );
	}
	else if( strcmp( (char *)node->name, "char" ) == 0 ){
		return new Object( (char)data[0] );
	}
	else if( strcmp( (char *)node->name, "float" ) == 0 ){
		return new Object( (double)strtod(data,NULL) );
	}
	else if( strcmp( (char *)node->name, "string" ) == 0 ){
		return new Object( data );
	}
	else if( strcmp( (char *)node->name, "binary" ) == 0 ){
		vector<unsigned char> stream;
		char *bytes   = (char *)data;
        char  byte[3] = {0};

        for( int i = 0; i < strlen(bytes); i += 2 ){
            memcpy( &byte, &bytes[i], 2 );
            stream.push_back( htoi(byte) );
        }
		return new Object(stream);
	}
	else if( strcmp( (char *)node->name, "array" ) == 0 ){
		Object *array  = new Object();
		xmlNode *child = NULL;
		for( child = node->children; child; child = child->next ){
			if( child->type == XML_ELEMENT_NODE) {
				array->push( xmlNode2Object(child) );
			}
		}
		return array;
	}
	else if( strcmp( (char *)node->name, "map" ) == 0 ){
		Object *map      = new Object();
		xmlNode *child   = NULL,
				*mapping = NULL,
				*object  = NULL;
		for( child = node->children; child; child = child->next ){
			if( child->type == XML_ELEMENT_NODE ){
				mapping = child;
				object  = child->next;
				map->map( xmlNode2Object(mapping), xmlNode2Object(object) );
				child = child->next;
			}
		}
		return map;
	}
	else if( strcmp( (char *)node->name, "matrix" ) == 0 ){
        Object *matrix = new Object();
        xmlNode *child = NULL,
                *row   = NULL,
                *item  = NULL;
        int i, j;

        matrix->type            = H_OT_MATRIX;
        matrix->value.m_rows    = 0;
        matrix->value.m_columns = 0;
        matrix->value.m_matrix  = NULL;

        i = 0;

        for( child = node->children; child; child = child->next ){
            if( strcmp( (char *)child->name, "rows" ) == 0 ){
                matrix->value.m_rows   = atoi((char *)child->children->content);
                matrix->value.m_matrix = new Object ** [matrix->value.m_rows];
            }
            else if( strcmp( (char *)child->name, "cols" ) == 0 ){
                matrix->value.m_columns = atoi((char *)child->children->content);
                for( int x = 0; x < matrix->value.m_rows; ++x ){
                    matrix->value.m_matrix[x] = new Object * [matrix->value.m_columns];
                }
            }
            else if( strcmp( (char *)child->name, "row" ) == 0 ){
                for( item = child->children, j = 0; item; item = item->next ){
                    if( item->type == XML_ELEMENT_NODE ){
                        matrix->value.m_matrix[i][j++] = xmlNode2Object(item);
                    }
                }
                ++i;
            }
        }

        return matrix;
	}
	else if( strcmp( (char *)node->name, "struct" ) == 0 ){
	    Object  *structure = new Object();
	    xmlNode *child     = NULL;
	    string   last_attr;

	    structure->type = H_OT_STRUCT;

        for( child = node->children; child; child = child->next ){
            if( strcmp( (char *)child->name, "attribute" ) == 0 ){
                structure->addAttribute( (char *)child->children->content );
                last_attr = (char *)child->children->content;
            }
            else{
                structure->setAttribute( (char *)last_attr.c_str(), xmlNode2Object(child) );
            }
        }

        return structure;
	}
	else{
		hybris_generic_error( "'%s' invalid xml object type", node->name );
	}
}

string Object2Xml( Object *o, unsigned int tabs = 0 ){
	unsigned int i, j;
	char         byte[3] = {0};
	string       xtabs;
	stringstream xml;

	for( i = 0; i < tabs; ++i ){ xtabs += "\t"; }
	switch( o->type ){
		case H_OT_INT    : xml << xtabs << "<int>"    << o->value.m_integer << "</int>\n";    break;
		case H_OT_ALIAS  : xml << xtabs << "<alias>"  << o->value.m_alias   << "</alias>\n";  break;
		case H_OT_FLOAT  : xml << xtabs << "<float>"  << o->value.m_double  << "</float>\n";  break;
		case H_OT_CHAR   : xml << xtabs << "<char>"   << o->value.m_char    << "</char>\n";   break;
		case H_OT_STRING : xml << xtabs << "<string>" << o->value.m_string  << "</string>\n"; break;
		case H_OT_BINARY :
            xml << xtabs << "<binary>\n";
            xml << xtabs << "\t";
            for( i = 0; i < o->size; ++i ){
                sprintf( byte, "%.2X", o->value.m_array[i]->value.m_char );
				xml << byte;
			}
			xml << "\n";
            xml << xtabs << "</binary>\n";
		break;

		case H_OT_ARRAY  :
			xml << xtabs << "<array>\n";
			for( i = 0; i < o->size; ++i ){
				xml << Object2Xml( o->value.m_array[i], tabs + 1 );
			}
			xml << xtabs << "</array>\n";
		break;
		case H_OT_MAP  :
			xml << xtabs << "<map>\n";
			for( i = 0; i < o->size; ++i ){
				xml << Object2Xml( o->value.m_map[i],   tabs + 1 );
				xml << Object2Xml( o->value.m_array[i], tabs + 1 );
			}
			xml << xtabs << "</map>\n";
		break;
		case H_OT_MATRIX :
            xml << xtabs << "<matrix>\n";
            xml << xtabs << "\t" << "<rows>" << o->value.m_rows    << "</rows>\n";
            xml << xtabs << "\t" << "<cols>" << o->value.m_columns << "</cols>\n";

            for( i = 0; i < o->value.m_rows; ++i ){
                xml << xtabs << "\t" << "<row>\n";
                for( j = 0; j < o->value.m_columns; ++j ){
                    xml << Object2Xml( o->value.m_matrix[i][j], tabs + 2 );
                }
                xml << xtabs << "\t" << "</row>\n";
            }
            xml << xtabs << "</matrix>\n";
		break;

		case H_OT_STRUCT :
            xml << xtabs << "<struct>\n";
            for( i = 0; i < o->value.m_struct_names.size(); ++i ){
				xml << xtabs << "\t" << "<attribute>" << o->value.m_struct_names[i] << "</attribute>\n";
				xml << Object2Xml( o->value.m_struct_values[i], tabs + 1 );
			}
            xml << xtabs << "</struct>\n";
		break;

		default :
            hybris_generic_error( "could not convert %s type to xml", Object::type_name(o) );
	}

	return xml.str().c_str();
}


HYBRIS_BUILTIN(hfromxml){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'fromxml' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	string normalized = data->at(0)->value.m_string;
	Object::replace( normalized, "\n", "" );
	Object::replace( normalized, "\t", "" );
	Object::replace( normalized, "\r", "" );

	doc = xmlReadMemory( normalized.c_str(), normalized.size(), NULL, NULL, 0 );
	if( doc == NULL ){
		hybris_generic_error( "could not parse xml object" );
	}

	Object *object = xmlNode2Object( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return object;
}


HYBRIS_BUILTIN(htoxml){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'toxml' requires 1 parameter (called with %d)", data->size() );
	}

	string xml = Object2Xml( data->at(0) );

	return new Object( (char *)xml.c_str() );
}
