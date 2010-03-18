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

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#ifdef XML_SUPPORT
    #include <libxml/parser.h>
    #include <libxml/tree.h>
#endif
#include <string.h>

extern void  hybris_syntax_error( const char *format, ... );
extern void  hybris_generic_error( const char *format, ... );

using std::string;
using std::stringstream;
using std::vector;
using std::cout;
using std::cin;

typedef unsigned short H_OBJECT_TYPE;

#define H_OT_NONE   0
#define H_OT_INT    1
#define H_OT_FLOAT  2
#define H_OT_CHAR   3
#define H_OT_STRING 4
#define H_OT_ARRAY  5
#define H_OT_MAP    6
#define H_OT_ALIAS  7
#define H_OT_MATRIX 8

class Object {
public  :
    /* return a string rapresentation of the type */
    static const char *type( Object *o );

    #ifdef XML_SUPPORT
    /* create an object from a xml tree */
	static Object *fromxml( xmlNode *node );
    /* create an object from a xml stream */
	static Object *fromxml( char *xml );
    #endif

    /* return 1 if a and b are of one of the types described on the arguments */
	static unsigned int assert_type( Object *a, Object *b, unsigned int ntypes, ... );
    /* replace each occurrence of "find" in "source" with "replace" */
    static void replace( string &source, const string find, string replace );

    static void parse_string( string& s );

    H_OBJECT_TYPE    xtype;
    unsigned int     xsize;
    unsigned int     is_extern;

    long             xint;
    double           xfloat;
    char             xchar;
    string           xstring;
	vector<Object *> xarray;
	vector<Object *> xmap;
	unsigned int     xalias;

	unsigned int     xrows;
	unsigned int     xcolumns;
	Object       *** xmatrix;

	#ifdef MEM_DEBUG
    void* operator new (size_t size);
    #endif

    Object( long value );
    Object( long value, unsigned int _is_extern );
    Object( double value );
    Object( char value );
    Object( char *value );
	Object();
	Object( unsigned int value );
	Object( unsigned int rows, unsigned int columns, vector<Object *>& data );
    Object( Object *o );
    Object( FILE *fp );

    void release();

    ~Object();

	int equals( Object *o );

	int mapFind( Object *map );

	Object * getObject();

    void compile( FILE *fp );
    unsigned char *serialize();

    void print( unsigned int tabs = 0 );

    void println( unsigned int tabs = 0 );

    #ifdef XML_SUPPORT
	string toxml( unsigned int tabs = 0 );
    #endif

    void input();
	Object * toString();
    Object * toInt();

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
    Object * operator - ();
    Object * operator ! ();
    Object * factorial();
    Object&  operator ++ ();
    Object&  operator -- ();
    Object * operator + ( Object *o );
	Object * operator += ( Object *o );
    Object * operator - ( Object *o );
	Object * operator -= ( Object *o );
    Object * operator * ( Object *o );
	Object * operator *= ( Object *o );
    Object * operator / ( Object *o );
	Object * operator /= ( Object *o );
    Object * operator % ( Object *o );
	Object * operator %= ( Object *o );
    Object * operator & ( Object *o );
	Object * operator &= ( Object *o );
    Object * operator | ( Object *o );
	Object * operator |= ( Object *o );
	Object * operator << ( Object *o );
	Object * operator <<= ( Object *o );
	Object * operator >> ( Object *o );
	Object * operator >>= ( Object *o );
    Object * operator ^ ( Object *o );
	Object * operator ^= ( Object *o );
    Object * operator ~ ();
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

