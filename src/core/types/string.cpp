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
#include "hybris.h"
#include <pcre.h>
#include <algorithm>

/** helpers **/
size_t string_replace( string &source, const string find, string replace ) {
    int i,
		count,
		f_len( find.length() ),
		r_len( replace.length() );

    for( i = 0, count = 0; (i = source.find( find, i )) != string::npos; i += r_len, ++count ){
        source.replace( i, f_len, replace );
    }

    return count;
}

/** builtin methods **/
Object *__string_length( vm_t *vm, Object *me, vframe_t *data ){
	return (Object *)gc_new_integer( ob_string_ucast(me)->value.size() );
}

Object *__string_find( vm_t *vm, Object *me, vframe_t *data ){
	if(  vargc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'find' requires 1 parameter (called with %d)",  vargc() );
	}
	ob_argv_types_assert( 0, otString, otChar, "find" );

	string needle = ob_is_char( vm_argv(0) ) ? string("") + ob_char_ucast(vm_argv(0))->value : ob_string_ucast(vm_argv(0))->value;

	int found = ob_string_ucast(me)->value.find(needle);

	return (Object *)gc_new_integer( found );
}

Object *__string_substr( vm_t *vm, Object *me, vframe_t *data ){
	if( vargc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'substr' requires at least 1 parameter (called with %d)", vargc() );
	}
	ob_type_assert( vm_argv(0), otInteger, "substr" );

	string sub;
	if( vargc() == 2 ){
		ob_type_assert( vm_argv(1), otInteger, "substr" );
		sub = ob_string_ucast(me)->value.substr( ob_ivalue( vm_argv(0) ), ob_ivalue( vm_argv(1) ) );
	}
	else{
		sub = ob_string_ucast(me)->value.substr( ob_ivalue( vm_argv(0) ) );
	}

	return (Object *)gc_new_string( sub.c_str() );
}

Object *__string_replace( vm_t *vm, Object *me, vframe_t *data ){
	if( vargc() < 2 ){
		hyb_error( H_ET_SYNTAX, "method 'replace' requires 2 parameters (called with %d)", vargc() );
	}
	ob_type_assert( vm_argv(0), otString, "replace" );
	ob_type_assert( vm_argv(1), otString, "replace" );

	string str  = ob_string_ucast(me)->value,
		   tmp  = str,
		   find = ob_svalue( data->at(0) ),
		   repl = ob_svalue( data->at(1) );

	int    i,
		   f_len( find.length() ),
		   r_len( repl.length() + 1 );

	for( i = 0; (i = str.find( find, i )) != string::npos ; i += r_len ){
		str.replace( i, f_len, repl );
	}

	return (Object *)gc_new_string( str.c_str() );
}

Object *__string_split( vm_t *vm, Object *me, vframe_t *data ){
	if( vargc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'split' requires 1 parameter (called with %d)", vargc() );
	}
	ob_types_assert( vm_argv(0), otString, otChar, "split" );

	string str = ob_string_ucast(me)->value,
		   tok = ob_svalue( vm_argv(0) );
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

Object *__string_trim( vm_t *vm, Object *me, vframe_t *data ){
	string s = ob_string_ucast(me)->value;

	// trim from start
	s.erase( s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))) );
	// trim from end
	s.erase( std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end() );

	return (Object *)gc_new_string(s.c_str());
}

Object *__string_repeat( vm_t *vm, Object *me, vframe_t *data ){
	if( vargc() < 1 ){
		hyb_error( H_ET_SYNTAX, "method 'repeat' requires 1 parameter (called with %d)", vargc() );
	}
	ob_type_assert( vm_argv(0), otInteger, "repeat" );

	string str 	  = ob_string_ucast(me)->value,
		   repeated("");
	size_t repeat = ob_ivalue( vm_argv(0) ),
		   i;

	for( i = 0; i < repeat; ++i ){
		repeated += str;
	}

	return (Object *)gc_new_string(repeated.c_str());
}

/** generic function pointers **/
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
	int rd = 0;
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
		return vm_raise_exception( "error during regex evaluation at offset %d (%s)", eoffset, error );
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
    	return vm_raise_exception( "index out of bounds" );
    }

    char chr = ob_string_ucast(me)->value[idx];

    return (Object *)gc_new_char( chr );
}

Object *string_cl_set( Object *me, Object *i, Object *v ){
    size_t idx = ob_ivalue(i);

    if( idx >= ob_string_ucast(me)->items ){
    	return vm_raise_exception( "index out of bounds" );
    }

    char c = (char)ob_ivalue(v);
    if( c == 0x00 ){
    	ob_string_ucast(me)->value.erase(idx);
    }
    else{
    	ob_string_ucast(me)->value[idx] = c;
    }

    return me;
}

Object *string_call_method( vm_t *vm, vframe_t *frame, Object *me, char *me_id, char *method_id, Node *argv ){
	ob_type_builtin_method_t *method = NULL;

	if( (method = ob_get_builtin_method( me, method_id )) == NULL ){
		hyb_error( H_ET_SYNTAX, "String type does not have a '%s' method", method_id );
	}
	ll_item_t *iitem;
	Object  *value,
			*result;
	vframe_t stack;
	size_t   i, argc = argv->children();

	/*
	 * Add this frame as the active stack
	 */
	vm_add_frame( vm, &stack );

	stack.owner = ob_typename(me) + string("::") + method_id;
	/*
	 * Evaluate each object and insert it into the stack
	 */
	ll_foreach_to( &argv->m_children, iitem, i, argc ){
		value = vm_exec( vm, frame, ll_node( iitem ) );

		if( frame->state.is(Exception) ){
			vm_pop_frame( vm );
			return frame->state.e_value;
		}
		else if( frame->state.is(Return) ){
			vm_pop_frame( vm );
			return frame->state.r_value;
		}

		stack.push( value );
	}

	/* execute the method */
	result = ((ob_type_builtin_method_t)method)( vm, me, &stack );

	/*
	 * Dismiss the stack.
	 */
	vm_pop_frame( vm );

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

static ob_builtin_method_t string_builtin_methods[] = {
	{ "length",  (ob_type_builtin_method_t *)__string_length },
	{ "find",    (ob_type_builtin_method_t *)__string_find },
	{ "substr",  (ob_type_builtin_method_t *)__string_substr },
	{ "replace", (ob_type_builtin_method_t *)__string_replace },
	{ "split",   (ob_type_builtin_method_t *)__string_split },
	{ "trim",    (ob_type_builtin_method_t *)__string_trim },
	{ "repeat",  (ob_type_builtin_method_t *)__string_repeat },
	OB_BUILIN_METHODS_END_MARKER
};

IMPLEMENT_TYPE(String) {
    /** type code **/
    otString,
	/** type name **/
    "string",
	/** type basic size **/
    OB_COLLECTION_SIZE,
    /** type builtin methods **/
    string_builtin_methods,
	/** generic function pointers **/
    0, // type_name
    0, // traverse
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

