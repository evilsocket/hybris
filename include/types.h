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

#include "gc.h"
#include <stdarg.h>
#include <string>
#include <vector>
#include <pcre.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

typedef unsigned char byte;

/*
 *  Helper macro to obtain the address of a pointer
 */
#ifndef H_ADDRESS_OF
#   define H_ADDRESS_OF(o) reinterpret_cast<ulong>(o)
#endif
/*
 *  Object memory attributes
 */
#define H_OA_NONE     0 // 00000000
#define H_OA_EXTERN   1 // 00000001
#define H_OA_CONSTANT 2 // 00000010
#define H_OA_GARBAGE  4 // 00000100
/*
 * PCRE support constants.
 */
#ifdef PCRE_SUPPORT
#   define H_PCRE_MAX_MATCHES   300

#   define H_PCRE_BOOL_MATCH    0x0
#   define H_PCRE_MULTI_MATCH   0x1
#   define H_PCRE_REPLACE_MATCH 0x2
#endif
/*
 * This macro define an object header elements.
 * Should be used in EVERY type declaration at the beginning
 * of the object structure itself to be down-casted to a base object.
 *
 * type       : type descriptor as pointer (for type checking)
 * ref        : reference counter (for garbage collection)
 * attributes : object memory attributes mask
 */
#define BASE_OBJECT_HEADER struct _object_type_t *type;       \
                           int                    ref;        \
                           size_t                 attributes
/*
 * Default object header initialization macro .
 */
#define BASE_OBJECT_HEADER_INIT(t) ref(0), \
                                   attributes(H_OA_GARBAGE), \
                                   type(&t ## _Type)
/*
 * This macro compare the object type structure pointer with a given
 * type, assuming that all type structure pointers are declared as :
 *      static Object {type}_Type = { ... }
 * to determine if the object it's of the given type .
 */
#define ob_is_typeof( o, t ) ((o)->type->code == (t ## _Type).code)
/*
 *  Determine whenever two objects are of the same type.
 */
#define ob_same_type( a, b ) ((a)->type->code == (b)->type->code)
/*
 * Macros to declare a type and then to implement its type structure.
 *
 * NOTE: The DECLARE_TYPE macro can be used inside type modules too
 * to use specific features of a type that it's being implemented
 * elsewhere in the code.
 *
 * For instance :
 *
 * DECLARE_TYPE(String);
 *
 * return String_Type.from_int(me);
 */
#define DECLARE_TYPE(t)   extern object_type_t t ## _Type
#define IMPLEMENT_TYPE(t) object_type_t t ## _Type =
/*
 * Macro to downcast any type structure to the base one.
 */
#define ob_dcast(o)      (Object *)(o)
/*
 * Pre declaration to implement basic object type .
 */
struct _object_type_t;
/*
 * Basic object used only to downcast more complex object types
 */
typedef struct _Object {
    BASE_OBJECT_HEADER;
}
Object;


/*
 * Type function pointers descriptors .
 */
// used to set garbage collection attributes for an object
typedef void     (*ob_set_references_function_t)( Object *o, int );
// free object inner buffer if any
typedef void     (*ob_free_function_t)          ( Object * );
// return the size of the object, or its items if it's a collection
typedef size_t   (*ob_size_function_t)          ( Object * );
// serialize the object, if size = 0 ignore it and take the object default size
typedef byte *   (*ob_serialize_function_t)     ( Object *, size_t );
// deserialize the object from a buffer
typedef Object * (*ob_deserialize_function_t)   ( Object *, byte *, size_t );
// write an object to a file descriptor
typedef Object * (*ob_to_fd_t)					( Object *, int, size_t );
// read an object from a file descriptor
typedef Object * (*ob_from_fd_t)				( Object *, int, size_t );
// {a} == {b} ?
typedef int      (*ob_cmp_function_t)           ( Object *, Object * );
// get the integer value rapresentation of the object
typedef long     (*ob_ivalue_function_t)        ( Object * );
// get the float value rapresentation of the object
typedef double   (*ob_fvalue_function_t)        ( Object * );
// get the logical value (true or false) rapresentation of the object
typedef bool     (*ob_lvalue_function_t)        ( Object * );
// get a string rapresentation of the object for interal use
typedef string   (*ob_svalue_function_t)        ( Object * );
// print the object to stdout
typedef void     (*ob_print_function_t)         ( Object *, int );
// read the object from stdin
typedef void     (*ob_scanf_function_t)         ( Object * );
// <operator>{a}
typedef Object * (*ob_unary_function_t)         ( Object * );
// {a}<operator>{b}
typedef Object * (*ob_binary_function_t)        ( Object *, Object * );
// {a}<operator{b} '=' {c}
typedef Object * (*ob_ternary_function_t)       ( Object *, Object *, Object * );
// functions to manage structure attributes
typedef void     (*ob_add_attribute_function_t) ( Object *, char * );
typedef Object * (*ob_get_attribute_function_t) ( Object *, char * );
typedef void     (*ob_set_attribute_function_t) ( Object *, char *, Object * );
/*
 * Object type codes enumeration
 */
enum H_OBJECT_TYPE {
    otVoid = 0,
    otInteger,
    otFloat,
    otChar,
    otString,
    otBinary,
    otVector,
    otMap,
    otMatrix,
    otStructure,
    otAlias,
    otExtern,
    otPointer
};
/*
 * Object type description structure .
 *
 * Each type implements its own functions to handle arithmetic, logical, ecc
 * operations or sets them to 0 (NULL) in case the type can't handle that
 * operation.
 * Any type function pointer shouldn't be used directly but with its relative
 * ob_* function to avoid sigsegvs on null function pointers.
 */
typedef struct _object_type_t {
    /** type code **/
    H_OBJECT_TYPE code;
    /** type name **/
    const char   *name;
    /** type basic size **/
    size_t        size;

    /** generic function pointers **/
    ob_set_references_function_t set_references;
    ob_unary_function_t         clone;
    ob_free_function_t          free;
    ob_size_function_t			get_size;
    ob_serialize_function_t     serialize;
    ob_deserialize_function_t   deserialize;
    ob_to_fd_t					to_fd;
    ob_from_fd_t				from_fd;
    ob_cmp_function_t           cmp;
    ob_ivalue_function_t        ivalue;
    ob_fvalue_function_t        fvalue;
    ob_lvalue_function_t        lvalue;
    ob_svalue_function_t        svalue;
    ob_print_function_t         print;
    ob_scanf_function_t         scanf;
    ob_unary_function_t         to_string;
    ob_unary_function_t         to_int;
    ob_unary_function_t         from_int;
    ob_unary_function_t         from_float;
    ob_binary_function_t        range;
    ob_binary_function_t        regexp;

    /** arithmetic operators **/
    ob_binary_function_t        assign;
    ob_unary_function_t         factorial;
    ob_unary_function_t         increment;
    ob_unary_function_t         decrement;
    ob_unary_function_t         minus;
    ob_binary_function_t        add;
    ob_binary_function_t        sub;
    ob_binary_function_t        mul;
    ob_binary_function_t        div;
    ob_binary_function_t        mod;
    ob_binary_function_t        inplace_add;
    ob_binary_function_t        inplace_sub;
    ob_binary_function_t        inplace_mul;
    ob_binary_function_t        inplace_div;
    ob_binary_function_t        inplace_mod;

	/** bitwise operators **/
    ob_binary_function_t        bw_and;
    ob_binary_function_t        bw_or;
    ob_unary_function_t         bw_not;
    ob_binary_function_t        bw_xor;
    ob_binary_function_t        bw_lshift;
    ob_binary_function_t        bw_rshift;
    ob_binary_function_t        bw_inplace_and;
    ob_binary_function_t        bw_inplace_or;
    ob_binary_function_t        bw_inplace_xor;
    ob_binary_function_t        bw_inplace_lshift;
    ob_binary_function_t        bw_inplace_rshift;

    /** logic operators **/
    ob_unary_function_t         l_not;
    ob_binary_function_t        l_same;
	ob_binary_function_t        l_diff;
    ob_binary_function_t        l_less;
    ob_binary_function_t        l_greater;
    ob_binary_function_t        l_less_or_same;
    ob_binary_function_t        l_greater_or_same;
    ob_binary_function_t        l_or;
    ob_binary_function_t        l_and;

    /** collection operators **/
    ob_binary_function_t        cl_concat;
    ob_binary_function_t        cl_inplace_concat;
    ob_binary_function_t        cl_push;
	ob_binary_function_t        cl_push_reference;
	ob_unary_function_t         cl_pop;
    ob_binary_function_t        cl_remove;
    ob_binary_function_t        cl_at;
    ob_ternary_function_t       cl_set;
    ob_ternary_function_t       cl_set_reference;

    /** structure operators **/
    ob_add_attribute_function_t add_attribute;
    ob_get_attribute_function_t get_attribute;
    ob_set_attribute_function_t set_attribute;
    ob_set_attribute_function_t set_attribute_reference;
}
object_type_t;

/*
 * Those are the function interfaces that will check for function availability
 * on each type, type compatibility, reference counting and so on .
 * They should be used on every access on type functions instead
 * of the function pointers directly.
 */
bool    ob_is_type_in( Object *o, ... );
void    ob_set_references( Object *o, int ref );
Object* ob_clone( Object *o );
bool    ob_free( Object *o );
size_t  ob_get_size( Object *o );
byte *  ob_serialize( Object *, size_t );
Object *ob_deserialize( Object *, byte *, size_t );
Object *ob_to_fd( Object *, int, size_t );
Object *ob_from_fd( Object *, int, size_t );
int     ob_cmp( Object *o, Object * cmp );
long    ob_ivalue( Object *o );
double  ob_fvalue( Object *o );
bool    ob_lvalue( Object *o );
string  ob_svalue( Object *o );
void    ob_print( Object *o, int tabs = 0 );
void    ob_input( Object *o );
Object *ob_to_string( Object *o );
Object *ob_to_int( Object *o );
Object *ob_from_int( Object *o );
Object *ob_from_float( Object *o );
Object *ob_range( Object *a, Object *b );
Object *ob_apply_regexp( Object *a, Object *b );
Object *ob_assign( Object *a, Object *b );
Object *ob_factorial( Object *o );
Object *ob_increment( Object *o );
Object *ob_decrement( Object *o );
Object *ob_uminus( Object *o );
Object *ob_add( Object *a, Object *b );
Object *ob_sub( Object *a, Object *b );
Object *ob_mul( Object *a, Object *b );
Object *ob_div( Object *a, Object *b );
Object *ob_mod( Object *a, Object *b );
Object *ob_inplace_add( Object *a, Object *b );
Object *ob_inplace_sub( Object *a, Object *b );
Object *ob_inplace_mul( Object *a, Object *b );
Object *ob_inplace_div( Object *a, Object *b );
Object *ob_inplace_mod( Object *a, Object *b );
Object *ob_bw_and( Object *a, Object *b );
Object *ob_bw_or( Object *a, Object *b );
Object *ob_bw_not( Object *o );
Object *ob_bw_xor( Object *a, Object *b );
Object *ob_bw_lshift( Object *a, Object *b );
Object *ob_bw_rshift( Object *a, Object *b );
Object *ob_bw_inplace_and( Object *a, Object *b );
Object *ob_bw_inplace_or( Object *a, Object *b );
Object *ob_bw_inplace_xor( Object *a, Object *b );
Object *ob_bw_inplace_lshift( Object *a, Object *b );
Object *ob_bw_inplace_rshift( Object *a, Object *b );
Object *ob_l_not( Object *o );
Object *ob_l_same( Object *a, Object *b );
Object *ob_l_diff( Object *a, Object *b );
Object *ob_l_less( Object *a, Object *b );
Object *ob_l_greater( Object *a, Object *b );
Object *ob_l_less_or_same( Object *a, Object *b );
Object *ob_l_greater_or_same( Object *a, Object *b );
Object *ob_l_or( Object *a, Object *b );
Object *ob_l_and( Object *a, Object *b );
Object *ob_cl_concat( Object *a, Object *b );
Object *ob_cl_inplace_concat( Object *a, Object *b );
Object *ob_cl_push( Object *a, Object *b );
Object *ob_cl_push_reference( Object *a, Object *b );
Object *ob_cl_pop( Object *o );
Object *ob_cl_remove( Object *a, Object *b );
Object *ob_cl_at( Object *a, Object *b );
Object *ob_cl_set( Object *a, Object *b, Object *c );
Object *ob_cl_set_reference( Object *a, Object *b, Object *c );
void    ob_add_attribute( Object *s, char *a );
Object *ob_get_attribute( Object *s, char *a );
void    ob_set_attribute( Object *s, char *a, Object *v );
void    ob_set_attribute_reference( Object *s, char *a, Object *v );

/**
 * Types definition.
 */
DECLARE_TYPE(Integer);

typedef struct _IntegerObject {
    BASE_OBJECT_HEADER;
    long value;

    _IntegerObject( long v ) : BASE_OBJECT_HEADER_INIT(Integer), value(v) {
    }
}
IntegerObject;
/*
 * Alias and Extern types, are basically Integers, but used as local function
 * pointer (the alias) and external (dynamically loaded dlls) functions
 * pointer (the extern).
 */
DECLARE_TYPE(Alias);

typedef struct _AliasObject {
    BASE_OBJECT_HEADER;
    long value;

    _AliasObject( long v ) : BASE_OBJECT_HEADER_INIT(Alias), value(v) {

    }
}
AliasObject;

DECLARE_TYPE(Extern);

typedef struct _ExternObject {
    BASE_OBJECT_HEADER;
    long value;

    _ExternObject( long v ) : BASE_OBJECT_HEADER_INIT(Extern), value(v) {

    }
}
ExternObject;

#define ob_is_int(o) 	   ob_is_typeof(o,Integer)
#define ob_is_alias(o)     ob_is_typeof(o,Alias)
#define ob_is_extern(o)    ob_is_typeof(o,Extern)
#define ob_int_ucast(o)    (IntegerObject *)(o)
#define ob_alias_ucast(o)  (AliasObject *)(o)
#define ob_extern_ucast(o) (ExternObject *)(o)
#define ob_int_val(o)      (((IntegerObject *)(o))->value)
#define ob_alias_val(o)    (((AliasObject *)(o))->value)
#define ob_extern_val(o)   (((ExternObject *)(o))->value)
/*
 * A macro to cast any pointer into an Integer representing its address.
 */
#define PTR_TO_INT_OBJ(p) gc_new_integer( H_ADDRESS_OF(p) )

DECLARE_TYPE(Float);

typedef struct _FloatObject {
    BASE_OBJECT_HEADER;
    double value;

    _FloatObject( double v ) : BASE_OBJECT_HEADER_INIT(Float), value(v) {

    }
}
FloatObject;

#define ob_is_float(o)    ob_is_typeof(o,Float)
#define ob_float_ucast(o) ((FloatObject *)(o))
#define ob_float_val(o)   (((FloatObject *)(o))->value)

DECLARE_TYPE(Char);

typedef struct _CharObject {
    BASE_OBJECT_HEADER;
    char value;

    _CharObject( char v ) : BASE_OBJECT_HEADER_INIT(Char), value(v) {

    }
}
CharObject;

#define ob_is_char(o)    ob_is_typeof(o,Char)
#define ob_char_ucast(o) ((CharObject *)(o))
#define ob_char_val(o)   ((CharObject *)(o))->value

DECLARE_TYPE(String);

void string_parse( string& s );
void string_replace( string &source, const string find, string replace );
void string_parse_pcre( string& raw, string& regex, int& opts );

typedef struct _StringObject {
    BASE_OBJECT_HEADER;
    size_t items;
    string value;

    _StringObject( char *v ) : items(0), BASE_OBJECT_HEADER_INIT(String), value(v) {
        string_parse(value);
        items = value.size();
    }
}
StringObject;

#define ob_is_string(o)    ob_is_typeof(o,String)
#define ob_string_ucast(o) ((StringObject *)(o))
#define ob_string_val(o)   ((StringObject *)(o))->value
#define ob_lpcstr_val(o)   (ob_string_val(o).c_str())

DECLARE_TYPE(Binary);

typedef struct _BinaryObject {
    BASE_OBJECT_HEADER;
    size_t           items;
    vector<Object *> value;

    _BinaryObject() : items(0), BASE_OBJECT_HEADER_INIT(Binary) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            value.reserve( RESERVE_VECTORS_SPACE );
        #endif
    }

    _BinaryObject( vector<unsigned char>& data ) : items(0), BASE_OBJECT_HEADER_INIT(Binary) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            value.reserve( RESERVE_VECTORS_SPACE );
        #endif
        vector<unsigned char>::iterator i;
        for( i = data.begin(); i != data.end(); i++ ){
            ob_cl_push_reference( (Object *)this, (Object *)gc_new_char(*i) );
        }
    }
}
BinaryObject;

typedef vector<Object *>::iterator BinaryObjectIterator;

#define ob_is_binary(o)    ob_is_typeof(o,Binary)
#define ob_binary_ucast(o) ((BinaryObject *)(o))

DECLARE_TYPE(Vector);

typedef struct _VectorObject {
    BASE_OBJECT_HEADER;
    size_t           items;
    vector<Object *> value;

    _VectorObject() : items(0), BASE_OBJECT_HEADER_INIT(Vector) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            value.reserve( RESERVE_VECTORS_SPACE );
        #endif
    }
}
VectorObject;

typedef vector<Object *>::iterator VectorObjectIterator;

#define ob_is_vector(o)    ob_is_typeof(o,Vector)
#define ob_vector_ucast(o) ((VectorObject *)(o))
#define ob_vector_val(o)   (((VectorObject *)(o)))

DECLARE_TYPE(Map);

int map_find( Object *m, Object *key );

typedef struct _MapObject {
    BASE_OBJECT_HEADER;
    size_t           items;
    vector<Object *> keys;
    vector<Object *> values;

    _MapObject() : items(0), BASE_OBJECT_HEADER_INIT(Map) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            keys.reserve( RESERVE_VECTORS_SPACE );
            values.reserve( RESERVE_VECTORS_SPACE );
        #endif
    }
}
MapObject;

typedef vector<Object *>::iterator MapObjectIterator;

#define ob_is_map(o)    ob_is_typeof(o,Map)
#define ob_map_ucast(o) ((MapObject *)(o))
#define ob_map_val(o)	(MapObject *)(o)

DECLARE_TYPE(Matrix);

typedef struct _MatrixObject {
    BASE_OBJECT_HEADER;
    size_t    items;
    size_t    rows;
    size_t    columns;
    Object ***matrix;

    _MatrixObject() : BASE_OBJECT_HEADER_INIT(Matrix), items(0), rows(0), columns(0) {

    }

    _MatrixObject( size_t _rows, size_t _columns ) : BASE_OBJECT_HEADER_INIT(Matrix) {
        size_t x, y;

        rows    = _rows;
        columns = _columns;
        items   = rows * columns;
        matrix  = new Object ** [rows];

        for( x = 0; x < rows; ++x ){
            matrix[x] = new Object * [columns];
        }

        for( x = 0; x < rows; ++x ){
            for( y = 0; y < columns; ++y ){
                matrix[x][y] = (Object *)&matrix;
            }
        }
    }

    _MatrixObject( size_t _rows, size_t _columns, vector<Object *>& data ) : BASE_OBJECT_HEADER_INIT(Matrix) {
        size_t x, y;

        rows    = _rows;
        columns = _columns;
        items   = rows * columns;
        matrix  = new Object ** [rows];

        for( x = 0; x < rows; ++x ){
            matrix[x] = new Object * [columns];
        }

        if( data.size() ){
            for( x = 0; x < rows; ++x ){
                for( y = 0; y < columns; ++y ){
                    Object * o   = data[ x * columns + y ];
                    matrix[x][y] = o;
                }
            }
        }
        else{
            for( x = 0; x < rows; ++x ){
                for( y = 0; y < columns; ++y ){
                    Object * o   = (Object *)gc_new_integer(0);
                    matrix[x][y] = o;
                }
            }
        }
    }
}
MatrixObject;

#define ob_is_matrix(o)      ob_is_typeof(o,Matrix)
#define ob_matrix_ucast(o)   ((MatrixObject *)(o))
#define ob_matrix_val(o)     ((MatrixObject *)o)

DECLARE_TYPE(Structure);

typedef struct _StructureObject {
    BASE_OBJECT_HEADER;
    size_t           items;
    vector<string>   names;
    vector<Object *> values;

    _StructureObject() : items(0), BASE_OBJECT_HEADER_INIT(Structure) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            names.reserve( RESERVE_VECTORS_SPACE );
            values.reserve( RESERVE_VECTORS_SPACE );
        #endif
    }
}
StructureObject;

typedef vector<string>::iterator   StructureObjectNameIterator;
typedef vector<Object *>::iterator StructureObjectValueIterator;

#define ob_is_struct(o)    ob_is_typeof(o,Structure)
#define ob_struct_ucast(o) ((StructureObject *)(o))

#endif

