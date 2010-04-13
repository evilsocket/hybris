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
#include "types.h"
#include <math.h>
#include <stdio.h>

extern void hyb_throw( H_ERROR_TYPE type, const char *format, ... );

/** helpers **/
void string_replace( string &source, const string find, string replace ) {
    size_t j;
    for( ; (j = source.find( find )) != string::npos; ){
        source.replace( j, find.length(), replace );
    }
}

void string_parse( string& s ){
    /* newline */
    string_replace( s, "\\n", "\n" );
    /* carriage return */
    string_replace( s, "\\r", "\r" );
    /* horizontal tab */
    string_replace( s, "\\t", "\t" );
    /* vertical tab */
    string_replace( s, "\\v", "\v" );
    /* audible bell */
    string_replace( s, "\\a", "\a" );
    /* backspace */
    string_replace( s, "\\b", "\b" );
    /* formfeed */
    string_replace( s, "\\f", "\f" );
    /* escaped double quote */
    string_replace( s, "\\\"", "\"" );

    // handle hex characters
    size_t j, i;
    for( ; (j = s.find( "\\x" )) != string::npos; ){
    string s_hex, repl;
    long l_hex;
    for( i = j + 2; i < s.length(); ++i ){
        // hex digit ?
        if( (s[i] >= 'A' && s[i] <= 'F') || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >= '0' && s[i] <= '9') ){
            s_hex += s[i];
        }
        else{
            break;
        }
    }

    l_hex = strtol( ( "0x" + s_hex ).c_str(), 0, 16 );
            repl += (char)l_hex;
            string_replace( s, "\\x" + s_hex, repl );
    }
}

int string_classify_pcre( string& r ){
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

void string_parse_pcre( string& raw, string& regex, int& opts ){
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
			hyb_throw( H_ET_SYNTAX, "invalid pcre regular expression syntax" );
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

/** generic function pointers **/
void string_set_references( Object *me, int ref ){
    me->ref += ref;
}

Object *string_clone( Object *me ){
    return (Object *)MK_STRING_OBJ( STRING_UPCAST(me)->value.c_str() );
}

size_t string_get_size( Object *me ){
	return STRING_UPCAST(me)->items;
}

byte *string_serialize( Object *o, size_t size ){
	size_t s   	  = (size > ob_get_size(o) ? ob_get_size(o) : size);
	byte  *buffer = new byte[s];

	memcpy( buffer, STRING_UPCAST(o)->value.c_str(), s );

	return buffer;
}

Object *string_deserialize( Object *o, byte *buffer, size_t size ){
	if( size ){
		o = OB_DOWNCAST( MK_STRING_OBJ("") );
		char *tmp = new char[size + 1];
		memset( &tmp, 0x00,   size + 1 );
		memcpy( &tmp, buffer, size );

		STRING_UPCAST(o)->value = tmp;

		delete[] tmp;
	}
	return o;
}

int string_cmp( Object *me, Object *cmp ){
    string svalue = ob_svalue(cmp),
           mvalue = STRING_UPCAST(me)->value;

    if( mvalue == svalue ){
        return 0;
    }
    else if( mvalue > svalue ){
        return 1;
    }
    else{
        return -1;
    }
}

long string_ivalue( Object *me ){
    return atol( STRING_UPCAST(me)->value.c_str() );
}

double string_fvalue( Object *me ){
    return atof( STRING_UPCAST(me)->value.c_str() );
}

bool string_lvalue( Object *me ){
    return (bool)STRING_UPCAST(me)->value.size();
}

string string_svalue( Object *me ){
    return STRING_UPCAST(me)->value;
}

void string_print( Object *me, int tabs ){
    for( int i = 0; i < tabs; ++i ){
        printf( "\t" );
    }
    printf( "%s", STRING_UPCAST(me)->value.c_str() );
}

void string_scanf( Object *me ){
    char tmp[0xFF] = {0};

    scanf( "%s", tmp );

    STRING_UPCAST(me)->value = tmp;
}

Object * string_to_string( Object *me ){
    return me;
}

Object * string_to_int( Object *me ){
    return (Object *)MK_INT_OBJ( atol(STRING_UPCAST(me)->value.c_str()) );
}

Object * string_from_int( Object *i ){
    char tmp[0xFF] = {0};
    long ivalue    = ob_ivalue(i);

    sprintf( tmp, "%ld", ivalue );

    return (Object *)MK_STRING_OBJ( tmp );
}

Object * string_from_float( Object *f ){
    char   tmp[0xFF] = {0};
    double fvalue    = ob_fvalue(f);

    sprintf( tmp, "%lf", fvalue );

    return (Object *)MK_STRING_OBJ( tmp );
}

Object * string_regexp( Object *me, Object *r ){
	string rawreg  = ob_svalue(r),
		   subject = ob_svalue(me),
		   regex;
	int    opts;

	string_parse_pcre( rawreg, regex, opts );

	if( string_classify_pcre( regex ) == H_PCRE_BOOL_MATCH ){
		pcrecpp::RE_Options OPTS(opts);
		pcrecpp::RE         REGEX( regex.c_str(), OPTS );

        return (Object *)MK_INT_OBJ( REGEX.PartialMatch(subject.c_str()) );
	}
	else{
		pcrecpp::RE_Options  OPTS(opts);
		pcrecpp::RE          REGEX( regex.c_str(), OPTS );
		pcrecpp::StringPiece SUBJECT( subject.c_str() );
		string  match;

        VectorObject *matches = MK_VECTOR_OBJ();
        int i = 0;

        while( REGEX.FindAndConsume( &SUBJECT, &match ) == true ){
            if( i++ > H_PCRE_MAX_MATCHES ){
                hyb_throw( H_ET_GENERIC, "something of your regex is forcing infinite matches" );
            }

            ob_cl_push_reference( (Object *)matches, (Object *)MK_STRING_OBJ(match.c_str()) );
        }

        return (Object *)matches;
	}
}

/** arithmetic operators **/
Object *string_assign( Object *me, Object *op ){
    if( IS_STRING_TYPE(op) ){
        STRING_UPCAST(me)->value = STRING_UPCAST(op)->value;
    }
    else {
        Object *clone = ob_clone(op);

        ob_set_references( clone, +1 );

        me = clone;
    }

    return me;
}

Object *string_l_same( Object *me, Object *op ){
    return (Object *)MK_INT_OBJ( (STRING_UPCAST(me))->value == ob_svalue(op) );
}

Object *string_l_diff( Object *me, Object *op ){
    return (Object *)MK_INT_OBJ( (STRING_UPCAST(me))->value != ob_svalue(op) );
}

/** collection operators **/
Object *string_cl_concat( Object *me, Object *op ){
    string mvalue = ob_svalue(me),
           svalue = ob_svalue(op);

    return (Object *)MK_STRING_OBJ( (mvalue + svalue).c_str() );
}

Object *string_cl_inplace_concat( Object *me, Object *op ){
    string svalue = ob_svalue(op);

    STRING_UPCAST(me)->value += svalue;
    STRING_UPCAST(me)->items += svalue.size();

    return me;
}

Object *string_cl_at( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);
    #ifdef BOUNDS_CHECK
    if( idx >= STRING_UPCAST(me)->items ){
        hyb_throw( H_ET_GENERIC, "index out of bounds" );
    }
    #endif
    char chr = STRING_UPCAST(me)->value[idx];

    return (Object *)MK_CHAR_OBJ( chr );
}

Object *string_cl_set( Object *me, Object *i, Object *v ){
    size_t idx = ob_ivalue(i);
    #ifdef BOUNDS_CHECK
    if( idx >= STRING_UPCAST(me)->items ){
        hyb_throw( H_ET_GENERIC, "index out of bounds" );
    }
    #endif

    STRING_UPCAST(me)->value[idx] = ob_ivalue(v);

    return me;
}

IMPLEMENT_TYPE(String) {
    /** type code **/
    otString,
	/** type name **/
    "string",
	/** type basic size **/
    0,

	/** generic function pointers **/
	string_set_references, // set_references
	string_clone, // clone
	0, // free
	string_get_size, // get_size
	string_serialize, // serialize
	string_deserialize, // deserialize
	string_cmp, // cmp
	string_ivalue, // ivalue
	string_fvalue, // fvalue
	string_lvalue, // lvalue
	string_svalue, // svalue
	string_print, // print
	string_scanf, // scanf
	string_to_string, // to_string
	string_to_int, // to_int
	string_from_int, // from_int
	string_from_float, // from_float
	0, // range
	string_regexp, // regexp

	/** arithmetic operators **/
	string_assign, // assign
    0, // factorial
    0, // increment
    0, // decrement
    0, // minus
    0, // add
    0, // sub
    0, // mul
    0, // div
    0, // mod
    0, // inplace_add
    0, // inplace_sub
    0, // inplace_mul
    0, // inplace_div
    0, // inplace_mod

	/** bitwise operators **/
	0, // bw_and
    0, // bw_or
    0, // bw_not
    0, // bw_xor
    0, // bw_lshift
    0, // bw_rshift
    0, // bw_inplace_and
    0, // bw_inplace_or
    0, // bw_inplace_xor
    0, // bw_inplace_lshift
    0, // bw_inplace_rshift

	/** logic operators **/
    0, // l_not
    string_l_same, // l_same
    string_l_diff, // l_diff
    0, // l_less
    0, // l_greater
    0, // l_less_or_same
    0, // l_greater_or_same
    0, // l_or
    0, // l_and

	/** collection operators **/
	string_cl_concat, // cl_concat
	string_cl_inplace_concat, // cl_inplace_concat
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	string_cl_at, // cl_at
	string_cl_set, // cl_set
	0, // cl_set_reference

	/** structure operators **/
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0  // set_attribute_reference;
};

