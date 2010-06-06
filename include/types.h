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

#include "common.h"
#include "gc.h"
#include "itree.h"
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
 * Pre declarations of useful types.
 */
class Node;
class MemorySegment;
typedef struct _vm_t 	 vm_t;
typedef MemorySegment vmem_t;
typedef MemorySegment vframe_t;

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
#define H_OA_CONSTANT 1 // 00000001

/*
 * This macro define an object header elements.
 * Should be used in EVERY type declaration at the beginning
 * of the object structure itself to be down-casted to a base object.
 *
 * type       : type descriptor as pointer (for type checking)
 * referenced : tell the vm to use a reference to this object instead of a clone
 * gc_size	  : size in bytes of the entire object
 * gc_mark    : mark-&-sweep gc flag
 * gc_count	  : number of times the object passed the garbage collection
 * attributes : object memory attributes mask
 */
#define BASE_OBJECT_HEADER struct _object_type_t *type;       \
						   bool					  referenced; \
						   size_t				  gc_size;    \
                           bool                   gc_mark;    \
						   size_t				  gc_count;   \
                           size_t                 attributes
/*
 * Default object header initialization macro .
 */
#define BASE_OBJECT_HEADER_INIT(t) gc_mark(false), \
								   referenced(false), \
								   gc_size(0), \
								   gc_count(0), \
                                   attributes(H_OA_NONE), \
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
 * Macros to assert an object type.
 */
#define ob_type_assert(o,t,f)      if( o->type->code != t ){ \
                                    hyb_error( H_ET_SYNTAX, "Unexpected '%s' variable for function " f ", expected '%s'", o->type->name, ob_type_to_string(t) ); \
                                 }
#define ob_types_assert(o,t1,t2,f) if( o->type->code != t1 && o->type->code != t2 ){ \
									   hyb_error( H_ET_SYNTAX, "Unexpected '%s' variable for function " f ", expected '%s' or '%s'", o->type->name, ob_type_to_string(t1), ob_type_to_string(t2) ); \
                                   }

#define ob_argv_type_assert( i, t, f ) 		 ob_type_assert( vm_argv(i), t, f )
#define ob_argv_types_assert( i, t1, t2, f ) ob_types_assert( vm_argv(i), t1, t2, f )

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
 * Type checking and casting macros.
 */
#define ob_is_boolean(o)    ob_is_typeof(o,Boolean)
#define ob_bool_ucast(o)    ((Boolean *)(o))
#define ob_bool_val(o)      (((Boolean *)(o))->value)
#define ob_is_int(o) 	    ob_is_typeof(o,Integer)
#define ob_is_alias(o)      ob_is_typeof(o,Alias)
#define ob_is_extern(o)     ob_is_typeof(o,Extern)
#define ob_int_ucast(o)     (Integer *)(o)
#define ob_alias_ucast(o)   (Alias *)(o)
#define ob_extern_ucast(o)  (Extern *)(o)
#define ob_int_val(o)       (((Integer *)(o))->value)
#define ob_alias_val(o)     (((Alias *)(o))->value)
#define ob_extern_val(o)    (((Extern *)(o))->value)
#define ob_is_float(o)      ob_is_typeof(o,Float)
#define ob_float_ucast(o)   ((Float *)(o))
#define ob_float_val(o)     (((Float *)(o))->value)
#define ob_is_char(o)       ob_is_typeof(o,Char)
#define ob_char_ucast(o)    ((Char *)(o))
#define ob_char_val(o)      ((Char *)(o))->value
#define ob_is_string(o)     ob_is_typeof(o,String)
#define ob_string_ucast(o)  ((String *)(o))
#define ob_string_val(o)    ((String *)(o))->value
#define ob_lpcstr_val(o)    (ob_string_val(o).c_str())
#define ob_is_binary(o)     ob_is_typeof(o,Binary)
#define ob_binary_ucast(o)  ((Binary *)(o))
#define ob_is_vector(o)     ob_is_typeof(o,Vector)
#define ob_vector_ucast(o)  ((Vector *)(o))
#define ob_vector_val(o)    (((Vector *)(o)))
#define ob_is_map(o)        ob_is_typeof(o,Map)
#define ob_map_ucast(o)     ((Map *)(o))
#define ob_map_val(o)	    (Map *)(o)
#define ob_is_struct(o)     ob_is_typeof(o,Structure)
#define ob_struct_ucast(o)  ((Structure *)(o))
#define ob_is_class(o)      ob_is_typeof(o,Class)
#define ob_class_ucast(o)   ((Class *)(o))
#define ob_class_val(o)     ((Class *)o)
#define ob_is_reference(o)  ob_is_typeof(o,Reference)
#define ob_ref_ucast(o)     ((Reference *)(o))
#define ob_reference_val(o) ((Reference *)o)
#define ob_is_handle(o)     ob_is_typeof(o,Handle)
#define ob_handle_ucast(o)  ((Handle *)(o))
#define ob_handle_val(o)    ((Handle *)o)->value
/*
 * A macro to cast any pointer into an Integer representing its address.
 */
#define PTR_TO_INT_OBJ(p) gc_new_integer( H_ADDRESS_OF(p) )
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
 * Function pointer prototype for objects builtin methods (if any).
 */
typedef Object * (*ob_type_builtin_method_t)( vm_t *, Object *, vframe_t * );
/*
 * Type builtin methods map.
 */
typedef struct {
	string 					  name;
	ob_type_builtin_method_t *method;
}
ob_builtin_method_t;
/*
 * End marker for builtin methods map.
 */
#define OB_BUILIN_METHODS_END_MARKER { "", NULL }
/*
 * Class for builtin methods lookup.
 */
class ob_builtin_methods_t : public ITree<ob_type_builtin_method_t> {
	public :

		INLINE ob_builtin_methods_t( ob_builtin_method_t methods[] ) : ITree<ob_type_builtin_method_t>() {
			size_t i;
			for( i = 0; methods[i].method != NULL; ++i ){
				insert( (char *)methods[i].name.c_str(), methods[i].method );
			}
		}

};
/*
 * Static array used for types with no builtin methods.
 */
static ob_builtin_method_t NO_BUILTIN_METHODS[] = { OB_BUILIN_METHODS_END_MARKER };

#ifndef H_ACCESS_SPECIFIER
	enum access_t {
		asPublic = 0,
		asPrivate,
		asProtected
	};
#	define H_ACCESS_SPECIFIER
#endif

/*
 * Type function pointers descriptors .
 */
// get the type name of the object
typedef const char *(*ob_typename_function_t)	( Object * );
// get the n-th object referenced by this
typedef Object * (*ob_traverse_function_t)      ( Object *, int );
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
// functions to manage structure and class attributes
typedef void     (*ob_define_attribute_function_t) ( Object *, char *, access_t, bool );
typedef access_t (*ob_attribute_access_function_t) ( Object *, char * );
typedef bool	 (*ob_attribute_is_static_function_t) ( Object *, char * );
typedef void     (*ob_set_attribute_access_function_t) ( Object *, char *, access_t );
typedef void     (*ob_add_attribute_function_t) ( Object *, char * );
typedef Object * (*ob_get_attribute_function_t) ( Object *, char *, bool );
typedef void     (*ob_set_attribute_function_t) ( Object *, char *, Object * );
// functions to manage class methods
typedef void     (*ob_define_method_function_t) ( Object *, char *, Node * );
typedef Node   * (*ob_get_method_function_t)	( Object *, char *, int );
typedef Object * (*ob_call_method_function_t)	( vm_t *, vframe_t *, Object *, char *, char *, Node * );
/*
 * Object type codes enumeration.
 * otEndMarker is used to mark the last allowed type in
 * vm_function_t::types.
 */
enum H_OBJECT_TYPE {
	otEndMarker = -1,
    otVoid = 0,
    otBoolean,
    otInteger,
    otFloat,
    otChar,
    otString,
    otBinary,
    otVector,
    otMap,
    otAlias,
    otExtern,
    otHandle,
    otStructure,
    otClass,
    otReference
};

/*
 * Set the object_type_t::size field to zero for collections, the
 * ob_get_size will return the number of elements instead the field.
 */
#define OB_COLLECTION_SIZE 0

INLINE const char *ob_type_to_string( H_OBJECT_TYPE type ){
	switch(type){
		case otVoid      : return "void";
		case otBoolean   : return "boolean";
		case otInteger   : return "integer";
		case otFloat     : return "float";
		case otChar      : return "char";
		case otString    : return "string";
		case otBinary    : return "binary";
		case otVector    : return "vector";
		case otMap       : return "map";
		case otAlias     : return "alias";
		case otExtern    : return "extern";
		case otHandle    : return "handle";
		case otStructure : return "structure";
		case otClass     : return "class";
		case otReference : return "reference";
	}
	/*
	 * We should never get here!
	 */
	assert(false);
}

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
    /*
     * Type code among H_OBJECT_TYPE enumeration values used
     * for type comparision.
     */
    H_OBJECT_TYPE 			    code;
    /*
     * Type name.
     */
    const char  			   *name;
    /*
     * Type basic size, if the object is a collection size is set to 0
     * and ob_get_size will return the number of its elements.
     */
    size_t        				size;
    /*
     * Type builtin methods map.
     */
    ob_builtin_methods_t 		builtin_methods;

    /** generic function pointers **/
    ob_typename_function_t      type_name;
    ob_traverse_function_t		traverse;
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
    ob_binary_function_t        cl_push;
	ob_binary_function_t        cl_push_reference;
	ob_unary_function_t         cl_pop;
    ob_binary_function_t        cl_remove;
    ob_binary_function_t        cl_at;
    ob_ternary_function_t       cl_set;
    ob_ternary_function_t       cl_set_reference;

    /** structure and class operators **/
    ob_define_attribute_function_t define_attribute;
    ob_attribute_access_function_t attribute_access;
    ob_attribute_is_static_function_t attribute_is_static;
    ob_set_attribute_access_function_t set_attribute_access;
    ob_add_attribute_function_t add_attribute;
    ob_get_attribute_function_t get_attribute;
    ob_set_attribute_function_t set_attribute;
    ob_set_attribute_function_t set_attribute_reference;

    /** class operators **/
    ob_define_method_function_t define_method;
    ob_get_method_function_t    get_method;
    ob_call_method_function_t   call_method;
}
object_type_t;

/**
 * Those are the function interfaces that will check for function availability
 * on each type, type compatibility, reference counting and so on .
 * They MUST be used on every access on type functions instead of the function pointers directly.
 */

/*
 * Macro to easily get a builtin method function pointer, given its name,
 * from the inner ITree each type has .
 */
#define ob_get_builtin_method( obj, method_id ) obj->type->builtin_methods.find(method_id)
/*
 * Return the type name of the object.
 */
const char *ob_typename( Object * o );
/*
 * Get the n-th object referenced by 'o'.
 */
Object *ob_traverse( Object *o, int index );
/*
 * Create a clone of the object.
 */
Object* ob_clone( Object *o );
/*
 * Eventually free object inner elements (for colletions) and decrement its reference counter.
 */
bool    ob_free( Object *o );
/*
 * Return the size of the object or, in case it's a collection, the number of its elements.
 */
size_t  ob_get_size( Object *o );
/*
 * Serialize the object to a binary stream.
 */
byte *  ob_serialize( Object *, size_t );
/*
 * Deserialize the object from a binary stream.
 */
Object *ob_deserialize( Object *, byte *, size_t );
/*
 * Write 'size' bytes of the object to a generic file descriptor, otherwise if
 * size is not specified (size = 0), write it all.
 */
Object *ob_to_fd( Object *, int, size_t );
/*
 * Read 'size' bytes from a generic file descriptor and create an object, if
 * size is not specified (size = 0), read 'ob_get_size' bytes or until a newline
 * byte is reached if the object is a string.
 */
Object *ob_from_fd( Object *, int, size_t );
/*
 * Compare two objects, return :
 *
 * 0  if o == cmp
 * 1  if o >  cmp
 * -1 if o < cmp
 */
int     ob_cmp( Object *o, Object * cmp );
/*
 * Get the integer representation of an object.
 */
long    ob_ivalue( Object *o );
/*
 * Get the float representation of an object.
 */
double  ob_fvalue( Object *o );
/*
 * Get the logical representation of an object, if object is a collection
 * return ob_get_size(0) != 0.
 */
bool    ob_lvalue( Object *o );
/*
 * Get the string representation of an object.
 */
string  ob_svalue( Object *o );
/*
 * Print an object to stdout preceeded by 'tabs' tabs.
 */
void    ob_print( Object *o, int tabs = 0 );
/*
 * Read from stdin and create an object accordingly to the format.
 */
void    ob_input( Object *o );
/*
 * Convert an object to string type.
 */
Object *ob_to_string( Object *o );
/*
 * Convert an object to integer type.
 */
Object *ob_to_int( Object *o );
/*
 * Create a vector representing the range [a,b].
 */
Object *ob_range( Object *a, Object *b );
/*
 * Apply a regular expression 'b' to the object 'a' and return
 * matches with the operator a ~= b.
 */
Object *ob_apply_regexp( Object *a, Object *b );
/*
 * Clone the object 'b', increment its references, call ob_free(a) and
 * assign the clone to 'a'.
 */
Object *ob_assign( Object *a, Object *b );
/*
 * Compute the factorial of the object.
 */
Object *ob_factorial( Object *o );
/*
 * Increment the object value by 1.
 */
Object *ob_increment( Object *o );
/*
 * Decrement the object value by 1.
 */
Object *ob_decrement( Object *o );
/*
 * Return -o.
 */
Object *ob_uminus( Object *o );
/*
 * Compute a + b.
 */
Object *ob_add( Object *a, Object *b );
/*
 * Compute a - b.
 */
Object *ob_sub( Object *a, Object *b );
/*
 * Compute a * b.
 */
Object *ob_mul( Object *a, Object *b );
/*
 * Compute a / b.
 */
Object *ob_div( Object *a, Object *b );
/*
 * Compute a % b.
 */
Object *ob_mod( Object *a, Object *b );
/*
 * Compute a += b.
 */
Object *ob_inplace_add( Object *a, Object *b );
/*
 * Compute a -= b.
 */
Object *ob_inplace_sub( Object *a, Object *b );
/*
 * Compute a *= b.
 */
Object *ob_inplace_mul( Object *a, Object *b );
/*
 * Compute a /= b.
 */
Object *ob_inplace_div( Object *a, Object *b );
/*
 * Compute a %= b.
 */
Object *ob_inplace_mod( Object *a, Object *b );
/*
 * Compute a & b.
 */
Object *ob_bw_and( Object *a, Object *b );
/*
 * Compute a | b.
 */
Object *ob_bw_or( Object *a, Object *b );
/*
 * Compute ~o.
 */
Object *ob_bw_not( Object *o );
/*
 * Compute a ^ b.
 */
Object *ob_bw_xor( Object *a, Object *b );
/*
 * Compute a << b.
 */
Object *ob_bw_lshift( Object *a, Object *b );
/*
 * Compute a >> b.
 */
Object *ob_bw_rshift( Object *a, Object *b );
/*
 * Compute a &= b.
 */
Object *ob_bw_inplace_and( Object *a, Object *b );
/*
 * Compute a |= b.
 */
Object *ob_bw_inplace_or( Object *a, Object *b );
/*
 * Compute a ^= b.
 */
Object *ob_bw_inplace_xor( Object *a, Object *b );
/*
 * Compute a <<= b.
 */
Object *ob_bw_inplace_lshift( Object *a, Object *b );
/*
 * Compute a >>= b.
 */
Object *ob_bw_inplace_rshift( Object *a, Object *b );
/*
 * Compute !o.
 */
Object *ob_l_not( Object *o );
/*
 * Compute a == b.
 */
Object *ob_l_same( Object *a, Object *b );
/*
 * Compute a != b.
 */
Object *ob_l_diff( Object *a, Object *b );
/*
 * Compute a < b.
 */
Object *ob_l_less( Object *a, Object *b );
/*
 * Compute a > b.
 */
Object *ob_l_greater( Object *a, Object *b );
/*
 * Compute a <= b.
 */
Object *ob_l_less_or_same( Object *a, Object *b );
/*
 * Compute a >= b.
 */
Object *ob_l_greater_or_same( Object *a, Object *b );
/*
 * Compute a || b.
 */
Object *ob_l_or( Object *a, Object *b );
/*
 * Compute a && b.
 */
Object *ob_l_and( Object *a, Object *b );
/*
 * Create a clone of 'b' and call ob_cl_push_reference( a, clone ) to
 * push the clone inside the collection 'a'.
 */
Object *ob_cl_push( Object *a, Object *b );
/*
 * Push the object 'b' inside the collection 'a', incrementing the
 * reference counter of 'b'.
 */
Object *ob_cl_push_reference( Object *a, Object *b );
/*
 * Remove the last element from the collection 'o' and return it.
 */
Object *ob_cl_pop( Object *o );
/*
 * Remove the element at index/key 'b' from the collection 'a' and return it.
 */
Object *ob_cl_remove( Object *a, Object *b );
/*
 * Get the item at index/key 'b' from the collection 'a'.
 */
Object *ob_cl_at( Object *a, Object *b );
/*
 * Create a clone of 'c' and call ob_cl_set_reference( a, b, clone ) to
 * put the clone at the index/key 'b' of the collection 'a'.
 */
Object *ob_cl_set( Object *a, Object *b, Object *c );
/*
 * Put the object 'c' at the index/key 'b' inside the collection 'a' and
 * increment 'c' reference counter.
 */
Object *ob_cl_set_reference( Object *a, Object *b, Object *c );
/*
 * Return the access level for a given attribute.
 */
access_t ob_attribute_access( Object *o, char * a );
/*
 * Return true if the given attribute exists and it's static,
 * otherwise return false.
 */
bool	 ob_attribute_is_static( Object *o, char *a );
/*
 * Define a new attribute inside the structure or the class.
 */
void     ob_define_attribute( Object *o, char *name, access_t a, bool is_static = false );
/*
 * Set the access level for a given attribute.
 */
void     ob_set_attribute_access( Object *o, char *name, access_t a );
/*
 * Define a new attribute 'a' for the structure 's'.
 */
void    ob_add_attribute( Object *s, char *a );
/*
 * Get the value of the attribute 'a' from the structure 's'.
 */
Object *ob_get_attribute( Object *s, char *a, bool with_descriptor = true );
/*
 * Create a clone of 'v' and call ob_set_attribute_reference( s, a, clone )
 * to set the value of the attribute 'a' inside the structure 's'.
 */
void    ob_set_attribute( Object *s, char *a, Object *v );
/*
 * Set the value of the attribute 'a' inside the structure 's' and increment
 * 'v' reference counter.
 */
void    ob_set_attribute_reference( Object *s, char *a, Object *v );
/*
 * Define the method 'name' with give 'code' inside the 'c' class.
 */
void    ob_define_method( Object *c, char *name, Node *code );
/*
 * Get the method 'name' from the 'c' class with 'argc' arguments.
 * If argc is not -1, the function will search for the "best match"
 * among methods with the same name and returns it, otherwise will
 * return the first method found.
 */
Node   *ob_get_method( Object *c, char *name, int argc = -1 );
/*
 * Execute a class method.
 */
Object *ob_call_method( vm_t *vm, vframe_t *frame, Object *owner, char *owner_id, char *method_id, Node *argv );
/**
 * Types definition.
 */
DECLARE_TYPE(Boolean);

typedef struct _Boolean {
    BASE_OBJECT_HEADER;
    bool value;

    _Boolean( bool v ) : BASE_OBJECT_HEADER_INIT(Boolean), value(v) {

    }
}
Boolean;

DECLARE_TYPE(Integer);

typedef struct _Integer {
    BASE_OBJECT_HEADER;
    long value;

    _Integer( long v ) : BASE_OBJECT_HEADER_INIT(Integer), value(v) {
    }
}
Integer;
/*
 * Alias and Extern types, are basically Integers, but used as local function
 * pointer (the alias) and external (dynamically loaded dlls) functions
 * pointer (the extern).
 */
DECLARE_TYPE(Alias);

typedef struct _Alias {
    BASE_OBJECT_HEADER;
    long value;

    _Alias( long v ) : BASE_OBJECT_HEADER_INIT(Alias), value(v) {

    }
}
Alias;

DECLARE_TYPE(Extern);

typedef struct _Extern {
    BASE_OBJECT_HEADER;
    long value;

    _Extern( long v ) : BASE_OBJECT_HEADER_INIT(Extern), value(v) {

    }
}
Extern;

DECLARE_TYPE(Float);

typedef struct _Float {
    BASE_OBJECT_HEADER;
    double value;

    _Float( double v ) : BASE_OBJECT_HEADER_INIT(Float), value(v) {

    }
}
Float;

DECLARE_TYPE(Char);

typedef struct _Char {
    BASE_OBJECT_HEADER;
    char value;

    _Char( char v ) : BASE_OBJECT_HEADER_INIT(Char), value(v) {

    }
}
Char;

DECLARE_TYPE(String);

size_t string_replace( string &source, const string find, string replace );
void   string_parse_pcre( string& raw, string& regex, int& opts );

typedef struct _String {
    BASE_OBJECT_HEADER;
    size_t items;
    string value;

    _String( char *v ) : items(0), BASE_OBJECT_HEADER_INIT(String) {
    	value = v;
        items = value.size();
    }
}
String;

DECLARE_TYPE(Binary);

typedef struct _Binary {
    BASE_OBJECT_HEADER;
    size_t           items;
    vector<Object *> value;

    _Binary() : items(0), BASE_OBJECT_HEADER_INIT(Binary) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            value.reserve( RESERVE_VECTORS_SPACE );
        #endif
    }

    _Binary( vector<unsigned char>& data ) : items(0), BASE_OBJECT_HEADER_INIT(Binary) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            value.reserve( RESERVE_VECTORS_SPACE );
        #endif
        vector<unsigned char>::iterator i;
        vv_foreach( vector<unsigned char>, i, data ){
            ob_cl_push_reference( (Object *)this, (Object *)gc_new_char(*i) );
        }
    }
}
Binary;

typedef vector<Object *>::iterator BinaryIterator;

DECLARE_TYPE(Vector);

typedef struct _Vector {
    BASE_OBJECT_HEADER;
    size_t           items;
    vector<Object *> value;

    _Vector() : items(0), BASE_OBJECT_HEADER_INIT(Vector) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            value.reserve( RESERVE_VECTORS_SPACE );
        #endif
    }
}
Vector;

typedef vector<Object *>::iterator VectorIterator;

DECLARE_TYPE(Map);

int map_find( Object *m, Object *key );

typedef struct _Map {
    BASE_OBJECT_HEADER;
    size_t           items;
    vector<Object *> keys;
    vector<Object *> values;

    _Map() : items(0), BASE_OBJECT_HEADER_INIT(Map) {
        // define to test space reservation optimization
        #ifdef RESERVED_VECTORS_SPACE
            keys.reserve( RESERVE_VECTORS_SPACE );
            values.reserve( RESERVE_VECTORS_SPACE );
        #endif
    }
}
Map;

typedef vector<Object *>::iterator MapIterator;

DECLARE_TYPE(Structure);

typedef struct _Structure {
    BASE_OBJECT_HEADER;
    size_t         items;
    ITree<Object>  s_attributes;

    _Structure() : items(0), BASE_OBJECT_HEADER_INIT(Structure) {

    }
}
Structure;

typedef ITree<Object>::iterator StructureAttributeIterator;

DECLARE_TYPE(Class);

typedef struct _class_attribute_t {
	string	 		name;
	bool	 	    is_static;
	access_t 	    access;
	Object  	   *value;
	pthread_mutex_t mutex;

	_class_attribute_t( string n, access_t a, Object *v, bool _static = false ) :
		name(n),
		access(a),
		value(v),
		is_static(_static),
		mutex(PTHREAD_MUTEX_INITIALIZER){

	}

	INLINE void lock(){
		if( is_static ){ pthread_mutex_lock(&mutex); }
	}

	INLINE void unlock(){
		if( is_static ){ pthread_mutex_unlock(&mutex); }
	}
}
class_attribute_t;

typedef vector<Node *> prototypes_t;

typedef struct _class_method_t {
	string		 name;
	/*
	 * A class could have more methods with the same name but
	 * different parameters, so we have to hold a vector of Nodes.
	 */
	prototypes_t prototypes;

	_class_method_t( string n, Node *m ) : name(n) {
		prototypes.push_back(m);
	}

	_class_method_t( string n, prototypes_t& p ) : name(n), prototypes(p) {
	}
}
class_method_t;

typedef struct _Class {
    BASE_OBJECT_HEADER;
    string name;

    ITree<class_attribute_t> c_attributes;
    ITree<class_method_t>	 c_methods;

    _Class() : BASE_OBJECT_HEADER_INIT(Class) {

    }
}
Class;

typedef ITree<class_attribute_t>::iterator ClassAttributeIterator;
typedef ITree<class_method_t>::iterator	   ClassMethodIterator;
typedef vector<Node *>::iterator	 	   ClassPrototypesIterator;

DECLARE_TYPE(Reference);

typedef struct _Reference {
    BASE_OBJECT_HEADER;
    Object *value;

    _Reference( Object *v ) : BASE_OBJECT_HEADER_INIT(Reference), value(v) {

    }
}
Reference;

DECLARE_TYPE(Handle);

typedef struct _Handle {
    BASE_OBJECT_HEADER;
    void *value;

    _Handle( void *v ) : BASE_OBJECT_HEADER_INIT(Handle), value(v) {

    }
}
Handle;
/*
 * Inline handlers implementation
 */
INLINE const char *ob_typename( Object * o ){
	return (o->type->type_name ? o->type->type_name(o) : o->type->name);
}

INLINE Object *ob_traverse( Object *o, int index ){
	#ifdef GC_DEBUG
		fprintf( stdout, "[GC DEBUG] Traversing object at %p, index %d.\n", o, index );
	#endif
	return (o->type->traverse ? o->type->traverse(o,index) : NULL);
}

INLINE Object* ob_clone( Object *o ){
    /*
	 * Every object has to implement its own clone.
	 */
	return o->type->clone(o);
}

INLINE bool ob_free( Object *o ){
    if( o->type->free != NULL ){
    	/*
    	 * The free function is defined only for collection types or
    	 * types that handles pointers anyway.
    	 */
        o->type->free(o);

        return true;
    }

    return false;
}

INLINE size_t ob_get_size( Object *o ){
	return (o->type->get_size ? o->type->get_size(o) : o->type->size);
}

INLINE byte * ob_serialize( Object *o, size_t size ){
	if( o->type->serialize != NULL ){
		return o->type->serialize(o,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't serialize '%s'", ob_typename(o) );
}

INLINE Object *ob_deserialize( Object *o, byte *buffer, size_t size ){
	if( o->type->deserialize != NULL ){
		return o->type->deserialize(o,buffer,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't deserialize '%s'", ob_typename(o) );
}

INLINE Object *ob_to_fd( Object *o, int fd, size_t size ){
	if( o->type->to_fd != NULL ){
		return o->type->to_fd(o,fd,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't write object '%s' to file descriptor", ob_typename(o) );
}

INLINE Object *ob_from_fd( Object *o, int fd, size_t size ){
	if( o->type->from_fd != NULL ){
		return o->type->from_fd(o,fd,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't read object '%s' from file descriptor", ob_typename(o) );
}

INLINE int ob_cmp( Object *o, Object * cmp ){
    if( o->type->cmp != NULL ){
        return o->type->cmp(o,cmp);
    }
    hyb_error( H_ET_SYNTAX, "couldn't compare '%s' object with '%s' object", ob_typename(o), ob_typename(cmp) );
}

INLINE long ob_ivalue( Object * o ){
    if( ob_is_int(o) ){
        return (ob_int_ucast(o))->value;
    }
    else if( o->type->ivalue != NULL ){
        return o->type->ivalue(o);
    }
    else if( o->type->lvalue != NULL ){
        return (long)o->type->lvalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the integer value of type '%s'", ob_typename(o) );
    }
    return 0;
}

INLINE double ob_fvalue( Object *o ){
    if( ob_is_float(o) ){
        return ob_float_ucast(o)->value;
    }
    else if( o->type->fvalue != NULL ){
        return o->type->fvalue(o);
    }
    else if( o->type->ivalue != NULL ){
        return (double)o->type->ivalue(o);
    }
    else if( o->type->lvalue != NULL ){
        return (double)o->type->lvalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the float value of type '%s'", ob_typename(o) );
    }
    return 0.0;
}

INLINE bool ob_lvalue( Object *o ){
    if( o->type->lvalue != NULL ){
        return o->type->lvalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the logical value of the object type '%s'", ob_typename(o) );
    }
}

INLINE string ob_svalue( Object *o ){
    if( o->type->svalue != NULL ){
        return o->type->svalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the string rapresentation of the object type '%s'", ob_typename(o) );
    }
}

INLINE void ob_print( Object *o, int tabs /*= 0*/ ){
    if( o->type->print != NULL ){
        return o->type->print(o,tabs);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't print the object type '%s'", ob_typename(o) );
    }
}

INLINE void ob_input( Object *o ){
    if( o->type->scanf != NULL ){
        return o->type->scanf(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't read the object type '%s' from stdin", ob_typename(o) );
    }
}

INLINE Object *ob_to_string( Object *o ){
	if( o->type->to_string != NULL ){
		return o->type->to_string(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' to string", ob_typename(o) );
	}
}

INLINE Object *ob_to_int( Object *o ){
	if( o->type->to_int != NULL ){
		return o->type->to_int(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' to int", ob_typename(o) );
	}
}

INLINE Object *ob_range( Object *a, Object *b ){
	if( a->type->range != NULL ){
		return a->type->range(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '..' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_apply_regexp( Object *a, Object *b ){
	if( a->type->regexp != NULL ){
		return a->type->regexp(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '~=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_assign( Object *a, Object *b ){
	/*
	 * Every object has to implement its own assign.
     *
     * The assign will call :
     *
     * 		ob_free(a)  --> a->ref--
	 */
	return a->type->assign(a,b);
}

INLINE Object *ob_factorial( Object *o ){
	if( o->type->factorial != NULL ){
		return o->type->factorial(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!' operator for object type '%s'", ob_typename(o) );
	}
}

INLINE Object *ob_increment( Object *o ){
	if( o->type->increment != NULL ){
		return o->type->increment(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '++' operator for object type '%s'", ob_typename(o) );
	}
}

INLINE Object *ob_decrement( Object *o ){
	if( o->type->decrement != NULL ){
		return o->type->decrement(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '--' operator for object type '%s'", ob_typename(o) );
	}
}

INLINE Object *ob_uminus( Object *o ){
	if( o->type->minus != NULL ){
		return o->type->minus(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-' operator for object type '%s'", ob_typename(o) );
	}
}

INLINE Object *ob_add( Object *a, Object *b ){
	if( a->type->add != NULL ){
		return a->type->add(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '+' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_sub( Object *a, Object *b ){
	if( a->type->sub != NULL ){
		return a->type->sub(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_mul( Object *a, Object *b ){
	if( a->type->mul != NULL ){
		return a->type->mul(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '*' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_div( Object *a, Object *b ){
	if( a->type->div != NULL ){
		return a->type->div(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '/' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_mod( Object *a, Object *b ){
	if( a->type->mod != NULL ){
		return a->type->mod(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '%' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_inplace_add( Object *a, Object *b ){
	if( a->type->inplace_add != NULL ){
		return a->type->inplace_add(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '+=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_inplace_sub( Object *a, Object *b ){
	if( a->type->inplace_sub != NULL ){
		return a->type->inplace_sub(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_inplace_mul( Object *a, Object *b ){
	if( a->type->inplace_mul != NULL ){
		return a->type->inplace_mul(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '*=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_inplace_div( Object *a, Object *b ){
	if( a->type->inplace_div != NULL ){
		return a->type->inplace_div(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '/=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_inplace_mod( Object *a, Object *b ){
	if( a->type->inplace_mod != NULL ){
		return a->type->inplace_mod(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '%=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_and( Object *a, Object *b ){
	if( a->type->bw_and != NULL ){
		return a->type->bw_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_or( Object *a, Object *b ){
	if( a->type->bw_or != NULL ){
		return a->type->bw_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '|' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_not( Object *o ){
	if( o->type->bw_not != NULL ){
		return o->type->bw_not(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '~' operator for object type '%s'", ob_typename(o) );
	}
}

INLINE Object *ob_bw_xor( Object *a, Object *b ){
	if( a->type->bw_xor != NULL ){
		return a->type->bw_xor(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '^' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_lshift( Object *a, Object *b ){
	if( a->type->bw_lshift != NULL ){
		return a->type->bw_lshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<<' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_rshift( Object *a, Object *b ){
	if( a->type->bw_rshift != NULL ){
		return a->type->bw_rshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>>' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_inplace_and( Object *a, Object *b ){
	if( a->type->bw_inplace_and != NULL ){
		return a->type->bw_inplace_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_inplace_or( Object *a, Object *b ){
	if( a->type->bw_inplace_or != NULL ){
		return a->type->bw_inplace_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '|=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_inplace_xor( Object *a, Object *b ){
	if( a->type->bw_inplace_xor != NULL ){
		return a->type->bw_inplace_xor(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '^=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_inplace_lshift( Object *a, Object *b ){
	if( a->type->bw_inplace_lshift != NULL ){
		return a->type->bw_inplace_lshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<<=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_bw_inplace_rshift( Object *a, Object *b ){
	if( a->type->bw_inplace_rshift != NULL ){
		return a->type->bw_inplace_rshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>>=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_not( Object *o ){
	if( o->type->l_not != NULL ){
		return o->type->l_not(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!' operator for object type '%s'", ob_typename(o) );
	}
}

INLINE Object *ob_l_same( Object *a, Object *b ){
	if( a->type->l_same != NULL ){
		return a->type->l_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '==' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_diff( Object *a, Object *b ){
	if( a->type->l_diff != NULL ){
		return a->type->l_diff(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_less( Object *a, Object *b ){
	if( a->type->l_less != NULL ){
		return a->type->l_less(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_greater( Object *a, Object *b ){
	if( a->type->l_greater != NULL ){
		return a->type->l_greater(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_less_or_same( Object *a, Object *b ){
	if( a->type->l_less_or_same != NULL ){
		return a->type->l_less_or_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_greater_or_same( Object *a, Object *b ){
	if( a->type->l_greater_or_same != NULL ){
		return a->type->l_greater_or_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>=' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_or( Object *a, Object *b ){
	if( a->type->l_or != NULL ){
		return a->type->l_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '||' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_l_and( Object *a, Object *b ){
	if( a->type->l_and != NULL ){
		return a->type->l_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&&' operator for object type '%s'", ob_typename(a) );
	}
}

INLINE Object *ob_cl_push( Object *a, Object *b ){
	if( a->type->cl_push != NULL ){
		return a->type->cl_push(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

INLINE Object *ob_cl_push_reference( Object *a, Object *b ){
	if( a->type->cl_push_reference != NULL ){
		return a->type->cl_push_reference(a,b);
	}
	else if( a->type->cl_push ){
		return a->type->cl_push(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

INLINE Object *ob_cl_pop( Object *o ){
	if( o->type->cl_pop != NULL ){
		return o->type->cl_pop(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(o) );
	}
}

INLINE Object *ob_cl_remove( Object *a, Object *b ){
	if( a->type->cl_remove != NULL ){
		return a->type->cl_remove(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

INLINE Object *ob_cl_at( Object *a, Object *b ){
	if( a->type->cl_at != NULL ){
		return a->type->cl_at(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable object type", ob_typename(a) );
	}
}

INLINE Object *ob_cl_set( Object *a, Object *b, Object *c ){
    if( a->type->cl_set != NULL ){
		return a->type->cl_set(a,b,c);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

INLINE Object *ob_cl_set_reference( Object *a, Object *b, Object *c ){
    if( a->type->cl_set_reference != NULL ){
		return a->type->cl_set_reference(a,b,c);
	}
	else if( a->type->cl_set ){
		return a->type->cl_set(a,b,c);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

INLINE void ob_define_attribute( Object *o, char *name, access_t a, bool is_static /*= false*/  ){
	if( o->type->define_attribute != NULL ){
		return o->type->define_attribute(o,name,a,is_static);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(o) );
	}
}

INLINE access_t ob_attribute_access( Object *o, char * a ){
	if( o->type->attribute_access != NULL ){
		return o->type->attribute_access(o,a);
	}
	else{
		return asPublic;
	}
}

INLINE bool ob_attribute_is_static( Object *o, char *a ){
	if( o->type->attribute_is_static != NULL ){
		return o->type->attribute_is_static(o,a);
	}
	else{
		return false;
	}
}

INLINE void ob_set_attribute_access( Object *o, char *name, access_t a ){
	if( o->type->set_attribute_access != NULL ){
		return o->type->set_attribute_access(o,name,a);
	}
}

INLINE void ob_add_attribute( Object *s, char *a ){
    if( s->type->add_attribute != NULL ){
		return s->type->add_attribute(s,a);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

INLINE Object *ob_get_attribute( Object *s, char *a, bool with_descriptor /*= true*/ ){
    if( s->type->get_attribute != NULL ){
		return s->type->get_attribute(s,a,with_descriptor);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

INLINE void ob_set_attribute( Object *s, char *a, Object *v ){
    if( s->type->set_attribute != NULL ){
		return s->type->set_attribute(s,a,v);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

INLINE void ob_set_attribute_reference( Object *s, char *a, Object *v ){
    if( s->type->set_attribute_reference != NULL ){
		return s->type->set_attribute_reference(s,a,v);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

INLINE void ob_define_method( Object *c, char *name, Node *code ){
	if( c->type->define_method != NULL ){
		c->type->define_method( c, name, code );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class", ob_typename(c) );
	}
}

INLINE Node *ob_get_method( Object *c, char *name, int argc /*= -1*/ ){
	if( c->type->get_method != NULL ){
		return c->type->get_method( c, name, argc );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class", ob_typename(c) );
	}
}

INLINE Object *ob_call_method( vm_t *vm, vframe_t *frame, Object *owner, char *owner_id, char *method_id, Node *argv ){
	if( owner->type->call_method != NULL ){
		return owner->type->call_method( vm, frame, owner, owner_id, method_id, argv );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class neither has builtin methods", ob_typename(owner) );
	}
}

#endif

