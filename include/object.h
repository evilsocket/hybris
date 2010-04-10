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
#   define H_ADDRESS_OF(o) reinterpret_cast<ulong>(o)
#endif

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <pcrecpp.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

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
    H_OT_STRUCT,
    H_OT_CLASS
};

// macros to easily create objects at runtime
#define MK_INT_OBJ(v)        new Object( static_cast<long>(v) )
#define MK_EXT_OBJ(v,e)      new Object( static_cast<long>(v), static_cast<unsigned int>(e) )
#define MK_FLOAT_OBJ(v)      new Object( static_cast<double>(v) )
#define MK_CHAR_OBJ(v)       new Object( static_cast<char>(v) )
#define MK_STRING_OBJ(v)     new Object( (char *)(v) )
#define MK_ALIAS_OBJ(v)      new Object( static_cast<unsigned int>(v) )
#define MK_CLONE_OBJ(o)      new Object( static_cast<Object *>(o) )
#define MK_COLLECTION_OBJ(o) new Object()
// this one takes a C pointer and returns an H_OT_INTEGER representing its address
#define PTR_TO_INT_OBJ(p)    new Object( reinterpret_cast<long>(p) )
// same as before, but for tmp objects that are cloned inside methods (no need for dynamic allocation)
#define MK_TMP_INT_OBJ(v)        &( Object( static_cast<long>(v) ) )
#define MK_TMP_EXT_OBJ(v,e)      &( Object( static_cast<long>(v), static_cast<unsigned int>(e) ) )
#define MK_TMP_FLOAT_OBJ(v)      &( Object( static_cast<double>(v) ) )
#define MK_TMP_CHAR_OBJ(v)       &( Object( static_cast<char>(v) ) )
#define MK_TMP_STRING_OBJ(v)     &( Object( (char *)(v) ) )
#define MK_TMP_ALIAS_OBJ(v)      &( Object( static_cast<unsigned int>(v) ) )
#define MK_TMP_CLONE_OBJ(o)      &( Object( static_cast<Object *>(o) ) )
#define MK_TMP_COLLECTION_OBJ(o) &( Object() )
#define PTR_TO_TMP_INT_OBJ(p)    &( Object( reinterpret_cast<long>(p) ) )

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

class NamedValue {
public:
    string  name;
    Object *value;

    NamedValue( string n, Object *v );
};

typedef vector<NamedValue>  struct_t;

enum H_ACCESS_SPECIFIER {
    H_AS_PRIVATE = 0,
    H_AS_PROTECTED,
    H_AS_PUBLIC,
    H_AS_STATIC
};

template< class T > class NamedMember {
public :
    string             name;
    T *                value;
    H_ACCESS_SPECIFIER access;

    NamedMember( string n, T *v, H_ACCESS_SPECIFIER a ) :
        name(n),
        value(v),
        access(a) {

    }
};

class Node;

typedef NamedMember<Object> attribute_t;
typedef NamedMember<Node>   method_t;

class class_t {
public :
    vector<attribute_t> attributes;
    vector<method_t>    methods;
};

class ObjectValue {
    public:

        long             m_integer;
        double           m_double;
        char             m_char;
        string           m_string;
        unsigned int     m_alias;

        vector<Object *> m_array;
        vector<Object *> m_map;

        unsigned int     m_rows;
        unsigned int     m_columns;
        Object       *** m_matrix;

        struct_t         m_struct;

        class_t          m_class;
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

    /* inline cast operators to access the object value */
    inline operator long(){ return value.m_integer; }
    inline operator double(){ return value.m_double; }
    inline operator char(){ return value.m_char; }
    inline operator const char *(){ return value.m_string.c_str(); }
    inline operator char *(){ return (char *)value.m_string.c_str(); }

	#ifdef MEM_DEBUG
    void* operator new (size_t size);
    #endif

    /* explicit type ctor */
    Object( H_OBJECT_TYPE type );
    /* default ctor */
    Object();
    /* ctors by type/vale */
    Object( long value );
    Object( long value, unsigned int _is_extern );
    Object( double value );
    Object( char value );
    Object( char *value );
    Object( vector<unsigned char>& data );
	Object( unsigned int value );
	Object( unsigned int rows, unsigned int columns, vector<Object *>& data );
	/* ctor by pointer */
    Object( Object *o );
    /* dtor, calls ::release */
    ~Object();

    Object& assign( Object *o );
    void    release( bool reset_attributes = true );
	int     equals( Object *o );

	void     addAttribute( char *name );
    Object  *getAttribute( char *name );
    void     setAttribute( char *name, Object *value );
    void     setAttribute_ref( char *name, Object *value );

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
	Object *map_ref( Object *map, Object *o );
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

