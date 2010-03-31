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
		case H_OT_MATRIX : return "matrix"; break;
	}
}

#ifdef XML_SUPPORT
Object *Object::fromxml( xmlNode *node ){
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
#endif

unsigned int Object::assert_type( Object *a, Object *b, unsigned int ntypes, ... ){
	unsigned int o, i, valid = 0;
	Object *objects[] = { a, b };
	va_list ap;

	for( o = 0; o < 2; ++o ){
		if( objects[o] != NULL ){
			valid = 0;
			va_start( ap, ntypes );
			for( i = 0; i < ntypes && valid == 0; ++i ){
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

	// handle hex characters
	size_t j, i;
	for( ; (j = s.find( "\\x" )) != string::npos; ){
		string s_hex, repl;
		long   l_hex;
		for( i = j + 2; i < s.length(); ++i ){
            // hex digit ?
            if( (s[i] >= 'A' && s[i] <= 'F') || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >= '0' && s[i] <= '9') ){
                s_hex += s[i];
            }
            else{
                break;
            }
		}

		l_hex  = strtol( ( "0x" + s_hex ).c_str(), 0, 16 );
        repl  += (char)l_hex;
        replace( s, "\\x" + s_hex, repl );
	}
}

#ifdef MEM_DEBUG
void * Object::operator new (size_t size){
    printf( "[MEM DEBUG] new object (+ %d bytes)\n", size );
    return ::new Object;
}
#endif

Object::Object( long value ) :
    xtype(H_OT_INT),
    xint(value),
    xsize(sizeof(long)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{

}

Object::Object( long value, unsigned int _is_extern ) :
    xtype(H_OT_INT),
    xint(value),
    xsize(sizeof(long)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
	if( _is_extern ){
        attributes |= H_OA_EXTERN;
	}
}

Object::Object( double value ) :
    xtype(H_OT_FLOAT),
    xfloat(value),
    xsize(sizeof(double)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{

}

Object::Object( char value ) :
    xtype(H_OT_CHAR),
    xchar(value),
    xsize(sizeof(char)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{

}

Object::Object( char *value ) :
    xtype(H_OT_STRING),
    xstring(value),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
	parse_string( xstring );
	xsize   = strlen(xstring.c_str()) + 1;
}

Object::Object() :
    xtype(H_OT_ARRAY),
    xsize(0),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{

}

Object::Object( unsigned int value ) :
    xtype(H_OT_ALIAS),
    xsize(sizeof(unsigned int)),
    xalias(value),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{

}

Object::Object( unsigned int rows, unsigned int columns, vector<Object *>& data ) :
    xtype(H_OT_MATRIX),
    xsize(0),
    xrows(rows),
    xcolumns(columns),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    unsigned int x, y;

    xmatrix = new Object ** [rows];
    for( x = 0; x < rows; ++x ){
        xmatrix[x] = new Object * [columns];
    }

    if( data.size() ){
        for( x = 0; x < rows; ++x ){
            for( y = 0; y < columns; ++y ){
                Object * o    = data[ x * columns + y ];
                xmatrix[x][y] = o;
                xsize        += o->xsize;
            }
        }
    }
    else{
        for( x = 0; x < rows; ++x ){
            for( y = 0; y < columns; ++y ){
                Object * o    = new Object( static_cast<long>(0) );
                xmatrix[x][y] = o;
                xsize        += o->xsize;
            }
        }
    }
}

Object::Object( Object *o ) :
    xsize(o->xsize),
    xtype(o->xtype),
    attributes(o->attributes)
{
	unsigned int i, j;

	switch( xtype ){
		case H_OT_INT    : xint    = o->xint;    break;
		case H_OT_ALIAS  : xalias  = o->xalias;  break;
		case H_OT_FLOAT  : xfloat  = o->xfloat;  break;
		case H_OT_CHAR   : xchar   = o->xchar;   break;
		case H_OT_STRING : xstring = o->xstring; break;
		case H_OT_ARRAY  :
			for( i = 0; i < xsize; ++i ){
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
		case H_OT_MAP    :
			for( i = 0; i < xsize; ++i ){
				xmap.push_back( new Object( o->xmap[i] ) );
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
		case H_OT_MATRIX :
            xrows    = o->xrows;
            xcolumns = o->xcolumns;
            xmatrix  = new Object ** [xrows];
            for( i = 0; i < xrows; ++i ){
                xmatrix[i] = new Object * [xcolumns];
            }
            for( i = 0; i < xrows; ++i ){
                for( j = 0; j < xcolumns; ++j ){
                    xmatrix[i][j] = new Object( o->xmatrix[i][j] );
                }
             }
		break;
	}
	attributes = o->attributes;
}

void Object::setGarbageAttribute( H_OBJECT_ATTRIBUTE mask ){
    unsigned int i, j;

    /* not garbage */
    if( mask == ~H_OA_GARBAGE ){
        attributes &= mask;
    }
    /* garbage */
    else{
        attributes |= mask;
    }
    /* eventually handle children */
    switch( xtype ){
        case H_OT_ARRAY  :
            for( i = 0; i < xsize; ++i ){
                xarray[i]->setGarbageAttribute(mask);
            }
        break;

        case H_OT_MAP    :
            for( i = 0; i < xsize; ++i ){
                xarray[i]->setGarbageAttribute(mask);
                xmap[i]->setGarbageAttribute(mask);
            }
        break;

        case H_OT_MATRIX :
            for( i = 0; i < xrows; ++i ){
                for( j = 0; j < xcolumns; ++j ){
                    xmatrix[i][j]->setGarbageAttribute(mask);
                }
            }
        break;
    }
}

Object& Object::assign( Object *o ){
    unsigned int i, j;

	release(false);

    xtype = o->xtype;
    xsize = o->xsize;
	switch( o->xtype ){
		case H_OT_INT    : xint    = o->xint;    break;
		case H_OT_ALIAS  : xalias  = o->xalias;  break;
		case H_OT_FLOAT  : xfloat  = o->xfloat;  break;
		case H_OT_CHAR   : xchar   = o->xchar;   break;
		case H_OT_STRING : xstring = o->xstring; break;
		case H_OT_ARRAY  :
			for( i = 0; i < xsize; ++i ){
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
		case H_OT_MAP    :
			for( i = 0; i < xsize; ++i ){
				xmap.push_back( new Object( o->xmap[i] ) );
				xarray.push_back( new Object( o->xarray[i] ) );
			}
		break;
		case H_OT_MATRIX :
            xrows    = o->xrows;
            xcolumns = o->xcolumns;
            xmatrix  = new Object ** [xrows];
            for( i = 0; i < xrows; ++i ){
                xmatrix[i] = new Object * [xcolumns];
            }
            for( i = 0; i < xrows; ++i ){
                for( j = 0; j < xcolumns; ++j ){
                    xmatrix[i][j] = new Object( o->xmatrix[i][j] );
                }
             }
		break;
	}

	return *this;
}

void Object::release( bool reset_attributes /*= true*/ ){
    unsigned int i, j;
    Object *o;

    switch( xtype ){
        case H_OT_STRING :
            xstring.clear();
        break;

        case H_OT_ARRAY  :
            for( i = 0; i < xsize; ++i ){
                o = xarray[i];
                if( o ){
                    delete o;
                    xarray[i] = NULL;
                }
            }
            xarray.clear();
        break;

        case H_OT_MAP    :
            for( i = 0; i < xsize; ++i ){
                o = xmap[i];
                if( o ){
                    delete o;
                    xmap[i] = NULL;
                }
                o = xarray[i];
                if( o ){
                    delete o;
                    xarray[i] = NULL;
                }
            }
            xmap.clear();
            xarray.clear();
        break;

        case H_OT_MATRIX :
            for( i = 0; i < xrows; ++i ){
                for( j = 0; j < xcolumns; ++j ){
                    o = xmatrix[i][j];
                    delete o;
                    xmatrix[i][j] = NULL;
                }
                delete [] xmatrix[i];
            }
            delete [] xmatrix;
            xrows    = 0;
            xcolumns = 0;
        break;
    }
    xsize      = 0;
    xtype      = H_OT_VOID;
    if( reset_attributes ){
        attributes = H_OA_NONE | H_OA_GARBAGE;
    }
}

Object::~Object(){
    #ifdef MEM_DEBUG
    printf( "[MEM DEBUG] deleted object (- %d bytes)\n", xsize );
    #endif
    release();
}

int Object::equals( Object *o ){
	if( xtype != o->xtype ){
		return 0;
	}
	else if( xsize != o->xsize ){
		return 0;
	}
	unsigned int i, j;
	switch( xtype ){
		case H_OT_INT    : return xint    == o->xint;
		case H_OT_ALIAS  : return xalias  == o->xalias;
		case H_OT_FLOAT  : return xfloat  == o->xfloat;
		case H_OT_CHAR   : return xchar   == o->xchar;
		case H_OT_STRING : return xstring == o->xstring;
		case H_OT_ARRAY  :
			for( i = 0; i < xsize; ++i ){
				if( xarray[i]->equals( o->xarray[i] ) == 0 ){
					return 0;
				}
			}
			return 1;
		break;
		case H_OT_MAP    :
			for( i = 0; i < xsize; ++i ){
				if( xmap[i]->equals( o->xmap[i] ) == 0 ){
					return 0;
				}
				if( xarray[i]->equals( o->xarray[i] ) == 0 ){
					return 0;
				}
			}
			return 1;
		break;
		case H_OT_MATRIX :
            if( xrows != o->xrows || xcolumns != o->xcolumns ){
                return 0;
            }
            for( i = 0; i < xrows; ++i ){
                for( j = 0; j < xcolumns; ++j ){
                    if( xmatrix[i][j]->equals( o->xmatrix[i][j] ) == 0 ){
                        return 0;
                    }
                }
            }
            return 1;
		break;
	}
}

int Object::mapFind( Object *map ){
	unsigned int i;
	for( i = 0; i < xsize; ++i ){
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
			for( i = 0; i < xsize; ++i ){
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
			for( i = 0; i < xsize; ++i ){
				memcpy( (buffer + offset), xarray[i]->serialize(), xarray[i]->xsize );
				offset += xarray[i]->xsize;
			}

		break;

		case H_OT_ARRAY  : hybris_generic_error( "could not serialize an array" ); break;
		case H_OT_MATRIX : hybris_generic_error( "could not serialize a matrix" ); break;
	}
	return buffer;
}

void Object::print( unsigned int tabs /*= 0*/ ){
	unsigned int i, j;
	for( i = 0; i < tabs; ++i ) printf( "\t" );
	switch(xtype){
		case H_OT_INT    : printf( "%d",  xint );           break;
		case H_OT_ALIAS  : printf( "0x%X", xalias );        break;
		case H_OT_FLOAT  : printf( "%lf", xfloat );         break;
		case H_OT_CHAR   : printf( "%c", xchar );           break;
		case H_OT_STRING : printf( "%s", xstring.c_str() ); break;
		case H_OT_ARRAY  :
			printf( "array {\n" );
			for( i = 0; i < xsize; ++i ){
				xarray[i]->println(tabs + 1);
			}
			for( i = 0; i < tabs; ++i ) printf( "\t" );
			printf( "}\n" );
			return;
		break;
		case H_OT_MAP  :
			printf( "map {\n" );
			for( i = 0; i < xsize; ++i ){
				xmap[i]->print(tabs + 1);
				printf( " -> " );
				xarray[i]->println(tabs + 1);
			}
			for( i = 0; i < tabs; ++i ) printf( "\t" );
			printf( "}\n" );
			return;
		break;
		case H_OT_MATRIX :
            printf( "matrix [%dx%d] {\n", xrows, xcolumns );
            for( i = 0; i < xrows; ++i ){
                for( j = 0; j < xcolumns; ++j ){
                    xmatrix[i][j]->print( tabs + 1 );
                }
                printf( "\n" );
            }
            printf( "}\n" );
		break;
	}
}

void Object::println( unsigned int tabs /*= 0*/ ){
	print(tabs);
	printf("\n");
}

#ifdef XML_SUPPORT
string Object::toxml( unsigned int tabs /*= 0*/ ){
	unsigned int i;
	string       xtabs;
	stringstream xml;

	for( i = 0; i < tabs; ++i ){ xtabs += "\t"; }
	switch(xtype){
		case H_OT_INT    : xml << xtabs << "<int>"    << xint    << "</int>\n";    break;
		case H_OT_ALIAS  : xml << xtabs << "<alias>"  << xalias  << "</alias>\n";  break;
		case H_OT_FLOAT  : xml << xtabs << "<float>"  << xfloat  << "</float>\n";  break;
		case H_OT_CHAR   : xml << xtabs << "<char>"   << xchar   << "</char>\n";   break;
		case H_OT_STRING : xml << xtabs << "<string>" << xstring << "</string>\n"; break;
		case H_OT_ARRAY  :
			xml << xtabs << "<array>\n";
			for( i = 0; i < xsize; ++i ){
				xml << xarray[i]->toxml( tabs + 1 );
			}
			xml << xtabs << "</array>\n";
		break;
		case H_OT_MAP  :
			xml << xtabs << "<map>\n";
			for( i = 0; i < xsize; ++i ){
				xml << xmap[i]->toxml( tabs + 1 );
				xml << xarray[i]->toxml( tabs + 1 );
			}
			xml << xtabs << "</map>\n";
		break;
	}

	return xml.str().c_str();
}
#endif

#ifdef PCRE_SUPPORT
int Object::classify_pcre( string& r ){
    pcrecpp::RE_Options  OPTS( PCRE_CASELESS | PCRE_EXTENDED );
	pcrecpp::RE          MULTI_REGEX( ".*[^" +
									  pcrecpp::RE::QuoteMeta("\\") + "]" +
									  pcrecpp::RE::QuoteMeta("(") + "[^" +
									  pcrecpp::RE::QuoteMeta(")") + "]+[^" +
									  pcrecpp::RE::QuoteMeta("\\") + "]" +
									  pcrecpp::RE::QuoteMeta(")") + ".*", OPTS );

	if( MULTI_REGEX.PartialMatch(r.c_str()) ){
		return H_PCRE_MULTI_MATCH;
	}
	else if(0){
		return H_PCRE_REPLACE_MATCH;
	}
	else{
		return H_PCRE_BOOL_MATCH;
	}
}

void Object::parse_pcre( string& raw, string& regex, int& opts ){
    unsigned int i;
	vector<string> blocks;
	pcrecpp::RE_Options  OPTS( PCRE_CASELESS | PCRE_EXTENDED );
	pcrecpp::RE          REGEX( "^/(.*?)/([i|m|s|x|U]*)$", OPTS );
	pcrecpp::StringPiece SUBJECT( raw );
	string   rex, sopts;

    while( REGEX.FindAndConsume( &SUBJECT, &rex, &sopts ) == true ){
		blocks.push_back( rex );
		blocks.push_back( sopts );
	}

	if( blocks.size() ){
		if( blocks.size() != 2 ){
			hybris_syntax_error( "invalid pcre regular expression syntax" );
		}
		opts  = 0;
		regex = blocks[0];
		sopts = blocks[1];

		/* parse options */
		for( i = 0; i < sopts.size(); i++ ){
			switch( sopts[i] ){
				case 'i' : opts |= PCRE_CASELESS;  break;
				case 'm' : opts |= PCRE_MULTILINE; break;
				case 's' : opts |= PCRE_DOTALL;    break;
				case 'x' : opts |= PCRE_EXTENDED;  break;
				case 'U' : opts |= PCRE_UNGREEDY;  break;
			}
		}
	}
	else{
		opts  = 0;
		regex = raw;
	}
}

Object *Object::regexp( Object *regexp ){
    if( assert_type( this, regexp, 1, H_OT_STRING ) == 0 ){
		hybris_syntax_error( "invalid types for '~=' regexp operator (%s, %s)", Object::type(this), Object::type(regexp) );
	}

	string rawreg  = regexp->xstring,
		   subject = this->xstring,
		   regex;
	int    opts;

	Object::parse_pcre( rawreg, regex, opts );

	if( Object::classify_pcre( regex ) == H_PCRE_BOOL_MATCH ){
		pcrecpp::RE_Options OPTS(opts);
		pcrecpp::RE         REGEX( regex.c_str(), OPTS );

		return new Object( (long)REGEX.PartialMatch(subject.c_str()) );
	}
	else{
		pcrecpp::RE_Options  OPTS(opts);
		pcrecpp::RE          REGEX( regex.c_str(), OPTS );
		pcrecpp::StringPiece SUBJECT( subject.c_str() );
		string  match;
		Object *matches = new Object();
		int i = 0;

		while( REGEX.FindAndConsume( &SUBJECT, &match ) == true ){
			if( i++ > H_PCRE_MAX_MATCHES ){
				hybris_generic_error( "something of your regex is forcing infinite matches" );
			}
			matches->push( new Object((char *)match.c_str()) );
		}

		return matches;
	}
}
#endif

void Object::input(){
	switch(xtype){
		case H_OT_INT    : cin >> xint;    break;
		case H_OT_FLOAT  : cin >> xfloat;  break;
		case H_OT_CHAR   : cin >> xchar;   break;
		case H_OT_STRING : cin >> xstring; break;
		case H_OT_ARRAY  : hybris_syntax_error( "could not read an array" ); break;
		case H_OT_MAP    : hybris_syntax_error( "could not read a map" ); break;
		case H_OT_ALIAS  : hybris_syntax_error( "could not read an alias" ); break;
		case H_OT_MATRIX : hybris_syntax_error( "could not read a matrix" ); break;
	}
}

long Object::lvalue(){
	switch(xtype){
		case H_OT_INT    : return xint;        break;
		case H_OT_ALIAS  : return xalias;      break;
		case H_OT_FLOAT  : return static_cast<long>(xfloat); break;
		case H_OT_CHAR   : return static_cast<long>(xchar);  break;
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    :
		case H_OT_MATRIX : return static_cast<long>(xsize);  break;
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
		case H_OT_MAP    :
        case H_OT_MATRIX : hybris_syntax_error( "'%s' is an invalid type for '.' operator", type(this) ); break;
	}
	switch(o->xtype){
		case H_OT_INT    : ret << o->xint;    break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", o->xalias ); ret << tmp; break;
		case H_OT_FLOAT  : ret << o->xfloat;  break;
		case H_OT_CHAR   : ret << o->xchar;   break;
		case H_OT_STRING : ret << o->xstring; break;
		case H_OT_ARRAY  :
		case H_OT_MAP    :
		case H_OT_MATRIX : hybris_syntax_error( "'%s' is an invalid type for '.' operator", type(o) ); break;
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
		case H_OT_MAP    :
		case H_OT_MATRIX : hybris_syntax_error( "'%s' is an invalid type for '.=' operator", type(this) ); break;
	}
	switch(o->xtype){
		case H_OT_INT    : ret << o->xint;    break;
		case H_OT_FLOAT  : ret << o->xfloat;  break;
		case H_OT_CHAR   : ret << o->xchar;   break;
		case H_OT_STRING : ret << o->xstring; break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", xalias ); ret << tmp; break;
		case H_OT_ARRAY  :
		case H_OT_MAP    :
		case H_OT_MATRIX : hybris_syntax_error( "'%s' is an invalid type for '.=' operator", type(o) ); break;
	}

	release(false);

	xtype   = H_OT_STRING;
	xstring = ret.str();
	parse_string( xstring );
	xsize   = strlen( xstring.c_str() ) + 1;

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
		case H_OT_MAP    :
		case H_OT_MATRIX : hybris_generic_error( "could not convert '%s' to string", type(this) ); break;
	}
	return new Object( (char *)ret.str().c_str() );
}

Object * Object::toInt(){
    long ret;
    switch(xtype){
		case H_OT_INT    : ret = xint;                  break;
		case H_OT_FLOAT  : ret = (long)xfloat;          break;
		case H_OT_CHAR   : ret = (long)xchar;           break;
		case H_OT_STRING : ret = atol(xstring.c_str()); break;
		case H_OT_ALIAS  :
		case H_OT_ARRAY  :
		case H_OT_MAP    :
		case H_OT_MATRIX : hybris_generic_error( "could not convert '%s' to int", type(this) ); break;
	}
	return new Object( ret );
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
		case H_OT_MATRIX : ret << "<matrix>"; break;
	}
	return ret.str();
}

Object *Object::push( Object *o ){
    Object *item = new Object(o);
    xarray.push_back( item );
	xtype = H_OT_ARRAY;
	xsize = xarray.size();
	return this;
}

Object *Object::push_ref( Object *o ){
	xarray.push_back( o );
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
	#ifdef BOUNDS_CHECK
	else if( xarray.size() <= 0 ){
	    hybris_generic_error( "could not pop an element from an empty array" );
	}
	#endif

	Object *element = xarray[ xarray.size() - 1 ];
	xarray.pop_back();
	xsize = xarray.size();
	return element;
}

Object *Object::mapPop(){
	if( xtype != H_OT_MAP ){
		hybris_generic_error( "could not pop an element from a non map type" );
	}
	#ifdef BOUNDS_CHECK
	else if( xarray.size() <= 0 || xmap.size() <= 0 ){
	    hybris_generic_error( "could not pop an element from an empty map" );
	}
	#endif

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
	#ifdef BOUNDS_CHECK
	else if( index->lvalue() >= xarray.size() ){
	    hybris_generic_error( "index out of bounds" );
	}
	#endif

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
        #ifdef BOUNDS_CHECK
        if( index->lvalue() >= xstring.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		return new Object( xstring[ index->lvalue() ] );
	}
	else if( xtype == H_OT_ARRAY ){
        #ifdef BOUNDS_CHECK
        if( index->lvalue() >= xarray.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		return xarray[ index->lvalue() ];
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
	else if( xtype == H_OT_MATRIX ){
	    #ifdef BOUNDS_CHECK
        if( index->lvalue() >= xcolumns ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
        Object *array    = new Object();
        unsigned int x = index->lvalue(),
                     y;
        for( y = 0; y < xrows; ++y ){
            array->push_ref( xmatrix[x][y] );
        }
        return array;
	}
	else{
		hybris_syntax_error( "'%s' is an invalid type for subscript operator", type(this) );
	}
}

Object *Object::at( char *map ){
	Object tmp(map);
	int i = mapFind(&tmp);

	if( i != -1 ){
        return xarray[i];
	}
	else{
        hybris_generic_error( "no mapped values for label '%s'", map );
	}
}

Object& Object::at( Object *index, Object *set ){
	if( xtype == H_OT_STRING ){
	    #ifdef BOUNDS_CHECK
        if( index->lvalue() >= xstring.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		xstring[ index->lvalue() ] = (char)set->lvalue();
	}
	else if( xtype == H_OT_ARRAY ){
	    #ifdef BOUNDS_CHECK
        if( index->lvalue() >= xarray.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
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

Object* Object::range( Object *to ){
    if( assert_type( this, to, 2, H_OT_CHAR, H_OT_INT ) == 0 ){
		hybris_syntax_error( "invalid type for range operator" );
	}
	if( xtype != to->xtype ){
        hybris_syntax_error( "types must be the same for range operator" );
    }

    Object *range = new Object();
    long i;

    if( xtype == H_OT_CHAR ){
        if( xchar < to->xchar ){
            for( i = xchar; i <= to->xchar; ++i ){
                range->push( new Object( (char)i ) );
            }
        }
        else{
            for( i = xchar; i >= to->xchar; i-- ){
                range->push( new Object( (char)i ) );
            }
        }
    }
    else if( xtype == H_OT_INT ){
        if( xint < to->xint ){
            for( i = xint; i <= to->xint; ++i ){
                range->push( new Object( i ) );
            }
        }
        else{
            for( i = xint; i >= to->xint; i-- ){
                range->push( new Object( i ) );
            }
        }
    }

    return range;
}

Object& Object::operator = ( Object *o ){
	return assign(o);
}

Object * Object::operator - (){
	switch(xtype){
		case H_OT_INT    : return new Object( static_cast<long>(~xint + 1) );   break;
		case H_OT_FLOAT  : return new Object( static_cast<double>(~(int)xfloat + 1) ); break;
		case H_OT_CHAR   : return new Object( static_cast<char>(~xchar + 1) );  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for unary minus operator" );
	}
}

Object * Object::operator ! (){
	switch(xtype){
		case H_OT_INT    : return new Object( static_cast<long>(!xint) );   break;
		case H_OT_FLOAT  : return new Object( static_cast<double>(!xfloat) ); break;
		case H_OT_CHAR   : return new Object( static_cast<char>(!xchar) );  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for unary not operator" );
	}
}

Object * Object::factorial(){
    double db_num,
           db_fact = 1.0;
    long   ln_num,
           ln_fact = 1,
           i;

    switch(xtype){
		case H_OT_INT    :
		case H_OT_CHAR   :
             ln_num = lvalue();
             for( i = 1; i <= ln_num; ++i ){
                ln_fact *= i;
             }
             return new Object(ln_fact);
        break;

		case H_OT_FLOAT  :
             db_num = xfloat;
             for( i = 1; i <= db_num; ++i ){
                db_fact *= i;
             }
             return new Object(db_fact);
		break;

		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for factorial operator" );
	}
}

Object& Object::operator ++ (){
	switch(xtype){
		case H_OT_INT    : ++xint;   break;
		case H_OT_FLOAT  : ++xfloat; break;
		case H_OT_CHAR   : ++xchar;  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for increment operator" );
	}
	return *this;
}

Object& Object::operator -- (){
	switch(xtype){
		case H_OT_INT    : --xint;   break;
		case H_OT_FLOAT  : --xfloat; break;
		case H_OT_CHAR   : --xchar;  break;
		case H_OT_ALIAS  :
		case H_OT_STRING :
		case H_OT_ARRAY  :
		case H_OT_MAP    : hybris_syntax_error( "unexpected type for decrement operator" );
	}
	return *this;
}

Object * Object::operator + ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for addition operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( xfloat + (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( o->xfloat + (xtype == H_OT_FLOAT ? xfloat : lvalue()) );
	}
	else if( xtype == H_OT_MATRIX ){
	    Object *matrix = new Object( this );
	    unsigned int x, y;

        if( o->xtype == H_OT_MATRIX ){
            if( xrows != o->xrows || xcolumns != o->xcolumns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*matrix->xmatrix[x][y]) += o->xmatrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*matrix->xmatrix[x][y]) += o;
                }
            }
        }
        return matrix;
	}
	else{
		return new Object( static_cast<long>(lvalue() + o->lvalue()) );
	}
}

Object * Object::operator += ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for addition operator" );
	}

	if( xtype == H_OT_FLOAT ){
		xfloat += (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_CHAR ){
		xchar += (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_MATRIX ){
	    unsigned int x, y;

        if( o->xtype == H_OT_MATRIX ){
            if( xrows != o->xrows || xcolumns != o->xcolumns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*this->xmatrix[x][y]) += o->xmatrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*this->xmatrix[x][y]) += o;
                }
            }
        }
	}
	else{
		xint += (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}

	return this;
}

Object * Object::operator - ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for subtraction operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( xfloat - (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( o->xfloat - (xtype == H_OT_FLOAT ? xfloat : lvalue()) );
	}
    else if( xtype == H_OT_MATRIX ){
	    Object *matrix = new Object( this );
	    unsigned int x, y;

        if( o->xtype == H_OT_MATRIX ){
            if( xrows != o->xrows || xcolumns != o->xcolumns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*matrix->xmatrix[x][y]) -= o->xmatrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*matrix->xmatrix[x][y]) -= o;
                }
            }
        }
        return matrix;
	}
	else{
		return new Object( static_cast<long>(lvalue() - o->lvalue()) );
	}
}

Object * Object::operator -= ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for subtraction operator" );
	}

	if( xtype == H_OT_FLOAT ){
		xfloat -= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_CHAR ){
		xchar -= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_MATRIX ){
	    unsigned int x, y;

        if( o->xtype == H_OT_MATRIX ){
            if( xrows != o->xrows || xcolumns != o->xcolumns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*this->xmatrix[x][y]) -= o->xmatrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*this->xmatrix[x][y]) -= o;
                }
            }
        }
	}
	else{
		xint -= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	return this;
}

Object * Object::operator * ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for multiplication operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( xfloat * (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( o->xfloat * (xtype == H_OT_FLOAT ? xfloat : lvalue()) );
	}
	else if( xtype == H_OT_MATRIX ){
	    unsigned int x, y, z;
	    Object *matrix;

	    if( o->xtype == H_OT_MATRIX ){
            if( xcolumns != o->xrows ){
                hybris_syntax_error( "first matrix columns have to be the same size of second matrix rows" );
            }
            vector<Object *> dummy;
            matrix = new Object( this->xcolumns, o->xrows, dummy );
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < o->xcolumns; ++y ){
                    for( z = 0; z < xcolumns; z++ ){
                        (*matrix->xmatrix[x][y]) = (*xmatrix[x][z]) * o->xmatrix[z][y];
                    }
                }
            }
        }
        else{
            matrix = new Object( this );
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*matrix->xmatrix[x][y]) *= o;
                }
            }
        }
        return matrix;
	}
	else{
		return new Object( static_cast<long>(lvalue() * o->lvalue()) );
	}
}

Object * Object::operator *= ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for multiplication operator" );
	}

	if( xtype == H_OT_FLOAT ){
		xfloat *= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_CHAR ){
		xchar *= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}
	else if( xtype == H_OT_MATRIX ){
	    unsigned int x, y, z;
	    Object *matrix;

	    if( o->xtype == H_OT_MATRIX ){
            if( xcolumns != o->xrows ){
                hybris_syntax_error( "first matrix columns have to be the same size of second matrix rows" );
            }
            vector<Object *> dummy;
            matrix = new Object( this->xcolumns, o->xrows, dummy );
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < o->xcolumns; ++y ){
                    for( z = 0; z < xcolumns; z++ ){
                        (*matrix->xmatrix[x][y]) = (*xmatrix[x][z]) * o->xmatrix[z][y];
                    }
                }
            }
        }
        else{
            matrix = new Object( this );
            for( x = 0; x < xrows; ++x ){
                for( y = 0; y < xcolumns; ++y ){
                    (*matrix->xmatrix[x][y]) *= o;
                }
            }
        }
        release(false);
        (*this) = matrix;
	}
	else{
		xint *= (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue());
	}

	return this;
}

Object * Object::operator / ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for division operator" );
	}

	if( xtype == H_OT_FLOAT ){
		return new Object( static_cast<double>(xfloat / (o->xtype == H_OT_FLOAT ? o->xfloat : o->lvalue()) ) );
	}
	else if( o->xtype == H_OT_FLOAT ){
		return new Object( static_cast<double>( o->xfloat / (xtype == H_OT_FLOAT ? xfloat : lvalue()) ) );
	}
	else{
		return new Object( static_cast<long>(lvalue() / o->lvalue()) );
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

	long a = lvalue(), b = o->lvalue(), mod;
	/* b is 0 or 1 */
    if( b == 0 || b == 1 ){
        mod = 0;
    }
    /* b is a power of 2 */
    else if( (b & (b - 1)) == 0 ){
        mod = a & (b - 1);
    }
    else{
        mod = a % b;
    }

    return new Object( mod );
}

Object * Object::operator %= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for modulus operator" );
	}

    long a = lvalue(), b = o->lvalue(), mod;
    /* b is 0 or 1 */
    if( b == 0 || b == 1 ){
        mod = 0;
    }
    /* b is a power of 2 */
    else if( (b & (b - 1)) == 0 ){
        mod = a & (b - 1);
    }
    else{
        mod = a % b;
    }

    if( xtype == H_OT_INT ){
        xint = mod;
    }
    else{
        xchar = (char)mod;
    }
}

Object * Object::operator & ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for and operator" );
	}

	return new Object( static_cast<long>(lvalue() & o->lvalue()) );
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

	return new Object( static_cast<long>(lvalue() | o->lvalue()) );
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

	return new Object( static_cast<long>(lvalue() << o->lvalue()) );
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

	return new Object( static_cast<long>(lvalue() >> o->lvalue()) );
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

	return new Object( static_cast<long>(lvalue() ^ o->lvalue()) );
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

	return new Object( static_cast<long>(~lvalue()) );
}

Object * Object::lnot (){
	if( assert_type( this, NULL, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for logical not operator" );
	}

	return new Object( static_cast<long>(!lvalue()) );
}

Object * Object::operator == ( Object *o ){
	if( xtype == H_OT_STRING && o->xtype == H_OT_STRING ){
		return new Object( static_cast<long>(xstring == o->xstring) );
	}
	else if( xtype == H_OT_ARRAY && o->xtype == H_OT_ARRAY ){
		return new Object( static_cast<long>( this->equals(o) ) );
	}
	else if( xtype == H_OT_MAP && o->xtype == H_OT_MAP ){
		return new Object( static_cast<long>( this->equals(o) ) );
	}
	else if( xtype == H_OT_MATRIX && o->xtype == H_OT_MATRIX ){
        return new Object( static_cast<long>( this->equals(o) ) );
	}
	else{
		return new Object( static_cast<long>(lvalue() == o->lvalue()) );
	}
}

Object * Object::operator != ( Object *o ){
	if( xtype == H_OT_STRING && o->xtype == H_OT_STRING ){
		return new Object( static_cast<long>(xstring != o->xstring) );
	}
	else if( xtype == H_OT_ARRAY && o->xtype == H_OT_ARRAY ){
		return new Object( static_cast<long>( !this->equals(o) ) );
	}
	else if( xtype == H_OT_MAP && o->xtype == H_OT_MAP ){
		return new Object( static_cast<long>( !this->equals(o) ) );
	}
	else if( xtype == H_OT_MATRIX && o->xtype == H_OT_MATRIX ){
        return new Object( static_cast<long>( !this->equals(o) ) );
	}
	else{
		return new Object( static_cast<long>(lvalue() != o->lvalue()) );
	}
}

Object * Object::operator < ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '<' comparision operator (%s, %s)", Object::type(this), Object::type(o) );
	}

	return new Object( static_cast<long>(lvalue() < o->lvalue()) );
}

Object * Object::operator > ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '>' comparision operator (%s, %s)", Object::type(this), Object::type(o) );
	}

	return new Object( static_cast<long>(lvalue() > o->lvalue()) );
}

Object * Object::operator <= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '<=' comparision operator (%s, %s)", Object::type(this), Object::type(o) );
	}

	return new Object( static_cast<long>(lvalue() <= o->lvalue()) );
}

Object * Object::operator >= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '>=' comparision operator (%s, %s)", Object::type(this), Object::type(o) );
	}

	return new Object( static_cast<long>(lvalue() >= o->lvalue()) );
}

Object * Object::operator || ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for '||' logical operator" );
	}

	return new Object( static_cast<long>(lvalue() || o->lvalue()) );
}

Object * Object::operator && ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for '&&' logical operator" );
	}

	return new Object( static_cast<long>(lvalue() && o->lvalue()) );
}
