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
#include "vm.h"
#include <math.h>
#include <stdio.h>
#include <pcre.h>
#include <algorithm>

extern void hyb_error( H_ERROR_TYPE type, const char *format, ... );

/** helpers **/
size_t string_replace( string &source, const string find, string replace ) {
    size_t j, count(0);
    for( ; (j = source.find( find )) != string::npos; count++ ){
        source.replace( j, find.length(), replace );
    }
    return count;
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

/** builtin methods **/
Object *__string_length( engine_t *engine, Object *me, vframe_t *data ){
	return (Object *)gc_new_integer( ob_string_ucast(me)->value.size() );
}

Object *__string_find( engine_t *engine, Object *me, vframe_t *data ){
	if(  ob_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'find' requires 1 parameter (called with %d)",  ob_argc() );
	}
	ob_argv_types_assert( 0, otString, otChar, "find" );

	string needle = ob_is_char( ob_argv(0) ) ? string("") + ob_char_ucast(ob_argv(0))->value : ob_string_ucast(ob_argv(0))->value;

	int found = ob_string_ucast(me)->value.find(needle);

	return (Object *)gc_new_integer( found );
}

Object *__string_substr( engine_t *engine, Object *me, vframe_t *data ){
	if( ob_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'substr' requires at least 1 parameter (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otInteger, "substr" );

	string sub;
	if( ob_argc() == 2 ){
		ob_type_assert( ob_argv(1), otInteger, "substr" );
		sub = ob_string_ucast(me)->value.substr( ob_ivalue( ob_argv(0) ), ob_ivalue( ob_argv(1) ) );
	}
	else{
		sub = ob_string_ucast(me)->value.substr( ob_ivalue( ob_argv(0) ) );
	}

	return (Object *)gc_new_string( sub.c_str() );
}

Object *__string_replace( engine_t *engine, Object *me, vframe_t *data ){
	if( ob_argc() < 2 ){
		hyb_error( H_ET_SYNTAX, "method 'replace' requires 2 parameters (called with %d)", ob_argc() );
	}
	ob_type_assert( ob_argv(0), otString, "replace" );
	ob_type_assert( ob_argv(1), otString, "replace" );

	string str  = ob_string_ucast(me)->value,
		   find = string_argv(0),
		   repl = string_argv(1);

	size_t i, f_len( find.length() );
	for( ; (i = str.find( find )) != string::npos ; ){
		str.replace( i, f_len, repl );
	}

	return (Object *)gc_new_string( str.c_str() );
}

Object *__string_split( engine_t *engine, Object *me, vframe_t *data ){
	if( ob_argc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'split' requires 1 parameter (called with %d)", ob_argc() );
	}
	ob_types_assert( ob_argv(0), otString, otChar, "split" );

	string str = ob_string_ucast(me)->value,
		   tok = ob_svalue( ob_argv(0) );
	vector<string> parts;
    int start = 0, end = 0, i;

  	while( (end = str.find(tok,start)) != string::npos ){
		parts.push_back( str.substr( start, end - start ) );
		start = end + tok.size();
	}
	parts.push_back( str.substr(start) );

   	Object *array = ob_dcast( gc_new_vector() );
	for( i = 0; i < parts.size(); ++i ){
		ob_cl_push_reference( array, ob_dcast( gc_new_string( parts[i].c_str() ) ) );
	}

	return array;
}

Object *__string_trim( engine_t *engine, Object *me, vframe_t *data ){
	string s = ob_string_ucast(me)->value;

	// trim from start
	s.erase( s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))) );
	// trim from end
	s.erase( std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end() );

	return (Object *)gc_new_string(s.c_str());
}

/** generic function pointers **/
const char *string_typename( Object *o ){
	return o->type->name;
}

Object *string_traverse( Object *me, int index ){
	return NULL;
}

Object *string_clone( Object *me ){
    return (Object *)gc_new_string( ob_string_ucast(me)->value.c_str() );
}

size_t string_get_size( Object *me ){
	return ob_string_ucast(me)->items;
}

byte *string_serialize( Object *o, size_t size ){
	size_t s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o) );
	byte  *buffer = new byte[s];

	memcpy( buffer, ob_string_ucast(o)->value.c_str(), s );

	return buffer;
}

Object *string_deserialize( Object *o, byte *buffer, size_t size ){
	if( size ){
		o = ob_dcast( gc_new_string("") );
		char *tmp = new char[size + 1];
		memset( &tmp, 0x00,   size + 1 );
		memcpy( &tmp, buffer, size );

		ob_string_ucast(o)->value = tmp;

		delete[] tmp;
	}
	else{
		size_t i = 0;
		byte   c;

		ob_string_ucast(o)->value = "";
		do{
			ob_string_val(o) += c = buffer[i++];
		}
		while( c != '\n' );

		ob_string_ucast(o)->items = i;
	}
	return o;
}

Object *string_to_fd( Object *o, int fd, size_t size ){
	size_t s = (size > ob_get_size(o) ? ob_get_size(o) : size != 0 ? size : ob_get_size(o));
	int    written;

	written = write( fd, ob_string_ucast(o)->value.c_str(), s );

	return ob_dcast( gc_new_integer(written) );
}

Object *string_from_fd( Object *o, int fd, size_t size ){
	int rd = 0, n;
	if( size ){
		char *tmp = (char *)alloca(size + 1);
		memset( tmp, 0x00, size + 1 );

		if( (rd = read( fd, tmp, size )) > 0 ){
			ob_string_ucast(o)->value = tmp;
		}
	}
	else{
		byte c, n;

		ob_string_ucast(o)->value = "";
		do{
			if( (n = read( fd, &c, sizeof(byte) )) > 0 ){
				rd++;
				ob_string_val(o) += c;
				ob_string_ucast(o)->items++;
			}
		}
		while( c != '\n' && n && c);
	}
	return ob_dcast( gc_new_integer(rd) );
}

int string_cmp( Object *me, Object *cmp ){
    string svalue = ob_svalue(cmp),
           mvalue = ob_string_ucast(me)->value;

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
    return atol( ob_string_ucast(me)->value.c_str() );
}

double string_fvalue( Object *me ){
    return atof( ob_string_ucast(me)->value.c_str() );
}

bool string_lvalue( Object *me ){
    return (bool)ob_string_ucast(me)->value.size();
}

string string_svalue( Object *me ){
    return ob_string_ucast(me)->value;
}

void string_print( Object *me, int tabs ){
    for( int i = 0; i < tabs; ++i ){
        fprintf( stdout, "\t" );
    }
    fprintf( stdout, "%s", ob_string_ucast(me)->value.c_str() );
}

void string_scanf( Object *me ){
    char tmp[0xFF] = {0};

    scanf( "%s", tmp );

    ob_string_ucast(me)->value = tmp;
}

Object * string_to_string( Object *me ){
    return me;
}

Object * string_to_int( Object *me ){
    return (Object *)gc_new_integer( atol(ob_string_ucast(me)->value.c_str()) );
}

void string_parse_pcre( string& raw, string& regex, int& opts ){
    int i, ccount, rc,
	   *offsets,
        eoffset;
	const char  *error;
    string       pattern("^/(.*?)/([i|m|s|x|U]*)$"),
				 sopts;
    pcre 		*compiled;
    extern vm_t *__hyb_vm;

    compiled = vm_pcre_compile( __hyb_vm, pattern, 0, &error, &eoffset );
    rc 		 = pcre_fullinfo( compiled, 0, PCRE_INFO_CAPTURECOUNT, &ccount );
    offsets  = new int[ 3 * (ccount + 1) ];
    rc 		 = pcre_exec( compiled, 0, raw.c_str(), raw.length(), 0, 0, offsets, 3 * (ccount + 1) );

    if( rc < 0 ){
    	regex = raw;
    	opts  = 0;
    }
    else{
    	regex = raw.substr( offsets[2], offsets[3] - offsets[2] );
    	sopts = raw.substr( offsets[4], offsets[5] - offsets[4] );
    	opts  = 0;

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

    delete[] offsets;
}

Object * string_regexp( Object *me, Object *r ){
	string 		 rawreg  = ob_svalue(r),
			     subject = ob_svalue(me),
				 pattern;
	int    		 opts, i, ccount, rc,
				*offsets, offset = 0,
				 eoffset;
	const char  *error;
	pcre 		*compiled;
	Object      *_pcre_return;
	extern vm_t *__hyb_vm;


	string_parse_pcre( rawreg, pattern, opts );

	compiled = vm_pcre_compile( __hyb_vm, pattern, opts, &error, &eoffset );
	if( !compiled ){
		hyb_error( H_ET_GENERIC, "error during regex evaluation at offset %d (%s)", eoffset, error );
    }

	rc = pcre_fullinfo( compiled, 0, PCRE_INFO_CAPTURECOUNT, &ccount );

	offsets = new int[ 3 * (ccount + 1) ];
	/*
	 * The regex is going to capture at least one element, return a vector.
	 */
	if( ccount > 0 ){
		_pcre_return = (Object *)gc_new_vector();

		while( (rc = pcre_exec( compiled, 0, subject.c_str(), subject.length(), offset, 0, offsets, 3 * (ccount + 1) )) > 0 ){
			const char *data;
			for( i = 1; i < rc; ++i ){
				pcre_get_substring( subject.c_str(), offsets, rc, i, &data );
				ob_cl_push_reference( _pcre_return,
									  (Object *)gc_new_string(data)
									);
			}
			offset = offsets[1];
		}
	}
	/*
	 * The regex will only match the subject against the pattern, no capture.
	 * Return an integer/boolean.
	 */
	else{
		rc = pcre_exec( compiled, 0, subject.c_str(), subject.length(), offset, 0, offsets, 3 * (ccount + 1) );
		_pcre_return = (Object *)gc_new_integer( rc >= 0 );
	}

	delete[] offsets;

	return _pcre_return;
}

/** arithmetic operators **/
Object *string_assign( Object *me, Object *op ){
    if( ob_is_string(op) ){
        ob_string_ucast(me)->value = ob_string_ucast(op)->value;
    }
    else {
        Object *clone = ob_clone(op);

        me = clone;
    }

    return me;
}

Object *string_add( Object *me, Object *op ){
    string mvalue = ob_svalue(me),
           svalue = ob_svalue(op);

    return (Object *)gc_new_string( (mvalue + svalue).c_str() );
}

Object *string_inplace_add( Object *me, Object *op ){
    string svalue = ob_svalue(op);

    ob_string_ucast(me)->value += svalue;
    ob_string_ucast(me)->items += svalue.size();

    return me;
}

Object *string_l_same( Object *me, Object *op ){
    return (Object *)gc_new_integer( (ob_string_ucast(me))->value == ob_svalue(op) );
}

Object *string_l_diff( Object *me, Object *op ){
    return (Object *)gc_new_integer( (ob_string_ucast(me))->value != ob_svalue(op) );
}

/** collection operators **/
Object *string_cl_at( Object *me, Object *i ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_string_ucast(me)->items ){
        hyb_error( H_ET_GENERIC, "index out of bounds" );
    }

    char chr = ob_string_ucast(me)->value[idx];

    return (Object *)gc_new_char( chr );
}

Object *string_cl_set( Object *me, Object *i, Object *v ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_string_ucast(me)->items ){
        hyb_error( H_ET_GENERIC, "index out of bounds" );
    }

    ob_string_ucast(me)->value[idx] = ob_ivalue(v);

    return me;
}

Object *string_call_method( engine_t *engine, vframe_t *frame, Object *me, char *me_id, char *method_id, Node *argv ){
	ob_type_builtin_method_t *method = NULL;

	if( (method = ob_get_builtin_method( me, method_id )) == NULL ){
		hyb_error( H_ET_SYNTAX, "String type does not have a '%s' method", method_id );
	}

	Object  *value,
			*result;
	vframe_t stack;
	size_t   i, argc = argv->children();

	stack.owner = ob_typename(me) + string("::") + method_id;
	/*
	 * Evaluate each object and insert it into the stack
	 */
	for( i = 0; i < argc; ++i ){
		value = engine_exec( engine, frame, argv->child(i) );
		stack.push( value );
	}
	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( engine->vm, &stack );

	/* execute the method */
	result = ((ob_type_builtin_method_t)method)( engine, me, &stack );

	/*
	 * Dismiss the stack.
	 */
	vm_pop_frame( engine->vm );

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

IMPLEMENT_TYPE(String) {
    /** type code **/
    otString,
	/** type name **/
    "string",
	/** type basic size **/
    0,
    /** type builtin methods **/
    {
    	{ "length",  (ob_type_builtin_method_t *)__string_length },
        { "find",    (ob_type_builtin_method_t *)__string_find },
        { "substr",  (ob_type_builtin_method_t *)__string_substr },
        { "replace", (ob_type_builtin_method_t *)__string_replace },
        { "split",   (ob_type_builtin_method_t *)__string_split },
        { "trim",    (ob_type_builtin_method_t *)__string_trim },
	    OB_BUILIN_METHODS_END_MARKER
    },

	/** generic function pointers **/
    string_typename, // type_name
    string_traverse, // traverse
	string_clone, // clone
	0, // free
	string_get_size, // get_size
	string_serialize, // serialize
	string_deserialize, // deserialize
	string_to_fd, // to_fd
	string_from_fd, // from_fd
	string_cmp, // cmp
	string_ivalue, // ivalue
	string_fvalue, // fvalue
	string_lvalue, // lvalue
	string_svalue, // svalue
	string_print, // print
	string_scanf, // scanf
	string_to_string, // to_string
	string_to_int, // to_int
	0, // range
	string_regexp, // regexp

	/** arithmetic operators **/
	string_assign, // assign
    0, // factorial
    0, // increment
    0, // decrement
    0, // minus
    string_add, // add
    0, // sub
    0, // mul
    0, // div
    0, // mod
    string_inplace_add, // inplace_add
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
	0, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	string_cl_at, // cl_at
	string_cl_set, // cl_set
	0, // cl_set_reference

	/** structure operators **/
	0, // define_attribute
	0, // attribute_access
	0, // attribute_is_static
	0, // set_attribute_access
    0, // add_attribute;
    0, // get_attribute;
    0, // set_attribute;
    0, // set_attribute_reference
    0, // define_method
    0, // get_method
    string_call_method // call_method
};

