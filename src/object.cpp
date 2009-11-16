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
#include "object.h"


const char *Object::type( Object *o ){
	switch(o->xtype){
		case H_OT_INT    : return "int";    break;
		case H_OT_CHAR   : return "char";   break;
		case H_OT_FLOAT  : return "float";  break;
		case H_OT_STRING : return "string"; break;
		case H_OT_ARRAY  : return "array";  break;
		case H_OT_MAP    : return "map";    break;
		case H_OT_ALIAS  : return "alias";  break;
	}
}

Object *Object::fromxml( xmlNode *node ){
	char *data = (char *)(node->children ? node->children->content : NULL);

	if( strcmp( (char *)node->name, "int" ) == 0 ){
		return new Object( (int)atoi(data) );
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
	else if( strcmp( (char *)node->name, "array" ) == 0 ){
		Object *array  = new Object();
		xmlNode *child = NULL;
		for( child = node->children; child; child = child->next ){
			if( child->type == XML_ELEMENT_NODE) {
				array->push( fromxml(child) );
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
				map->map( fromxml(mapping), fromxml(object) );
				child = child->next;
			}
		}
		return map;
	}
	else{
		hybris_generic_error( "'%s' invalid xml object type", node->name );
	}
}

Object *Object::fromxml( char *xml ){
	xmlDoc  *doc  = NULL;
	xmlNode *root = NULL;

	LIBXML_TEST_VERSION

	string normalized = xml;
	replace( normalized, "\n", "" );
	replace( normalized, "\t", "" );
	replace( normalized, "\r", "" );

	doc = xmlReadMemory( normalized.c_str(), normalized.size(), NULL, NULL, 0 );
	if( doc == NULL ){
		hybris_generic_error( "could not parse xml object" );
	}

	Object *object = fromxml( xmlDocGetRootElement(doc) );

	xmlFreeDoc(doc);
	xmlCleanupParser();

	return object;
}

unsigned int Object::assert_type( Object *a, Object *b, unsigned int ntypes, ... ){
	unsigned int o, i, valid = 0;
	Object *objects[] = { a, b };
	va_list ap;

	for( o = 0; o < 2; o++ ){
		if( objects[o] != NULL ){
			valid = 0;
			va_start( ap, ntypes );
			for( i = 0; i < ntypes && valid == 0; i++ ){
				if( objects[o]->xtype == va_arg( ap, int ) ){
					valid = 1;
				}
			}
			va_end(ap);
			if( valid == 0 ){
				return 0;
			}
		}
	}
	return 1;
}

void Object::replace( string &source, const string find, string replace ) {
	size_t j;
	for( ; (j = source.find( find )) != string::npos; ){
		source.replace( j, find.length(), replace );
	}
}

void Object::parse_string( string& s ){
	/* newline */
	replace( s, "\\n", "\n" );
	/* carriage return */
	replace( s, "\\r", "\r" );
	/* horizontal tab */
	replace( s, "\\t", "\t" );
	/* vertical tab */
	replace( s, "\\v", "\v" );
	/* audible bell */
	replace( s, "\\a", "\a" );
	/* backspace */
	replace( s, "\\b", "\b" );
	/* formfeed */
	replace( s, "\\f", "\f" );
	/* escaped double quote */
	replace( s, "\\\"", "\"" );
}

Object::Object( int value ) {
	xtype = H_OT_INT;
	xint  = value;
	xsize = sizeof(int);
	is_extern = 0;
}

Object::Object( int value, unsigned int _is_extern ) {
	xtype = H_OT_INT;
	xint  = value;
	xsize = sizeof(int);
	is_extern = _is_extern;
}

Object::Object( double value ) {
	xtype  = H_OT_FLOAT;
	xfloat = value;
	xsize  = sizeof(double);
	is_extern = 0;
}

Object::Object( char value ) {
	xtype = H_OT_CHAR;
	xchar = value;
	xsize = sizeof(char);
	is_extern = 0;
}

Object::Object( char *value ) {
	xtype   = H_OT_STRING;
	xstring = value;
	parse_string( xstring );
	xsize   = strlen(xstring.c_str()) + 1;
	is_extern = 0;
}

Object::Object(){
	xtype = H_OT_ARRAY;
	xsize = 0;
	is_extern = 0;
}

Object::Object( unsigned int value ){
	xtype  = H_OT_ALIAS;
	xsize  = sizeof(unsigned int);
	xalias = value;
	is_extern = 0;
}

Object::Object( Object *o ) {
	unsigned int i;
	xsize = o->xsize;
	switch( (xtype = o->xtype) ){
		case H_OT_INT    : xint    = o->xint;    break;
		case H_OT_ALIAS  : xalias  = o->xalias;  break;
		case H_OT_FLOAT  : xfloat  = o->xfloat;  break;
		case H_OT_CHAR   : xchar   = o->xchar;   break;
		case H_OT_STRING : xstring = o->xstring; break;
		case H_OT_ARRAY  :
			for( i = 0; i < xsize; i++ ){
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
		case H_OT_MAP    :
			for( i = 0; i < xsize; i++ ){
				xmap.push_back( new Object( o->xmap[i] ) );
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
	}
	is_extern = o->is_extern;
}

Object::Object( FILE *fp ) {
	fread( &xtype, 1, sizeof(xtype), fp );
	fread( &xsize, 1, sizeof(xsize), fp );
	fread( &is_extern, 1, sizeof(is_extern), fp );
	char c;
	unsigned int i;
	switch( xtype ){
		case H_OT_INT    : fread( &xint, 1, xsize, fp );   break;
		case H_OT_ALIAS  : fread( &xalias, 1, xsize, fp ); break;
		case H_OT_FLOAT  : fread( &xfloat, 1, xsize, fp ); break;
		case H_OT_CHAR   : fread( &xchar, 1, xsize, fp );  break;
		case H_OT_STRING :
			for( i = 0; i < xsize; i++ ){
				fread( &c, 1, sizeof(char), fp );
				xstring += c;
			}
		break;
		case H_OT_ARRAY  :
			for( i = 0; i < xsize; i++ ){
				xarray.push_back( new Object(fp) );
			}
		break;
		case H_OT_MAP    :
			for( i = 0; i < xsize; i++ ){
				xmap.push_back( new Object(fp) );
				xarray.push_back( new Object(fp) );
			}
		break;
	}
}

Object::~Object(){
	unsigned int i;
	if( xtype == H_OT_ARRAY ){
		for( i = 0; i < xsize; i++ ){
			delete xarray[i];
		}
	}
	else if( xtype == H_OT_MAP ){
		for( i = 0; i < xsize; i++ ){
			delete xmap[i];
			delete xarray[i];
		}
	}
}

int Object::equals( Object *o ){
	if( xtype != o->xtype ){
		return 0;
	}
	else if( xsize != o->xsize ){
		return 0;
	}
	unsigned int i;
	switch( xtype ){
		case H_OT_INT    : return xint == o->xint;
		case H_OT_ALIAS  : return xalias == o->xalias;
		case H_OT_FLOAT  : return xfloat == o->xfloat;
		case H_OT_CHAR   : return xchar == o->xchar;
		case H_OT_STRING : return xstring == o->xstring;
		case H_OT_ARRAY  :
			for( i = 0; i < xsize; i++ ){
				if( xarray[i]->equals( o->xarray[i] ) == 0 ){
					return 0;
				}
			}
			return 1;
		break;
		case H_OT_MAP    :
			for( i = 0; i < xsize; i++ ){
				if( xmap[i]->equals( o->xmap[i] ) == 0 ){
					return 0;
				}
				if( xarray[i]->equals( o->xarray[i] ) == 0 ){
					return 0;
				}
			}
			return 1;
		break;
	}
}

int Object::mapFind( Object *map ){
	unsigned int i;
	for( i = 0; i < xsize; i++ ){
		if( xmap[i]->equals(map) ){
			return i;
		}
	}
	return -1;
}

Object * Object::getObject(){
	if( xtype != H_OT_ALIAS ){
		hybris_generic_error( "invalid pointer reference" );
	}
	return (Object *)xalias;
}

void Object::compile( FILE *fp ){
	fwrite( &xtype, 1, sizeof(H_OBJECT_TYPE), fp );
	fwrite( &xsize, 1, sizeof(unsigned int), fp );
	fwrite( &is_extern, 1, sizeof(unsigned int), fp );
	unsigned int i;
	switch( xtype ){
		case H_OT_INT    : fwrite( &xint, 1, xsize, fp );           break;
		case H_OT_ALIAS  : fwrite( &xalias, 1, xsize, fp );         break;
		case H_OT_FLOAT  : fwrite( &xfloat, 1, xsize, fp );         break;
		case H_OT_CHAR   : fwrite( &xchar, 1, xsize, fp );          break;
		case H_OT_STRING : fwrite( xstring.c_str(), 1, xsize, fp ); break;
		case H_OT_ARRAY  :
			for( i = 0; i < xsize; i++ ){
				xarray[i]->compile(fp);
			}
		break;
		case H_OT_MAP    :
			for( i = 0; i < xsize; i++ ){
				xmap[i]->compile(fp);
				xarray[i]->compile(fp);
			}
		break;
	}
}

unsigned char *Object::serialize(){
	unsigned char *buffer = new unsigned char[ xsize ];
	unsigned int xmapsize = 0, i, offset = 0;

	memset( buffer, 0x00, xsize );
	switch( xtype ){
		case H_OT_INT    : memcpy( buffer, &xint, xsize );           break;
		case H_OT_ALIAS  : memcpy( buffer, &xalias, xsize );         break;
		case H_OT_FLOAT  : memcpy( buffer, &xfloat, xsize );         break;
		case H_OT_CHAR   : memcpy( buffer, &xchar, xsize );          break;
		case H_OT_STRING : memcpy( buffer, xstring.c_str(), xsize ); break;
		case H_OT_MAP    :
			// compute the map objects size
			for( i = 0; i < xsize; i++ ){
				switch( xarray[i]->xtype ){
					case H_OT_INT    : xmapsize += xarray[i]->xsize; break;
					case H_OT_ALIAS  : xmapsize += xarray[i]->getObject()->xsize; break;
					case H_OT_FLOAT  : xmapsize += xarray[i]->xsize; break;
					case H_OT_CHAR   : xmapsize += xarray[i]->xsize; break;
					case H_OT_STRING : xmapsize += xarray[i]->xsize; break;
					default:
						hybris_generic_error( "invalid map sub type '%s' for field %s", Object::type(xarray[i]), xmap[i]->toString() );
				}
			}

			// alloc new buffer
			delete[] buffer;
			buffer = new unsigned char[ xmapsize ];

			// create the serialized stream looping each map's object
			for( i = 0; i < xsize; i++ ){
				memcpy( (buffer + offset), xarray[i]->serialize(), xarray[i]->xsize );
				offset += xarray[i]->xsize;
			}

		break;

		case H_OT_ARRAY  : hybris_generic_error( "could not serialize an array" ); break;
	}
	return buffer;
}

void Object::print( unsigned int tabs /*= 0*/ ){
	unsigned int i;
	for( i = 0; i < tabs; i++ ) printf( "\t" );
	switch(xtype){
		case H_OT_INT    : printf( "%d",  xint );           break;
		case H_OT_ALIAS  : printf( "0x%X", xalias );        break;
		case H_OT_FLOAT  : printf( "%lf", xfloat );         break;
		case H_OT_CHAR   : printf( "%c", xchar );           break;
		case H_OT_STRING : printf( "%s", xstring.c_str() ); break;
		case H_OT_ARRAY  :
			printf( "array {\n" );
			for( i = 0; i < xsize; i++ ){
				xarray[i]->println(tabs + 1);
			}
			for( i = 0; i < tabs; i++ ) printf( "\t" );
			printf( "}\n" );
			return;
		break;
		case H_OT_MAP  :
			printf( "map {\n" );
			for( i = 0; i < xsize; i++ ){
				xmap[i]->print(tabs + 1);
				printf( " -> " );
				xarray[i]->println(tabs + 1);
			}
			for( i = 0; i < tabs; i++ ) printf( "\t" );
			printf( "}\n" );
			return;
		break;
	}
}

void Object::println( unsigned int tabs /*= 0*/ ){
	print(tabs);
	printf("\n");
}

string Object::toxml( unsigned int tabs /*= 0*/ ){
	unsigned int i;
	string       xtabs;
	stringstream xml;

	for( i = 0; i < tabs; i++ ){ xtabs += "\t"; }
	switch(xtype){
		case H_OT_INT    : xml << xtabs << "<int>"    << xint    << "</int>\n";    break;
		case H_OT_ALIAS  : xml << xtabs << "<alias>"  << xalias  << "</alias>\n";  break;
		case H_OT_FLOAT  : xml << xtabs << "<float>"  << xfloat  << "</float>\n";  break;
		case H_OT_CHAR   : xml << xtabs << "<char>"   << xchar   << "</char>\n";   break;
		case H_OT_STRING : xml << xtabs << "<string>" << xstring << "</string>\n"; break;
		case H_OT_ARRAY  :
			xml << xtabs << "<array>\n";
			for( i = 0; i < xsize; i++ ){
				xml << xarray[i]->toxml( tabs + 1 );
			}
			xml << xtabs << "</array>\n";
		break;
		case H_OT_MAP  :
			xml << xtabs << "<map>\n";
			for( i = 0; i < xsize; i++ ){
				xml << xmap[i]->toxml( tabs + 1 );
				xml << xarray[i]->toxml( tabs + 1 );
			}
			xml << xtabs << "</map>\n";
		break;
	}

	return xml.str().c_str();
}

void Object::input(){
	switch(xtype){
		case H_OT_INT    : cin >> xint;    break;
		case H_OT_FLOAT  : cin >> xfloat;  break;
		case H_OT_CHAR   : cin >> xchar;   break;
		case H_OT_STRING : cin >> xstring; break;
		case H_OT_ARRAY  : hybris_syntax_error( "could not read an array" ); break;
		case H_OT_MAP    : hybris_syntax_error( "could not read a map" ); break;
		case H_OT_ALIAS  : hybris_syntax_error( "could not read an alias" ); break;
	}
}

int Object::lvalue(){
	switch(xtype){
		case H_OT_INT    : return xint;        break;
		case H_OT_ALIAS  : return xalias;      break;
		case H_OT_FLOAT  : return (int)xfloat; break;
		case H_OT_CHAR   : return (int)xchar;  break;
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : return (int)xsize;  break;
	}
}

Object * Object::dot( Object *o ){
	stringstream ret;
	char         tmp[0xFF] = {0};

	switch(xtype){
		case H_OT_INT    : ret << xint;    break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", xalias ); ret << tmp; break;
		case H_OT_FLOAT  : ret << xfloat;  break;
		case H_OT_CHAR   : ret << xchar;   break;
		case H_OT_STRING : ret << xstring; break;
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "'%s' is an invalid type for '.' operator", type(this) ); break;
	}
	switch(o->xtype){
		case H_OT_INT    : ret << o->xint;    break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", o->xalias ); ret << tmp; break;
		case H_OT_FLOAT  : ret << o->xfloat;  break;
		case H_OT_CHAR   : ret << o->xchar;   break;
		case H_OT_STRING : ret << o->xstring; break;
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "'%s' is an invalid type for '.' operator", type(o) ); break;
	}

	return new Object( (char *)ret.str().c_str() );
}

Object * Object::dotequal( Object *o ){
	stringstream ret;
	char         tmp[0xFF] = {0};

	switch(xtype){
		case H_OT_INT    : ret << xint;    break;
		case H_OT_FLOAT  : ret << xfloat;  break;
		case H_OT_CHAR   : ret << xchar;   break;
		case H_OT_STRING : ret << xstring; break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", xalias ); ret << tmp; break;
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "'%s' is an invalid type for '.=' operator", type(this) ); break;
	}
	switch(o->xtype){
		case H_OT_INT    : ret << o->xint;    break;
		case H_OT_FLOAT  : ret << o->xfloat;  break;
		case H_OT_CHAR   : ret << o->xchar;   break;
		case H_OT_STRING : ret << o->xstring; break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", xalias ); ret << tmp; break;
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "'%s' is an invalid type for '.=' operator", type(o) ); break;
	}

	xtype   = H_OT_STRING;
	xstring = ret.str();
	parse_string( xstring );
	xsize   = strlen(xstring.c_str()) + 1;

	return this;
}

Object * Object::toString(){
	stringstream ret;
	switch(xtype){
		case H_OT_INT    : ret << xint;    break;
		case H_OT_FLOAT  : ret << xfloat;  break;
		case H_OT_CHAR   : ret << xchar;   break;
		case H_OT_STRING : ret << xstring; break;
		case H_OT_ALIAS  :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_generic_error( "could not convert '%s' to string", type(this) ); break;
	}
	return new Object( (char *)ret.str().c_str() );
}

string Object::svalue(){
	stringstream ret;
	switch(xtype){
		case H_OT_INT    : ret << xint;      break;
		case H_OT_FLOAT  : ret << xfloat;    break;
		case H_OT_CHAR   : ret << "'"  << xchar   << "'";  break;
		case H_OT_STRING : ret << "\"" << xstring << "\""; break;
		case H_OT_ALIAS  : ret << "<alias>"; break;
		case H_OT_ARRAY  : ret << "<array>"; break;
		case H_OT_MAP    : ret << "<map>";   break;
	}
	return ret.str();
}

Object *Object::push( Object *o ){
	xarray.push_back( new Object(o) );
	xtype = H_OT_ARRAY;
	xsize = xarray.size();
	return this;
}

Object *Object::map( Object *map, Object *o ){
	xmap.push_back( new Object(map) );
	xarray.push_back( new Object(o) );
	xtype = H_OT_MAP;
	xsize = xmap.size();
	return this;
}

Object *Object::pop(){
	if( xtype != H_OT_ARRAY ){
		hybris_generic_error( "could not pop an element from a non array type" );
	}
	Object *element = xarray[ xarray.size() - 1 ];
	xarray.pop_back();
	xsize = xarray.size();
	return element;
}

Object *Object::mapPop(){
	if( xtype != H_OT_MAP ){
		hybris_generic_error( "could not pop an element from a non map type" );
	}
	Object *element = xarray[ xarray.size() - 1 ];
	xarray.pop_back();
	xmap.pop_back();
	xsize = xmap.size();
	return element;
}

Object *Object::remove( Object *index ){
	if( xtype != H_OT_ARRAY ){
		hybris_generic_error( "could not remove an element from a non array type" );
	}
	unsigned int i = index->lvalue();
	Object *element = xarray[i];
	xarray.erase( xarray.begin() + i );
	xsize = xarray.size();
	return element;
}

Object *Object::unmap( Object *map ){
	if( xtype != H_OT_MAP ){
		hybris_generic_error( "could not unmap an element from a non map type" );
	}
	int i = mapFind(map);
	if( i != -1 ){
		Object *element = xarray[i];
		xmap.erase( xmap.begin() + i );
		xarray.erase( xarray.begin() + i );
		xsize = xmap.size();
		return element;
	}
	return this;
}

Object *Object::at( Object *index ){
	if( xtype == H_OT_STRING ){
		return new Object( xstring[ index->lvalue() ] );
	}
	else if( xtype == H_OT_ARRAY ){
		return new Object( xarray[ index->lvalue() ] );
	}
	else if( xtype == H_OT_MAP ){
		int i = mapFind(index);
		if( i != -1 ){
			return new Object( xarray[i] );
		}
		else{
			hybris_generic_error( "no mapped values for label '%s'", index->toString() );
		}
	}
	else{
		hybris_syntax_error( "'%s' is an invalid type for subscript operator", type(this) );
	}
}

Object *Object::at( char *map ){
	Object tmp(map);
	return xarray[mapFind(&tmp)];
}

Object& Object::at( Object *index, Object *set ){
	if( xtype == H_OT_STRING ){
		xstring[ index->lvalue() ] = (char)set->lvalue();
	}
	else if( xtype == H_OT_ARRAY ){
		delete xarray[ index->lvalue() ];
		xarray[ index->lvalue() ] = new Object(set);
	}
	else if( xtype == H_OT_MAP ){
		int i = mapFind(index);
		if( i != -1 ){
			delete xarray[i];
			xarray[i] = new Object(set);
		}
		else{
			map( index, set );
		}
	}
	else{
		hybris_syntax_error( "'%s' is an invalid type for subscript operator", type(this) );
	}

	return *this;
}

Object& Object::operator = ( Object *o ){
	unsigned int i;
	switch( o->xtype ){
		case H_OT_INT    : xint    = o->xint;    xsize = o->xsize; xtype = o->xtype; break;
		case H_OT_ALIAS  : xalias  = o->xalias;  xsize = o->xsize; xtype = o->xtype; break;
		case H_OT_FLOAT  : xfloat  = o->xfloat;  xsize = o->xsize; xtype = o->xtype; break;
		case H_OT_CHAR   : xchar   = o->xchar;   xsize = o->xsize; xtype = o->xtype; break;
		case H_OT_STRING : xstring = o->xstring; xsize = o->xsize; xtype = o->xtype; break;
		case H_OT_ARRAY  :
			if( xtype == H_OT_ARRAY ){
				for( i = 0; i < xsize; i++ ){
					delete xarray[i];
				}
				xarray.clear();
			}

			xtype = o->xtype;
			xsize = o->xsize;
			for( i = 0; i < xsize; i++ ){
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
		case H_OT_MAP    :
			if( xtype == H_OT_ARRAY || xtype == H_OT_MAP ){
				for( i = 0; i < xsize; i++ ){
					delete xarray[i];
				}
				xarray.clear();
				if( xtype == H_OT_MAP ){
					for( i = 0; i < xsize; i++ ){
						delete xmap[i];
					}
					xmap.clear();
				}
			}

			xtype = o->xtype;
			xsize = o->xsize;
			for( i = 0; i < xsize; i++ ){
				xmap.push_back( new Object( o->xmap[i] ) );
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
	}
	return *this;
}

Object * Object::operator - (){
	switch(xtype){
		case H_OT_INT    : return new Object( -xint );   break;
		case H_OT_FLOAT  : return new Object( -xfloat ); break;
		case H_OT_CHAR   : return new Object( -xchar );  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for unary minus operator" );
	}
}

Object * Object::operator ! (){
	switch(xtype){
		case H_OT_INT    : return new Object( !xint );   break;
		case H_OT_FLOAT  : return new Object( !xfloat ); break;
		case H_OT_CHAR   : return new Object( !xchar );  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for unary not operator" );
	}
}

Object& Object::operator ++ (){
	switch(xtype){
		case H_OT_INT    : xint++;   break;
		case H_OT_FLOAT  : xfloat++; break;
		case H_OT_CHAR   : xchar++;  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for increment operator" );
	}
	return *this;
}

Object& Object::operator -- (){
	switch(xtype){
		case H_OT_INT    : xint--;   break;
		case H_OT_FLOAT  : xfloat--; break;
		case H_OT_CHAR   : xchar--;  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for decrement operator" );
	}
	return *this;
}

Object * Object::operator + ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for addition operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( xfloat + (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( o->xfloat + (xtype == H_OT_FLOAT ? xfloat : lvalue()) );
	}
	else{
		return new Object( lvalue() + o->lvalue() );
	}
}

Object * Object::operator += ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for addition operator" );
	}

	if( xtype == H_OT_FLOAT ){
		xfloat += (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_CHAR ){
		xchar += (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else{
		xint += (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	return this;
}

Object * Object::operator - ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for subtraction operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( xfloat - (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( o->xfloat - (xtype == H_OT_FLOAT ? xfloat : lvalue()) );
	}
	else{
		return new Object( lvalue() - o->lvalue() );
	}
}

Object * Object::operator -= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for subtraction operator" );
	}

	if( xtype == H_OT_FLOAT ){
		xfloat -= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_CHAR ){
		xchar -= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else{
		xint -= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	return this;
}

Object * Object::operator * ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for multiplication operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( xfloat * (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( o->xfloat * (xtype == H_OT_FLOAT ? xfloat : lvalue()) );
	}
	else{
		return new Object( lvalue() * o->lvalue() );
	}
}

Object * Object::operator *= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for multiplication operator" );
	}

	if( xtype == H_OT_FLOAT ){
		xfloat *= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_CHAR ){
		xchar *= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else{
		xint *= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
}

Object * Object::operator / ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for division operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( xfloat / (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( o->xfloat / (xtype == H_OT_FLOAT ? xfloat : lvalue()) );
	}
	else{
		return new Object( lvalue() / o->lvalue() );
	}
}

Object * Object::operator /= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for division operator" );
	}

	if( xtype == H_OT_FLOAT ){
		xfloat /= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_CHAR ){
		xchar /= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else{
		xint /= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	return this;
}

Object * Object::operator % ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for modulus operator" );
	}

	return new Object( lvalue() % o->lvalue() );
}

Object * Object::operator %= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for modulus operator" );
	}

	int val = lvalue() % o->lvalue();
	if( xtype == H_OT_INT ){
		xint = val;
	}
	else{
		xchar = (char)val;
	}
}

Object * Object::operator & ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for and operator" );
	}

	return new Object( lvalue() & o->lvalue() );
}

Object * Object::operator &= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for and operator" );
	}

	int val = lvalue() & o->lvalue();
	if( xtype == H_OT_INT ){
		xint = val;
	}
	else{
		xchar = (char)val;
	}
	return this;
}

Object * Object::operator | ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for or operator" );
	}

	return new Object( lvalue() | o->lvalue() );
}

Object * Object::operator |= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for or operator" );
	}

	int val = lvalue() | o->lvalue();
	if( xtype == H_OT_INT ){
		xint = val;
	}
	else{
		xchar = (char)val;
	}
	return this;
}

Object * Object::operator << ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for left shift operator" );
	}

	return new Object( lvalue() << o->lvalue() );
}

Object * Object::operator <<= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for left shift operator" );
	}

	int val = lvalue() << o->lvalue();
	if( xtype == H_OT_INT ){
		xint = val;
	}
	else{
		xchar = (char)val;
	}
	return this;
}

Object * Object::operator >> ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for right shift operator" );
	}

	return new Object( lvalue() >> o->lvalue() );
}

Object * Object::operator >>= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for right shift operator" );
	}

	int val = lvalue() >> o->lvalue();
	if( xtype == H_OT_INT ){
		xint = val;
	}
	else{
		xchar = (char)val;
	}
	return this;
}

Object * Object::operator ^ ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for xor operator" );
	}

	return new Object( lvalue() ^ o->lvalue() );
}

Object * Object::operator ^= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for xor operator" );
	}

	int val = lvalue() ^ o->lvalue();
	if( xtype == H_OT_INT ){
		xint = val;
	}
	else{
		xchar = (char)val;
	}
	return this;
}

Object * Object::operator ~ (){
	if( assert_type( this, NULL, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for not operator" );
	}

	return new Object( ~lvalue() );
}

Object * Object::lnot (){
	if( assert_type( this, NULL, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for logical not operator" );
	}

	return new Object( !lvalue() );
}

Object * Object::operator == ( Object *o ){
	if( xtype == H_OT_STRING && o->xtype == H_OT_STRING ){
		return new Object( (xstring == o->xstring) );
	}
	else{
		return new Object( lvalue() == o->lvalue() );
	}
}

Object * Object::operator != ( Object *o ){
	if( xtype == H_OT_STRING && o->xtype == H_OT_STRING ){
		return new Object( (xstring != o->xstring) );
	}
	else{
		return new Object( lvalue() != o->lvalue() );
	}
}

Object * Object::operator < ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for comparision operator" );
	}

	return new Object( lvalue() < o->lvalue() );
}

Object * Object::operator > ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for comparision operator" );
	}

	return new Object( lvalue() > o->lvalue() );
}

Object * Object::operator <= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for comparision operator" );
	}

	return new Object( lvalue() <= o->lvalue() );
}

Object * Object::operator >= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for comparision operator" );
	}

	return new Object( lvalue() >= o->lvalue() );
}

Object * Object::operator || ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for logical operator" );
	}

	return new Object( lvalue() || o->lvalue() );
}

Object * Object::operator && ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for logical operator" );
	}

	return new Object( lvalue() && o->lvalue() );
}
