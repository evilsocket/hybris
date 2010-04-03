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
#ifndef _HOBJECT_H_
#	define _HOBJECT_H_

#include "config.h"

/* helper macro to obtain the address of a pointer */
#ifndef H_ADDRESS_OF
#   define H_ADDRESS_OF(o) reinterpret_cast<unsigned long>(o)
#endif

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#ifdef PCRE_SUPPORT
    #include <pcrecpp.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

extern void  hybris_syntax_error( const char *format, ... );
extern void  hybris_generic_error( const char *format, ... );

using namespace std;

/* object types */
enum H_OBJECT_TYPE {
    H_OT_VOID = 0,
    H_OT_INT,
    H_OT_FLOAT,
    H_OT_CHAR,
    H_OT_STRING,
    H_OT_BINARY,
    H_OT_ARRAY,
    H_OT_MAP,
    H_OT_ALIAS,
    H_OT_MATRIX,
    H_OT_STRUCT
};

#ifdef GC_SUPPORT
    typedef unsigned char H_OBJECT_ATTRIBUTE;

/* object attributes */
#   define H_OA_NONE     0 // 00000000
#   define H_OA_EXTERN   1 // 00000001
#   define H_OA_CONSTANT 2 // 00000010
#   define H_OA_GARBAGE  4 // 00000100
#endif

#ifdef PCRE_SUPPORT
#   define H_PCRE_MAX_MATCHES   300

#   define H_PCRE_BOOL_MATCH    0x0
#   define H_PCRE_MULTI_MATCH   0x1
#   define H_PCRE_REPLACE_MATCH 0x2
#endif

class Object;

class ObjectValue {
    public:

        long             m_integer;
        double           m_double;
        char             m_char;
        string           m_string;
        vector<Object *> m_array;
        vector<Object *> m_map;
        unsigned int     m_alias;

        unsigned int     m_rows;
        unsigned int     m_columns;
        Object       *** m_matrix;

        vector<string>   m_struct_names;
        vector<Object *> m_struct_values;
};

class Object {
public  :
     /* return 1 if a and b are of one of the types described on the arguments */
	static unsigned int assert_type( Object *a, Object *b, unsigned int ntypes, ... );
    /* replace each occurrence of "find" in "source" with "replace" */
    static void replace( string &source, const string find, string replace );
    /* parse a string for escape and special characters encoding */
    static void parse_string( string& s );
    /* return a string rapresentation of the type */
    static const char *type_name( Object *o );

    #ifdef PCRE_SUPPORT
    /* tells if a regex is a single match or multi match */
    static int  classify_pcre( string& r );
    /* split raw regex and its options from pcre regex */
    static void parse_pcre( string& raw, string& regex, int& opts );
    /* regex operator */
    Object     *regexp( Object *regexp );
    #endif

    #ifdef GC_SUPPORT
    /* object gc attribute mask */
    H_OBJECT_ATTRIBUTE attributes;
    /* set the object as deletable or not deletable */
    void setGarbageAttribute( H_OBJECT_ATTRIBUTE mask );
    #endif
    /* type of the object */
    H_OBJECT_TYPE      type;
    /* size of the object */
    unsigned int       size;
    /* value of the object */
    ObjectValue        value;

	#ifdef MEM_DEBUG
    void* operator new (size_t size);
    #endif

    /* ctors */
    Object( long value );
    Object( long value, unsigned int _is_extern );
    Object( double value );
    Object( char value );
    Object( char *value );
    Object( vector<unsigned char>& data );
	Object();
	Object( unsigned int value );
	Object( unsigned int rows, unsigned int columns, vector<Object *>& data );
    Object( Object *o );
    /* dtor, calls ::release */
    ~Object();

    void release( bool reset_attributes = true );

    Object& assign( Object *o );


	int equals( Object *o );

	void     addAttribute( char *name );
    Object  *getAttribute( char *name );
    void     setAttribute( char *name, Object *value );

	int mapFind( Object *map );

	Object * getObject();

    /* console i/o methods */
    void print( unsigned int tabs = 0 );
    void println( unsigned int tabs = 0 );
    void input();
    /* conversion methods */
	Object * toString();
    Object * toInt();

    /* operators */
	long lvalue();
    Object * dot( Object *o );
	Object * dotequal( Object *o );
    string svalue();
	Object *push( Object *o );
	Object *push_ref( Object *o );
	Object *map( Object *map, Object *o );
	Object *pop();
	Object *mapPop();
	Object *remove( Object *index );
	Object *unmap( Object *map );
    Object *at( Object *index );
	Object *at( char *map );
    Object& at( Object *index, Object *set );
    Object* range( Object *to );

    Object&  operator = ( Object *o );
    Object * factorial();
    Object&  operator ++ ();
    Object&  operator -- ();
    Object * operator + ( Object *o );
    Object * operator - ( Object *o );
    Object * operator - ();
    Object * operator * ( Object *o );
    Object * operator / ( Object *o );
    Object * operator % ( Object *o );
	Object * operator += ( Object *o );
	Object * operator -= ( Object *o );
	Object * operator *= ( Object *o );
	Object * operator /= ( Object *o );
	Object * operator %= ( Object *o );

    Object * operator & ( Object *o );
    Object * operator | ( Object *o );
    Object * operator ~ ();
    Object * operator ^ ( Object *o );
    Object * operator << ( Object *o );
    Object * operator >> ( Object *o );
	Object * operator &= ( Object *o );
    Object * operator |= ( Object *o );
    Object * operator ^= ( Object *o );
	Object * operator <<= ( Object *o );
	Object * operator >>= ( Object *o );

    Object * operator ! ();
	Object * lnot ();
    Object * operator == ( Object *o );
    Object * operator != ( Object *o );
    Object * operator < ( Object *o );
    Object * operator > ( Object *o );
    Object * operator <= ( Object *o );
    Object * operator >= ( Object *o );
    Object * operator || ( Object *o );
    Object * operator && ( Object *o );
};

#endif

