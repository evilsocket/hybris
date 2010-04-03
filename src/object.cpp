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

const char *Object::type_name( Object *o ){
	switch(o->type){
		case H_OT_INT    : return "int";    break;
		case H_OT_CHAR   : return "char";   break;
		case H_OT_FLOAT  : return "float";  break;
		case H_OT_STRING : return "string"; break;
		case H_OT_BINARY : return "binary"; break;
		case H_OT_ARRAY  : return "array";  break;
		case H_OT_MAP    : return "map";    break;
		case H_OT_ALIAS  : return "alias";  break;
		case H_OT_MATRIX : return "matrix"; break;
		case H_OT_STRUCT : return "struct"; break;
	}
}

unsigned int Object::assert_type( Object *a, Object *b, unsigned int ntypes, ... ){
	unsigned int o, i, valid = 0;
	Object *objects[] = { a, b };
	va_list ap;

	for( o = 0; o < 2; ++o ){
		if( objects[o] != NULL ){
			valid = 0;
			va_start( ap, ntypes );
			for( i = 0; i < ntypes && valid == 0; ++i ){
				if( objects[o]->type == va_arg( ap, int ) ){
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
    type(H_OT_INT),
    size(sizeof(long)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    this->value.m_integer = value;
}

Object::Object( long value, unsigned int _is_extern ) :
    type(H_OT_INT),
    size(sizeof(long)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    this->value.m_integer = value;
	if( _is_extern ){
        attributes |= H_OA_EXTERN;
	}
}

Object::Object( double value ) :
    type(H_OT_FLOAT),
    size(sizeof(double)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    this->value.m_double = value;
}

Object::Object( char value ) :
    type(H_OT_CHAR),
    size(sizeof(char)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    this->value.m_char = value;
}

Object::Object( char *value ) :
    type(H_OT_STRING),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    this->value.m_string = value;
	parse_string( this->value.m_string );
	size   = this->value.m_string.size() + 1;
}

Object::Object( vector<unsigned char>& data ) :
    type(H_OT_BINARY),
    size(data.size()),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    vector<unsigned char>::iterator i;

    for( i = data.begin(); i != data.end(); i++ ){
        value.m_array.push_back( new Object( (char)(*i) ) );
    }
}

Object::Object() :
    type(H_OT_ARRAY),
    size(0),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{

}

Object::Object( unsigned int value ) :
    type(H_OT_ALIAS),
    size(sizeof(unsigned int)),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    this->value.m_alias = value;
}

Object::Object( unsigned int rows, unsigned int columns, vector<Object *>& data ) :
    type(H_OT_MATRIX),
    size(0),
    attributes(H_OA_NONE | H_OA_GARBAGE)
{
    unsigned int x, y;

    this->value.m_rows    = rows;
    this->value.m_columns = columns;
    this->value.m_matrix  = new Object ** [rows];

    for( x = 0; x < rows; ++x ){
        value.m_matrix[x] = new Object * [columns];
    }

    if( data.size() ){
        for( x = 0; x < rows; ++x ){
            for( y = 0; y < columns; ++y ){
                Object * o    = data[ x * columns + y ];
                value.m_matrix[x][y] = o;
                size        += o->size;
            }
        }
    }
    else{
        for( x = 0; x < rows; ++x ){
            for( y = 0; y < columns; ++y ){
                Object * o    = new Object( static_cast<long>(0) );
                value.m_matrix[x][y] = o;
                size        += o->size;
            }
        }
    }
}

Object::Object( Object *o ) :
    size(o->size),
    type(o->type),
    attributes(o->attributes)
{
	unsigned int i, j, attrs( o->value.m_struct_names.size() ), vals( o->value.m_struct_values.size() );

	switch( type ){
		case H_OT_INT    : value.m_integer    = o->value.m_integer;    break;
		case H_OT_ALIAS  : value.m_alias  = o->value.m_alias;  break;
		case H_OT_FLOAT  : value.m_double  = o->value.m_double;  break;
		case H_OT_CHAR   : value.m_char   = o->value.m_char;   break;
		case H_OT_STRING : value.m_string = o->value.m_string; break;
		case H_OT_BINARY :
            for( i = 0; i < size; ++i ){
				value.m_array.push_back( new Object( o->value.m_array[i] ) );
			}
		break;
		case H_OT_ARRAY  :
			for( i = 0; i < size; ++i ){
				value.m_array.push_back( new Object( o->value.m_array[i] ) );
			}
		break;
		case H_OT_MAP    :
			for( i = 0; i < size; ++i ){
				value.m_map.push_back( new Object( o->value.m_map[i] ) );
				value.m_array.push_back( new Object( o->value.m_array[i] ) );
			}
		break;
		case H_OT_MATRIX :
            value.m_rows    = o->value.m_rows;
            value.m_columns = o->value.m_columns;
            value.m_matrix  = new Object ** [value.m_rows];
            for( i = 0; i < value.m_rows; ++i ){
                value.m_matrix[i] = new Object * [value.m_columns];
            }
            for( i = 0; i < value.m_rows; ++i ){
                for( j = 0; j < value.m_columns; ++j ){
                    value.m_matrix[i][j] = new Object( o->value.m_matrix[i][j] );
                }
             }
		break;

		case H_OT_STRUCT :
            for( i = 0; i < attrs; ++i ){
                setAttribute( (char *)o->value.m_struct_names[i].c_str(), o->value.m_struct_values[i] );
            }
		break;
	}
	attributes = o->attributes;
}

void Object::setGarbageAttribute( H_OBJECT_ATTRIBUTE mask ){
    unsigned int i, j, vals( value.m_struct_values.size() );

    /* not garbage */
    if( mask == ~H_OA_GARBAGE ){
        attributes &= mask;
    }
    /* garbage */
    else{
        attributes |= mask;
    }
    /* eventually handle children */
    switch( type ){
        case H_OT_BINARY :
            for( i = 0; i < size; ++i ){
                value.m_array[i]->setGarbageAttribute(mask);
            }
        break;

        case H_OT_ARRAY  :
            for( i = 0; i < size; ++i ){
                value.m_array[i]->setGarbageAttribute(mask);
            }
        break;

        case H_OT_MAP    :
            for( i = 0; i < size; ++i ){
                value.m_array[i]->setGarbageAttribute(mask);
                value.m_map[i]->setGarbageAttribute(mask);
            }
        break;

        case H_OT_MATRIX :
            for( i = 0; i < value.m_rows; ++i ){
                for( j = 0; j < value.m_columns; ++j ){
                    value.m_matrix[i][j]->setGarbageAttribute(mask);
                }
            }
        break;

        case H_OT_STRUCT :
            for( i = 0; i < vals; ++i ){
                value.m_struct_values[i]->setGarbageAttribute(mask);
            }
		break;
    }
}

Object& Object::assign( Object *o ){
    unsigned int i, j,  attrs( o->value.m_struct_names.size() ), vals( o->value.m_struct_values.size() );

	release(false);

    type = o->type;
    size = o->size;
	switch( o->type ){
		case H_OT_INT    : value.m_integer    = o->value.m_integer;    break;
		case H_OT_ALIAS  : value.m_alias  = o->value.m_alias;  break;
		case H_OT_FLOAT  : value.m_double  = o->value.m_double;  break;
		case H_OT_CHAR   : value.m_char   = o->value.m_char;   break;
		case H_OT_STRING : value.m_string = o->value.m_string; break;
		case H_OT_BINARY :
            for( i = 0; i < size; ++i ){
				value.m_array.push_back( new Object( o->value.m_array[i] ) );
			}
		break;
		case H_OT_ARRAY  :
			for( i = 0; i < size; ++i ){
				value.m_array.push_back( new Object( o->value.m_array[i] ) );
			}
		break;
		case H_OT_MAP    :
			for( i = 0; i < size; ++i ){
				value.m_map.push_back( new Object( o->value.m_map[i] ) );
				value.m_array.push_back( new Object( o->value.m_array[i] ) );
			}
		break;
		case H_OT_MATRIX :
            value.m_rows    = o->value.m_rows;
            value.m_columns = o->value.m_columns;
            value.m_matrix  = new Object ** [value.m_rows];
            for( i = 0; i < value.m_rows; ++i ){
                value.m_matrix[i] = new Object * [value.m_columns];
            }
            for( i = 0; i < value.m_rows; ++i ){
                for( j = 0; j < value.m_columns; ++j ){
                    value.m_matrix[i][j] = new Object( o->value.m_matrix[i][j] );
                }
             }
		break;
		case H_OT_STRUCT :
            for( i = 0; i < attrs; ++i ){
                setAttribute( (char *)o->value.m_struct_names[i].c_str(), o->value.m_struct_values[i] );
            }
		break;
	}

	return *this;
}

void Object::release( bool reset_attributes /*= true*/ ){
    unsigned int i, j;
    Object *o;

    switch( type ){
        case H_OT_STRING :
            value.m_string.clear();
        break;

        case H_OT_BINARY :
            for( i = 0; i < size; ++i ){
                o = value.m_array[i];
                if( o ){
                    delete o;
                    value.m_array[i] = NULL;
                }
            }
            value.m_array.clear();
        break;

        case H_OT_ARRAY  :
            for( i = 0; i < size; ++i ){
                o = value.m_array[i];
                if( o ){
                    delete o;
                    value.m_array[i] = NULL;
                }
            }
            value.m_array.clear();
        break;

        case H_OT_MAP    :
            for( i = 0; i < size; ++i ){
                o = value.m_map[i];
                if( o ){
                    delete o;
                    value.m_map[i] = NULL;
                }
                o = value.m_array[i];
                if( o ){
                    delete o;
                    value.m_array[i] = NULL;
                }
            }
            value.m_map.clear();
            value.m_array.clear();
        break;

        case H_OT_MATRIX :
            for( i = 0; i < value.m_rows; ++i ){
                for( j = 0; j < value.m_columns; ++j ){
                    o = value.m_matrix[i][j];
                    delete o;
                    value.m_matrix[i][j] = NULL;
                }
                delete [] value.m_matrix[i];
            }
            delete [] value.m_matrix;
            value.m_rows    = 0;
            value.m_columns = 0;
        break;

        case H_OT_STRUCT :
            for( i = 0; i < value.m_struct_values.size(); ++i ){
                o = value.m_struct_values[i];
                delete o;
                value.m_struct_values[i] = NULL;
            }
            value.m_struct_values.clear();
            value.m_struct_names.clear();
        break;
    }
    size      = 0;
    type      = H_OT_VOID;
    if( reset_attributes ){
        attributes = H_OA_NONE | H_OA_GARBAGE;
    }
}

Object::~Object(){
    #ifdef MEM_DEBUG
    printf( "[MEM DEBUG] deleted object (- %d bytes)\n", size );
    #endif
    release();
}

int Object::equals( Object *o ){
	if( type != o->type ){
		return 0;
	}
	else if( size != o->size ){
		return 0;
	}
	unsigned int i, j;
	switch( type ){
		case H_OT_INT    : return value.m_integer    == o->value.m_integer;
		case H_OT_ALIAS  : return value.m_alias  == o->value.m_alias;
		case H_OT_FLOAT  : return value.m_double  == o->value.m_double;
		case H_OT_CHAR   : return value.m_char   == o->value.m_char;
		case H_OT_STRING : return value.m_string == o->value.m_string;
		case H_OT_BINARY :
            for( i = 0; i < size; ++i ){
				if( value.m_array[i]->equals( o->value.m_array[i] ) == 0 ){
					return 0;
				}
			}
			return 1;
		break;
		case H_OT_ARRAY  :
			for( i = 0; i < size; ++i ){
				if( value.m_array[i]->equals( o->value.m_array[i] ) == 0 ){
					return 0;
				}
			}
			return 1;
		break;
		case H_OT_MAP    :
			for( i = 0; i < size; ++i ){
				if( value.m_map[i]->equals( o->value.m_map[i] ) == 0 ){
					return 0;
				}
				if( value.m_array[i]->equals( o->value.m_array[i] ) == 0 ){
					return 0;
				}
			}
			return 1;
		break;
		case H_OT_MATRIX :
            if( value.m_rows != o->value.m_rows || value.m_columns != o->value.m_columns ){
                return 0;
            }
            for( i = 0; i < value.m_rows; ++i ){
                for( j = 0; j < value.m_columns; ++j ){
                    if( value.m_matrix[i][j]->equals( o->value.m_matrix[i][j] ) == 0 ){
                        return 0;
                    }
                }
            }
            return 1;
		break;
		case H_OT_STRUCT :
            if( value.m_struct_names.size()  != o->value.m_struct_names.size() ||
                value.m_struct_values.size() != o->value.m_struct_values.size() ){
                return 0;
            }

            for( i = 0; i < value.m_struct_names.size(); ++i ){
                if( value.m_struct_names[i]  != o->value.m_struct_names[i] ||
                    value.m_struct_values[i]->equals(o->value.m_struct_values[i]) == 0 ){
                    return 0;
                }
                return 1;
            }

		break;
	}
}

void Object::addAttribute( char *name ){
    type = H_OT_STRUCT;
    size++;

    value.m_struct_names.push_back( string(name) );
    value.m_struct_values.push_back( new Object((long)0) );
}

Object *Object::getAttribute( char *name ){
    int i, sz( value.m_struct_names.size() );

    for( i = 0; i < sz; ++i ){
        if( value.m_struct_names[i] == name ){
            return value.m_struct_values[i];
        }
    }
    return NULL;
}

void Object::setAttribute( char *name, Object *value ){
    int i, sz( this->value.m_struct_names.size() );

    for( i = 0; i < sz; ++i ){
        if( this->value.m_struct_names[i] == name ){
            this->value.m_struct_values[i]->assign(value);
            return;
        }
    }

    this->value.m_struct_names.push_back( string(name) );
    this->value.m_struct_values.push_back( new Object(value) );
}

int Object::mapFind( Object *map ){
	unsigned int i;
	for( i = 0; i < size; ++i ){
		if( value.m_map[i]->equals(map) ){
			return i;
		}
	}
	return -1;
}

Object * Object::getObject(){
	if( type != H_OT_ALIAS ){
		hybris_generic_error( "invalid pointer reference" );
	}
	return (Object *)value.m_alias;
}

void Object::print( unsigned int tabs /*= 0*/ ){
	unsigned int i, j, vals( value.m_struct_names.size() );
	for( i = 0; i < tabs; ++i ) printf( "\t" );
	switch(type){
		case H_OT_INT    : printf( "%d",  value.m_integer );           break;
		case H_OT_ALIAS  : printf( "0x%X", value.m_alias );        break;
		case H_OT_FLOAT  : printf( "%lf", value.m_double );         break;
		case H_OT_CHAR   : printf( "%c", value.m_char );           break;
		case H_OT_STRING : printf( "%s", value.m_string.c_str() ); break;
		case H_OT_BINARY :
            printf( "binary {\n\t" );
            for( i = 0; i < size; ++i ){
				printf( "%.2X", value.m_array[i]->value.m_char );
			}
			for( i = 0; i < tabs; ++i ) printf( "\t" );
			printf( "\n}\n" );
			return;
		break;
		case H_OT_ARRAY  :
			printf( "array {\n" );
			for( i = 0; i < size; ++i ){
				value.m_array[i]->println(tabs + 1);
			}
			for( i = 0; i < tabs; ++i ) printf( "\t" );
			printf( "}\n" );
			return;
		break;
		case H_OT_MAP  :
			printf( "map {\n" );
			for( i = 0; i < size; ++i ){
				value.m_map[i]->print(tabs + 1);
				printf( " -> " );
				value.m_array[i]->println(tabs + 1);
			}
			for( i = 0; i < tabs; ++i ) printf( "\t" );
			printf( "}\n" );
			return;
		break;
		case H_OT_MATRIX :
            printf( "matrix [%dx%d] {\n", value.m_rows, value.m_columns );
            for( i = 0; i < value.m_rows; ++i ){
                for( j = 0; j < value.m_columns; ++j ){
                    value.m_matrix[i][j]->print( tabs + 1 );
                }
                printf( "\n" );
            }
            printf( "}\n" );
		break;

		case H_OT_STRUCT :
            printf( "struct {\n" );
			for( i = 0; i < vals; ++i ){
			    for( j = 0; j <= tabs; ++j ) printf( "\t" );
			    printf( "%s : ", value.m_struct_names[i].c_str() );
                value.m_struct_values[i]->print( tabs + 1 );
                printf( "\n" );
			}
			for( i = 0; i < tabs; ++i ) printf( "\t" );
			printf( "}\n" );
		break;
	}
}

void Object::println( unsigned int tabs /*= 0*/ ){
	print(tabs);
	printf("\n");
}

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
		hybris_syntax_error( "invalid types for '~=' regexp operator (%s, %s)", type_name(this), type_name(regexp) );
	}

	string rawreg  = regexp->value.m_string,
		   subject = this->value.m_string,
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
	switch(type){
		case H_OT_INT    : cin >> value.m_integer; break;
		case H_OT_FLOAT  : cin >> value.m_double;  break;
		case H_OT_CHAR   : cin >> value.m_char;    break;
		case H_OT_STRING : cin >> value.m_string;  break;

		default :
             hybris_syntax_error( "could not read type '%s' from stdin", type_name(this) );
	}
}

long Object::lvalue(){
	switch(type){
		case H_OT_INT    : return value.m_integer;                   break;
		case H_OT_ALIAS  : return value.m_alias;                     break;
		case H_OT_FLOAT  : return static_cast<long>(value.m_double); break;
		case H_OT_CHAR   : return static_cast<long>(value.m_char);   break;

		default :
            return static_cast<long>(size);
	}
}

Object * Object::dot( Object *o ){
	stringstream ret;
	char         tmp[0xFF] = {0};

	switch(type){
		case H_OT_INT    : ret << value.m_integer;    break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", value.m_alias ); ret << tmp; break;
		case H_OT_FLOAT  : ret << value.m_double;  break;
		case H_OT_CHAR   : ret << value.m_char;   break;
		case H_OT_STRING : ret << value.m_string; break;

		default :
            hybris_syntax_error( "'%s' is an invalid type for '.' operator", type_name(this) );
	}
	switch(o->type){
		case H_OT_INT    : ret << o->value.m_integer;    break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", o->value.m_alias ); ret << tmp; break;
		case H_OT_FLOAT  : ret << o->value.m_double;  break;
		case H_OT_CHAR   : ret << o->value.m_char;   break;
		case H_OT_STRING : ret << o->value.m_string; break;

		default :
            hybris_syntax_error( "'%s' is an invalid type for '.' operator", type_name(o) );
	}

	return new Object( (char *)ret.str().c_str() );
}

Object * Object::dotequal( Object *o ){
	stringstream ret;
	char         tmp[0xFF] = {0};

	switch(type){
		case H_OT_INT    : ret << value.m_integer;    break;
		case H_OT_FLOAT  : ret << value.m_double;  break;
		case H_OT_CHAR   : ret << value.m_char;   break;
		case H_OT_STRING : ret << value.m_string; break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", value.m_alias ); ret << tmp; break;

		default:
            hybris_syntax_error( "'%s' is an invalid type for '.=' operator", type_name(this) );
	}
	switch(o->type){
		case H_OT_INT    : ret << o->value.m_integer;    break;
		case H_OT_FLOAT  : ret << o->value.m_double;  break;
		case H_OT_CHAR   : ret << o->value.m_char;   break;
		case H_OT_STRING : ret << o->value.m_string; break;
		case H_OT_ALIAS  : sprintf( tmp, "0x%X", value.m_alias ); ret << tmp; break;

		default :
            hybris_syntax_error( "'%s' is an invalid type for '.=' operator", type_name(o) );
	}

	release(false);

	type   = H_OT_STRING;
	value.m_string = ret.str();
	parse_string( value.m_string );
	size   = strlen( value.m_string.c_str() ) + 1;

	return this;
}

Object * Object::toString(){
	stringstream ret;
	switch(type){
		case H_OT_INT    : ret << value.m_integer; break;
		case H_OT_FLOAT  : ret << value.m_double;  break;
		case H_OT_CHAR   : ret << value.m_char;    break;
		case H_OT_STRING : ret << value.m_string;  break;
		case H_OT_BINARY :
            for( int i = 0; i < size; ++i ){
                ret << value.m_array[i]->value.m_char;
            }
        break;

		default :
            hybris_generic_error( "could not convert '%s' to string", type_name(this) );
	}
	return new Object( (char *)ret.str().c_str() );
}

Object * Object::toInt(){
    long ret;
    switch(type){
		case H_OT_INT    : ret = value.m_integer;              break;
		case H_OT_FLOAT  : ret = (long)value.m_double;         break;
		case H_OT_CHAR   : ret = (long)value.m_char;           break;
		case H_OT_STRING : ret = atol(value.m_string.c_str()); break;

		default :
            hybris_generic_error( "could not convert '%s' to int", type_name(this) );
	}
	return new Object( ret );
}

string Object::svalue(){
	stringstream ret;
	switch(type){
		case H_OT_INT    : ret << value.m_integer;      break;
		case H_OT_FLOAT  : ret << value.m_double;    break;
		case H_OT_CHAR   : ret << "'"  << value.m_char   << "'";  break;
		case H_OT_STRING : ret << "\"" << value.m_string << "\""; break;

		default :
            ret << "<" << type_name(this) << ">";
	}
	return ret.str();
}

Object *Object::push( Object *o ){
    Object *item = new Object(o);
    value.m_array.push_back( item );
	type = H_OT_ARRAY;
	size = value.m_array.size();
	return this;
}

Object *Object::push_ref( Object *o ){
	value.m_array.push_back( o );
	type = H_OT_ARRAY;
	size = value.m_array.size();
	return this;
}

Object *Object::map( Object *map, Object *o ){
	value.m_map.push_back( new Object(map) );
	value.m_array.push_back( new Object(o) );
	type = H_OT_MAP;
	size = value.m_map.size();
	return this;
}

Object *Object::pop(){
	if( type != H_OT_ARRAY ){
		hybris_generic_error( "could not pop an element from a non array type" );
	}
	#ifdef BOUNDS_CHECK
	else if( value.m_array.size() <= 0 ){
	    hybris_generic_error( "could not pop an element from an empty array" );
	}
	#endif

	Object *element = value.m_array[ value.m_array.size() - 1 ];
	value.m_array.pop_back();
	size = value.m_array.size();
	return element;
}

Object *Object::mapPop(){
	if( type != H_OT_MAP ){
		hybris_generic_error( "could not pop an element from a non map type" );
	}
	#ifdef BOUNDS_CHECK
	else if( value.m_array.size() <= 0 || value.m_map.size() <= 0 ){
	    hybris_generic_error( "could not pop an element from an empty map" );
	}
	#endif

	Object *element = value.m_array[ value.m_array.size() - 1 ];
	value.m_array.pop_back();
	value.m_map.pop_back();
	size = value.m_map.size();
	return element;
}

Object *Object::remove( Object *index ){
	if( type != H_OT_ARRAY ){
		hybris_generic_error( "could not remove an element from a non array type" );
	}
	#ifdef BOUNDS_CHECK
	else if( index->lvalue() >= value.m_array.size() ){
	    hybris_generic_error( "index out of bounds" );
	}
	#endif

	unsigned int i = index->lvalue();
	Object *element = value.m_array[i];
	value.m_array.erase( value.m_array.begin() + i );
	size = value.m_array.size();
	return element;
}

Object *Object::unmap( Object *map ){
	if( type != H_OT_MAP ){
		hybris_generic_error( "could not unmap an element from a non map type" );
	}
	int i = mapFind(map);
	if( i != -1 ){
		Object *element = value.m_array[i];
		value.m_map.erase( value.m_map.begin() + i );
		value.m_array.erase( value.m_array.begin() + i );
		size = value.m_map.size();
		return element;
	}
	return this;
}

Object *Object::at( Object *index ){
	if( type == H_OT_STRING ){
        #ifdef BOUNDS_CHECK
        if( index->lvalue() >= value.m_string.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		return new Object( value.m_string[ index->lvalue() ] );
	}
	else if( type == H_OT_ARRAY || type == H_OT_BINARY  ){
        #ifdef BOUNDS_CHECK
        if( index->lvalue() >= value.m_array.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		return value.m_array[ index->lvalue() ];
	}
	else if( type == H_OT_MAP ){
		int i = mapFind(index);
		if( i != -1 ){
			return new Object( value.m_array[i] );
		}
		else{
			hybris_generic_error( "no mapped values for label '%s'", index->toString() );
		}
	}
	else if( type == H_OT_MATRIX ){
	    #ifdef BOUNDS_CHECK
        if( index->lvalue() >= value.m_columns ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
        Object *array    = new Object();
        unsigned int x = index->lvalue(),
                     y;
        for( y = 0; y < value.m_rows; ++y ){
            array->push_ref( value.m_matrix[x][y] );
        }
        return array;
	}
	else{
		hybris_syntax_error( "'%s' is an invalid type for subscript operator", type_name(this) );
	}
}

Object *Object::at( char *map ){
	Object tmp(map);
	int i = mapFind(&tmp);

	if( i != -1 ){
        return value.m_array[i];
	}
	else{
        hybris_generic_error( "no mapped values for label '%s'", map );
	}
}

Object& Object::at( Object *index, Object *set ){
	if( type == H_OT_STRING ){
	    #ifdef BOUNDS_CHECK
        if( index->lvalue() >= value.m_string.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		value.m_string[ index->lvalue() ] = (char)set->lvalue();
	}
	else if( type == H_OT_BINARY ){
	    if( set->type != H_OT_CHAR && set->type != H_OT_INT && set->type != H_OT_FLOAT ){
	        hybris_syntax_error( "binary type allows only char, int or float types in its subscript operator" );
	    }
	    #ifdef BOUNDS_CHECK
        if( index->lvalue() >= value.m_array.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		delete value.m_array[ index->lvalue() ];
		Object *integer = set->toInt();
		value.m_array[ index->lvalue() ] = new Object((char)integer->value.m_integer);
		delete integer;
	}
	else if( type == H_OT_ARRAY ){
	    #ifdef BOUNDS_CHECK
        if( index->lvalue() >= value.m_array.size() ){
            hybris_generic_error( "index out of bounds" );
        }
        #endif
		delete value.m_array[ index->lvalue() ];
		value.m_array[ index->lvalue() ] = new Object(set);
	}
	else if( type == H_OT_MAP ){
		int i = mapFind(index);
		if( i != -1 ){
			delete value.m_array[i];
			value.m_array[i] = new Object(set);
		}
		else{
			map( index, set );
		}
	}
	else{
		hybris_syntax_error( "'%s' is an invalid type for subscript operator", type_name(this) );
	}

	return *this;
}

Object* Object::range( Object *to ){
    if( assert_type( this, to, 2, H_OT_CHAR, H_OT_INT ) == 0 ){
		hybris_syntax_error( "invalid type for range operator" );
	}
	if( type != to->type ){
        hybris_syntax_error( "types must be the same for range operator" );
    }

    Object *range = new Object();
    long i;

    if( type == H_OT_CHAR ){
        if( value.m_char < to->value.m_char ){
            for( i = value.m_char; i <= to->value.m_char; ++i ){
                range->push( new Object( (char)i ) );
            }
        }
        else{
            for( i = value.m_char; i >= to->value.m_char; i-- ){
                range->push( new Object( (char)i ) );
            }
        }
    }
    else if( type == H_OT_INT ){
        if( value.m_integer < to->value.m_integer ){
            for( i = value.m_integer; i <= to->value.m_integer; ++i ){
                range->push( new Object( i ) );
            }
        }
        else{
            for( i = value.m_integer; i >= to->value.m_integer; i-- ){
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
	switch(type){
		case H_OT_INT    : return new Object( static_cast<long>(~value.m_integer + 1) );   break;
		case H_OT_FLOAT  : return new Object( static_cast<double>(~(int)value.m_double + 1) ); break;
		case H_OT_CHAR   : return new Object( static_cast<char>(~value.m_char + 1) );  break;

		default :
            hybris_syntax_error( "unexpected %s type for unary minus operator", type_name(this) );
	}
}

Object * Object::operator ! (){
	switch(type){
		case H_OT_INT    : return new Object( static_cast<long>(!value.m_integer) );   break;
		case H_OT_FLOAT  : return new Object( static_cast<double>(!value.m_double) ); break;
		case H_OT_CHAR   : return new Object( static_cast<char>(!value.m_char) );  break;

		default :
            hybris_syntax_error( "unexpected %s type for unary not operator", type_name(this) );
	}
}

Object * Object::factorial(){
    double db_num,
           db_fact = 1.0;
    long   ln_num,
           ln_fact = 1,
           i;

    switch(type){
		case H_OT_INT    :
		case H_OT_CHAR   :
             ln_num = lvalue();
             for( i = 1; i <= ln_num; ++i ){
                ln_fact *= i;
             }
             return new Object(ln_fact);
        break;

		case H_OT_FLOAT  :
             db_num = value.m_double;
             for( i = 1; i <= db_num; ++i ){
                db_fact *= i;
             }
             return new Object(db_fact);
		break;

		default :
            hybris_syntax_error( "unexpected %s type for factorial operator", type_name(this) );
	}
}

Object& Object::operator ++ (){
	switch(type){
		case H_OT_INT    : ++value.m_integer;   break;
		case H_OT_FLOAT  : ++value.m_double; break;
		case H_OT_CHAR   : ++value.m_char;  break;

		default :
            hybris_syntax_error( "unexpected %s type for increment operator", type_name(this) );
	}
	return *this;
}

Object& Object::operator -- (){
	switch(type){
		case H_OT_INT    : --value.m_integer;   break;
		case H_OT_FLOAT  : --value.m_double; break;
		case H_OT_CHAR   : --value.m_char;  break;

		default :
            hybris_syntax_error( "unexpected %s type for decrement operator", type_name(this) );
	}
	return *this;
}

Object * Object::operator + ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for addition operator" );
	}

	if( type == H_OT_FLOAT ){
		return new Object( value.m_double + (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue()) );
	}
	else if( o->type == H_OT_FLOAT ){
		return new Object( o->value.m_double + (type == H_OT_FLOAT ? value.m_double : lvalue()) );
	}
	else if( type == H_OT_MATRIX ){
	    Object *matrix = new Object( this );
	    unsigned int x, y;

        if( o->type == H_OT_MATRIX ){
            if( value.m_rows != o->value.m_rows || value.m_columns != o->value.m_columns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*matrix->value.m_matrix[x][y]) += o->value.m_matrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*matrix->value.m_matrix[x][y]) += o;
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

	if( type == H_OT_FLOAT ){
		value.m_double += (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else if( type == H_OT_CHAR ){
		value.m_char += (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else if( type == H_OT_MATRIX ){
	    unsigned int x, y;

        if( o->type == H_OT_MATRIX ){
            if( value.m_rows != o->value.m_rows || value.m_columns != o->value.m_columns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*this->value.m_matrix[x][y]) += o->value.m_matrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*this->value.m_matrix[x][y]) += o;
                }
            }
        }
	}
	else{
		value.m_integer += (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}

	return this;
}

Object * Object::operator - ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for subtraction operator" );
	}

	if( type == H_OT_FLOAT ){
		return new Object( value.m_double - (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue()) );
	}
	else if( o->type == H_OT_FLOAT ){
		return new Object( o->value.m_double - (type == H_OT_FLOAT ? value.m_double : lvalue()) );
	}
    else if( type == H_OT_MATRIX ){
	    Object *matrix = new Object( this );
	    unsigned int x, y;

        if( o->type == H_OT_MATRIX ){
            if( value.m_rows != o->value.m_rows || value.m_columns != o->value.m_columns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*matrix->value.m_matrix[x][y]) -= o->value.m_matrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*matrix->value.m_matrix[x][y]) -= o;
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

	if( type == H_OT_FLOAT ){
		value.m_double -= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else if( type == H_OT_CHAR ){
		value.m_char -= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else if( type == H_OT_MATRIX ){
	    unsigned int x, y;

        if( o->type == H_OT_MATRIX ){
            if( value.m_rows != o->value.m_rows || value.m_columns != o->value.m_columns ){
                hybris_syntax_error( "matrices have to be the same size" );
            }
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*this->value.m_matrix[x][y]) -= o->value.m_matrix[x][y];
                }
            }
        }
        else{
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*this->value.m_matrix[x][y]) -= o;
                }
            }
        }
	}
	else{
		value.m_integer -= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	return this;
}

Object * Object::operator * ( Object *o ){
	if( assert_type( this, o, 4, H_OT_CHAR, H_OT_INT, H_OT_FLOAT, H_OT_MATRIX ) == 0 ){
		hybris_syntax_error( "invalid type for multiplication operator" );
	}

	if( type == H_OT_FLOAT ){
		return new Object( value.m_double * (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue()) );
	}
	else if( o->type == H_OT_FLOAT ){
		return new Object( o->value.m_double * (type == H_OT_FLOAT ? value.m_double : lvalue()) );
	}
	else if( type == H_OT_MATRIX ){
	    unsigned int x, y, z;
	    Object *matrix;

	    if( o->type == H_OT_MATRIX ){
            if( value.m_columns != o->value.m_rows ){
                hybris_syntax_error( "first matrix columns have to be the same size of second matrix rows" );
            }
            vector<Object *> dummy;
            matrix = new Object( this->value.m_columns, o->value.m_rows, dummy );
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < o->value.m_columns; ++y ){
                    for( z = 0; z < value.m_columns; z++ ){
                        (*matrix->value.m_matrix[x][y]) = (*value.m_matrix[x][z]) * o->value.m_matrix[z][y];
                    }
                }
            }
        }
        else{
            matrix = new Object( this );
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*matrix->value.m_matrix[x][y]) *= o;
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

	if( type == H_OT_FLOAT ){
		value.m_double *= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else if( type == H_OT_CHAR ){
		value.m_char *= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else if( type == H_OT_MATRIX ){
	    unsigned int x, y, z;
	    Object *matrix;

	    if( o->type == H_OT_MATRIX ){
            if( value.m_columns != o->value.m_rows ){
                hybris_syntax_error( "first matrix columns have to be the same size of second matrix rows" );
            }
            vector<Object *> dummy;
            matrix = new Object( this->value.m_columns, o->value.m_rows, dummy );
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < o->value.m_columns; ++y ){
                    for( z = 0; z < value.m_columns; z++ ){
                        (*matrix->value.m_matrix[x][y]) = (*value.m_matrix[x][z]) * o->value.m_matrix[z][y];
                    }
                }
            }
        }
        else{
            matrix = new Object( this );
            for( x = 0; x < value.m_rows; ++x ){
                for( y = 0; y < value.m_columns; ++y ){
                    (*matrix->value.m_matrix[x][y]) *= o;
                }
            }
        }
        release(false);
        (*this) = matrix;
	}
	else{
		value.m_integer *= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}

	return this;
}

Object * Object::operator / ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for division operator" );
	}

	if( type == H_OT_FLOAT ){
		return new Object( static_cast<double>(value.m_double / (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue()) ) );
	}
	else if( o->type == H_OT_FLOAT ){
		return new Object( static_cast<double>( o->value.m_double / (type == H_OT_FLOAT ? value.m_double : lvalue()) ) );
	}
	else{
		return new Object( static_cast<long>(lvalue() / o->lvalue()) );
	}
}

Object * Object::operator /= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid type for division operator" );
	}

	if( type == H_OT_FLOAT ){
		value.m_double /= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else if( type == H_OT_CHAR ){
		value.m_char /= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
	}
	else{
		value.m_integer /= (o->type == H_OT_FLOAT ? o->value.m_double : o->lvalue());
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

    if( type == H_OT_INT ){
        value.m_integer = mod;
    }
    else{
        value.m_char = (char)mod;
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
	if( type == H_OT_INT ){
		value.m_integer = val;
	}
	else{
		value.m_char = (char)val;
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
	if( type == H_OT_INT ){
		value.m_integer = val;
	}
	else{
		value.m_char = (char)val;
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
	if( type == H_OT_INT ){
		value.m_integer = val;
	}
	else{
		value.m_char = (char)val;
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
	if( type == H_OT_INT ){
		value.m_integer = val;
	}
	else{
		value.m_char = (char)val;
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
	if( type == H_OT_INT ){
		value.m_integer = val;
	}
	else{
		value.m_char = (char)val;
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
    return new Object( static_cast<long>( this->equals(o) ) );
}

Object * Object::operator != ( Object *o ){
	return new Object( static_cast<long>( !this->equals(o) ) );
}

Object * Object::operator < ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '<' comparision operator (%s, %s)", type_name(this), type_name(o) );
	}

	return new Object( static_cast<long>(lvalue() < o->lvalue()) );
}

Object * Object::operator > ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '>' comparision operator (%s, %s)", type_name(this), type_name(o) );
	}

	return new Object( static_cast<long>(lvalue() > o->lvalue()) );
}

Object * Object::operator <= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '<=' comparision operator (%s, %s)", type_name(this), type_name(o) );
	}

	return new Object( static_cast<long>(lvalue() <= o->lvalue()) );
}

Object * Object::operator >= ( Object *o ){
	if( assert_type( this, o, 3, H_OT_CHAR, H_OT_INT, H_OT_FLOAT ) == 0 ){
		hybris_syntax_error( "invalid types for '>=' comparision operator (%s, %s)", type_name(this), type_name(o) );
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

