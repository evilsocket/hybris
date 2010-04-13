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
#include <sstream>
#include <hybris.h>

using namespace std;

HYBRIS_DEFINE_FUNCTION(hisint);
HYBRIS_DEFINE_FUNCTION(hisfloat);
HYBRIS_DEFINE_FUNCTION(hischar);
HYBRIS_DEFINE_FUNCTION(hisstring);
HYBRIS_DEFINE_FUNCTION(hisarray);
HYBRIS_DEFINE_FUNCTION(hismap);
HYBRIS_DEFINE_FUNCTION(hisalias);
HYBRIS_DEFINE_FUNCTION(htypeof);
HYBRIS_DEFINE_FUNCTION(hsizeof);
HYBRIS_DEFINE_FUNCTION(htoint);
HYBRIS_DEFINE_FUNCTION(htostring);
HYBRIS_DEFINE_FUNCTION(hfromxml);
HYBRIS_DEFINE_FUNCTION(htoxml);

extern "C" named_function_t hybris_module_functions[] = {
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
	{ "fromxml", hfromxml },
	{ "toxml", htoxml },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hisint){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'isint' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->code == otInteger ) );
}

HYBRIS_DEFINE_FUNCTION(hisfloat){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'isfloat' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->code == otFloat ) );
}

HYBRIS_DEFINE_FUNCTION(hischar){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'ischar' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->code == otChar ) );
}

HYBRIS_DEFINE_FUNCTION(hisstring){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'isstring' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->code == otString ) );
}

HYBRIS_DEFINE_FUNCTION(hisarray){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'isarray' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->code == otVector ) );
}

HYBRIS_DEFINE_FUNCTION(hismap){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'ismap' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->code == otMap ) );
}

HYBRIS_DEFINE_FUNCTION(hisalias){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'isalias' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->code == otAlias ) );
}

HYBRIS_DEFINE_FUNCTION(htypeof){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'typeof' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_STRING_OBJ( HYB_ARGV(0)->type->name ) );
}

HYBRIS_DEFINE_FUNCTION(hsizeof){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'sizeof' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return OB_DOWNCAST( MK_INT_OBJ( HYB_ARGV(0)->type->size ) );
}

HYBRIS_DEFINE_FUNCTION(htoint){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'toint' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return ob_to_int( HYB_ARGV(0) );
}

HYBRIS_DEFINE_FUNCTION(htostring){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'tostring' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	return ob_to_string( HYB_ARGV(0) );
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
		return OB_DOWNCAST( MK_INT_OBJ( atoi(data) ) );
	}
	else if( strcmp( (char *)node->name, "alias" ) == 0 ){
		return OB_DOWNCAST( MK_ALIAS_OBJ( atoi(data) ) );
	}
	else if( strcmp( (char *)node->name, "char" ) == 0 ){
		return OB_DOWNCAST( MK_CHAR_OBJ( data[0] ) );
	}
	else if( strcmp( (char *)node->name, "float" ) == 0 ){
		return OB_DOWNCAST( MK_FLOAT_OBJ( strtod(data,NULL) ) );
	}
	else if( strcmp( (char *)node->name, "string" ) == 0 ){
		return OB_DOWNCAST( MK_STRING_OBJ( data ) );
	}
	else if( strcmp( (char *)node->name, "binary" ) == 0 ){
		vector<unsigned char> stream;
		char *bytes   = (char *)data;
		char  byte[3] = {0};

		for( int i = 0; i < strlen(bytes); i += 2 ){
			memcpy( &byte, &bytes[i], 2 );
			stream.push_back( htoi(byte) );
		}
		return OB_DOWNCAST( MK_BINARY_OBJ(stream) );
	}
	else if( strcmp( (char *)node->name, "array" ) == 0 ){
		Object *array  = OB_DOWNCAST( MK_VECTOR_OBJ() );
		xmlNode *child = NULL;
		for( child = node->children; child; child = child->next ){
			if( child->type == XML_ELEMENT_NODE) {
				ob_cl_push_reference( array, xmlNode2Object(child) );
			}
		}
		return OB_DOWNCAST( array );
	}
	else if( strcmp( (char *)node->name, "map" ) == 0 ){
		Object *map      = OB_DOWNCAST( MK_MAP_OBJ() );
		xmlNode *child   = NULL,
				*mapping = NULL,
				*object  = NULL;
		for( child = node->children; child; child = child->next ){
			if( child->type == XML_ELEMENT_NODE ){
				mapping = child;
				object  = child->next;

				ob_cl_set_reference( map, xmlNode2Object(mapping), xmlNode2Object(object) );

				child = child->next;
			}
		}
		return OB_DOWNCAST( map );
	}
	else if( strcmp( (char *)node->name, "matrix" ) == 0 ){
		vector<Object *> dummy;
		MatrixObject *matrix = MK_MATRIX_OBJ(0,0,dummy);
		xmlNode *child = NULL,
				*row   = NULL,
				*item  = NULL;
		int i, j;

		matrix->rows    = 0;
		matrix->columns = 0;
		matrix->items   = 0;
		matrix->matrix  = NULL;

		i = 0;

		for( child = node->children; child; child = child->next ){
			if( strcmp( (char *)child->name, "rows" ) == 0 ){
				matrix->rows   = atoi((char *)child->children->content);
				matrix->matrix = new Object ** [matrix->rows];
			}
			else if( strcmp( (char *)child->name, "cols" ) == 0 ){
				matrix->columns = atoi((char *)child->children->content);
				for( int x = 0; x < matrix->rows; ++x ){
					matrix->matrix[x] = new Object * [matrix->columns];
				}
			}
			else if( strcmp( (char *)child->name, "row" ) == 0 ){
				for( item = child->children, j = 0; item; item = item->next ){
					if( item->type == XML_ELEMENT_NODE ){
						matrix->matrix[i][j++] = xmlNode2Object(item);
						matrix->items++;
					}
				}
				++i;
			}
		}

		return OB_DOWNCAST( matrix );
	}
	else if( strcmp( (char *)node->name, "struct" ) == 0 ){
		Object  *structure = OB_DOWNCAST( MK_STRUCT_OBJ() );
		xmlNode *child     = NULL;
		string   last_attr;

		for( child = node->children; child; child = child->next ){
			if( strcmp( (char *)child->name, "attribute" ) == 0 ){
				ob_add_attribute( structure, (char *)child->children->content );

				last_attr = (char *)child->children->content;
			}
			else{
				ob_set_attribute_reference( structure, (char *)last_attr.c_str(), xmlNode2Object(child) );
			}
		}

		return OB_DOWNCAST( structure );
	}
	else{
		hyb_throw( H_ET_GENERIC, "'%s' invalid xml object type", node->name );
	}
}

string Object2Xml( Object *o, unsigned int tabs = 0 ){
	unsigned int i, j;
	char         byte[3] = {0};
	string       xtabs;
	stringstream xml;

	for( i = 0; i < tabs; ++i ){ xtabs += "\t"; }
	switch( o->type->code ){
		case otInteger :
		case otAlias   :
		case otFloat   :
		case otChar    :
		case otString  :
			xml << xtabs << "<" << o->type->name << ">" << ob_svalue(o) << "</" << o->type->name << ">\n";
		break;

		case otBinary :
            xml << xtabs << "<binary>\n";
            xml << xtabs << "\t";
            for( i = 0; i < BINARY_UPCAST(o)->items; ++i ){
                sprintf( byte, "%.2X", CHAR_UPCAST( BINARY_UPCAST(o)->value[i] )->value );
				xml << byte;
			}
			xml << "\n";
            xml << xtabs << "</binary>\n";
		break;

		case otVector  :
			xml << xtabs << "<array>\n";
			for( i = 0; i < VECTOR_UPCAST(o)->items; ++i ){
				xml << Object2Xml( VECTOR_UPCAST(o)->value[i], tabs + 1 );
			}
			xml << xtabs << "</array>\n";
		break;
		case otMap  :
			xml << xtabs << "<map>\n";
			for( i = 0; i < MAP_UPCAST(o)->items; ++i ){
				xml << Object2Xml( MAP_UPCAST(o)->keys[i],   tabs + 1 );
				xml << Object2Xml( MAP_UPCAST(o)->values[i], tabs + 1 );
			}
			xml << xtabs << "</map>\n";
		break;
		case otMatrix :
            xml << xtabs << "<matrix>\n";
            xml << xtabs << "\t" << "<rows>" << MATRIX_UPCAST(o)->rows    << "</rows>\n";
            xml << xtabs << "\t" << "<cols>" <<MATRIX_UPCAST(o)->columns << "</cols>\n";

            for( i = 0; i < MATRIX_UPCAST(o)->rows; ++i ){
                xml << xtabs << "\t" << "<row>\n";
                for( j = 0; j < MATRIX_UPCAST(o)->columns; ++j ){
                    xml << Object2Xml( MATRIX_UPCAST(o)->matrix[i][j], tabs + 2 );
                }
                xml << xtabs << "\t" << "</row>\n";
            }
            xml << xtabs << "</matrix>\n";
		break;

		case otStructure :
            xml << xtabs << "<struct>\n";
            for( i = 0; i < STRUCT_UPCAST(o)->items; ++i ){
				xml << xtabs << "\t" << "<attribute>" << STRUCT_UPCAST(o)->names[i] << "</attribute>\n";
				xml << Object2Xml( STRUCT_UPCAST(o)->values[i], tabs + 1 );
			}
            xml << xtabs << "</struct>\n";
		break;

		default :
            hyb_throw( H_ET_GENERIC, "could not convert %s type to xml", o->type->name );
	}

	return xml.str();
}


HYBRIS_DEFINE_FUNCTION(hfromxml){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'fromxml' requires 1 parameter (called with %d)", HYB_ARGC() );
	}
	HYB_TYPE_ASSERT( HYB_ARGV(0), otString );

	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	string normalized = STRING_ARGV(0);

	string_replace( normalized, "\n", "" );
	string_replace( normalized, "\t", "" );
	string_replace( normalized, "\r", "" );

	doc = xmlReadMemory( normalized.c_str(), normalized.size(), NULL, NULL, 0 );
	if( doc == NULL ){
		hyb_throw( H_ET_GENERIC, "could not parse xml object" );
	}

	Object *object = xmlNode2Object( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return object;
}


HYBRIS_DEFINE_FUNCTION(htoxml){
	if( HYB_ARGC() != 1 ){
		hyb_throw( H_ET_SYNTAX, "function 'toxml' requires 1 parameter (called with %d)", HYB_ARGC() );
	}

	string xml = Object2Xml( HYB_ARGV(0) );

	return OB_DOWNCAST( MK_STRING_OBJ( xml.c_str() ) );
}
