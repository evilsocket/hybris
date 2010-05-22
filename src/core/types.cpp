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
#include "types.h"
#include "common.h"
#include "memory.h"
#include "node.h"
#include "vm.h"

#define HYB_UNIMPLEMENTED_FUNCTION 0

IntegerObject __default_return_value(0);
IntegerObject __default_error_value(-1);

extern void hyb_error( H_ERROR_TYPE type, const char *format, ... );

bool ob_is_type_in( Object *o, ... ){
    va_list        ap;
	object_type_t *ptype;

	va_start( ap, o );
	do{
		ptype = va_arg( ap, object_type_t * );
		if( o->type->code == ptype->code ){
            return true;
		}
	}
	while(ptype);
	va_end(ap);

	return false;
}

const char *ob_typename( Object * o ){
	return (o->type->type_name ? o->type->type_name(o) : o->type->name);
}

Object *ob_traverse( Object *o, int index ){
	#ifdef GC_DEBUG
		fprintf( stdout, "[GC DEBUG] Traversing object at %p, index %d.\n", o, index );
	#endif
	return (o->type->traverse ? o->type->traverse(o,index) : NULL);
}

Object* ob_clone( Object *o ){
    /*
	 * Every object has to implement its own clone.
	 */
	return o->type->clone(o);
}

bool ob_free( Object *o ){
    if( o->type->free != HYB_UNIMPLEMENTED_FUNCTION ){
    	/*
    	 * The free function is defined only for collection types or
    	 * types that handles pointers anyway.
    	 */
        o->type->free(o);

        return true;
    }

    return false;
}

size_t ob_get_size( Object *o ){
	return (o->type->get_size ? o->type->get_size(o) : o->type->size);
}

byte * ob_serialize( Object *o, size_t size ){
	if( o->type->serialize != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->serialize(o,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't serialize '%s'", ob_typename(o) );
}

Object *ob_deserialize( Object *o, byte *buffer, size_t size ){
	if( o->type->deserialize != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->deserialize(o,buffer,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't deserialize '%s'", ob_typename(o) );
}

Object *ob_to_fd( Object *o, int fd, size_t size ){
	if( o->type->to_fd != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->to_fd(o,fd,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't write object '%s' to file descriptor", ob_typename(o) );
}

Object *ob_from_fd( Object *o, int fd, size_t size ){
	if( o->type->from_fd != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->from_fd(o,fd,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't read object '%s' from file descriptor", ob_typename(o) );
}

int ob_cmp( Object *o, Object * cmp ){
    if( o->type->cmp != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->cmp(o,cmp);
    }
    hyb_error( H_ET_SYNTAX, "couldn't compare '%s' object with '%s' object", ob_typename(o), ob_typename(cmp) );
}

long ob_ivalue( Object * o ){
    if( ob_is_int(o) ){
        return (ob_int_ucast(o))->value;
    }
    else if( o->type->ivalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->ivalue(o);
    }
    else if( o->type->lvalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return (long)o->type->lvalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the integer value of type '%s'", ob_typename(o) );
    }
    return 0;
}

double ob_fvalue( Object *o ){
    if( ob_is_float(o) ){
        return ob_float_ucast(o)->value;
    }
    else if( o->type->fvalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->fvalue(o);
    }
    else if( o->type->ivalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return (double)o->type->ivalue(o);
    }
    else if( o->type->lvalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return (double)o->type->lvalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the float value of type '%s'", ob_typename(o) );
    }
    return 0.0;
}

bool ob_lvalue( Object *o ){
    if( o->type->lvalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->lvalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the logical value of the object type '%s'", ob_typename(o) );
    }
}

string ob_svalue( Object *o ){
    if( o->type->svalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->svalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the string rapresentation of the object type '%s'", ob_typename(o) );
    }
}

void ob_print( Object *o, int tabs /* = 0 */){
    if( o->type->print != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->print(o,tabs);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't print the object type '%s'", ob_typename(o) );
    }
}

void ob_input( Object *o ){
    if( o->type->scanf != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->scanf(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't read the object type '%s' from stdin", ob_typename(o) );
    }
}

Object *ob_to_string( Object *o ){
	if( o->type->to_string != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->to_string(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' to string", ob_typename(o) );
	}
}

Object *ob_to_int( Object *o ){
	if( o->type->to_int != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->to_int(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' to int", ob_typename(o) );
	}
}

Object *ob_range( Object *a, Object *b ){
	/*
	 * Range ( '..' ) operator admits only integer and char operands,
	 * both of the same type, so :
	 *
	 * int .. int   : OK
	 * char .. char : OK
	 * char .. int  : Syntax error "types must be the same for '..' operator"
	 * ...
	 */
	if( ob_is_type_in( a, &Char_Type, &Integer_Type, NULL ) == false ){
        hyb_error( H_ET_SYNTAX, "invalid type '%s' for left operand of '..'", ob_typename(a) );
    }
    else if( ob_is_type_in( b, &Char_Type, &Integer_Type, NULL ) == false ){
        hyb_error( H_ET_SYNTAX, "invalid type %s for right operand '..'", ob_typename(a) );
    }
    else if( ob_same_type(a,b) == false ){
		hyb_error( H_ET_SYNTAX, "types must be the same for '..' operator" );
	}

	if( a->type->range != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->range(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '..' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_apply_regexp( Object *a, Object *b ){
	if( a->type->regexp != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->regexp(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '~=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_assign( Object *a, Object *b ){
	/*
	 * Every object has to implement its own assign.
     *
	 * NOTE: If 'a' is already defined, the assign method will just replace
	 * its value.
	 */
	return a->type->assign(a,b);
}

Object *ob_factorial( Object *o ){
	if( o->type->factorial != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->factorial(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!' operator for object type '%s'", ob_typename(o) );
	}
}

Object *ob_increment( Object *o ){
	if( o->type->increment != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->increment(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '++' operator for object type '%s'", ob_typename(o) );
	}
}

Object *ob_decrement( Object *o ){
	if( o->type->decrement != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->decrement(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '--' operator for object type '%s'", ob_typename(o) );
	}
}

Object *ob_uminus( Object *o ){
	if( o->type->minus != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->minus(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-' operator for object type '%s'", ob_typename(o) );
	}
}

Object *ob_add( Object *a, Object *b ){
	if( a->type->add != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->add(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '+' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_sub( Object *a, Object *b ){
	if( a->type->sub != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->sub(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_mul( Object *a, Object *b ){
	if( a->type->mul != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->mul(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '*' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_div( Object *a, Object *b ){
	if( a->type->div != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->div(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '/' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_mod( Object *a, Object *b ){
	if( a->type->mod != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->mod(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '%' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_inplace_add( Object *a, Object *b ){
	if( a->type->inplace_add != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_add(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '+=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_inplace_sub( Object *a, Object *b ){
	if( a->type->inplace_sub != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_sub(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_inplace_mul( Object *a, Object *b ){
	if( a->type->inplace_mul != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_mul(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '*=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_inplace_div( Object *a, Object *b ){
	if( a->type->inplace_div != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_div(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '/=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_inplace_mod( Object *a, Object *b ){
	if( a->type->inplace_mod != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_mod(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '%=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_and( Object *a, Object *b ){
	if( a->type->bw_and != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_or( Object *a, Object *b ){
	if( a->type->bw_or != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '|' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_not( Object *o ){
	if( o->type->bw_not != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->bw_not(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '~' operator for object type '%s'", ob_typename(o) );
	}
}

Object *ob_bw_xor( Object *a, Object *b ){
	if( a->type->bw_xor != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_xor(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '^' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_lshift( Object *a, Object *b ){
	if( a->type->bw_lshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_lshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<<' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_rshift( Object *a, Object *b ){
	if( a->type->bw_rshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_rshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>>' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_inplace_and( Object *a, Object *b ){
	if( a->type->bw_inplace_and != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_inplace_or( Object *a, Object *b ){
	if( a->type->bw_inplace_or != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '|=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_inplace_xor( Object *a, Object *b ){
	if( a->type->bw_inplace_xor != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_xor(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '^=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_inplace_lshift( Object *a, Object *b ){
	if( a->type->bw_inplace_lshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_lshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<<=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_bw_inplace_rshift( Object *a, Object *b ){
	if( a->type->bw_inplace_rshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_rshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>>=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_not( Object *o ){
	if( o->type->l_not != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->l_not(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!' operator for object type '%s'", ob_typename(o) );
	}
}

Object *ob_l_same( Object *a, Object *b ){
	if( a->type->l_same != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '==' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_diff( Object *a, Object *b ){
	if( a->type->l_diff != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_diff(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_less( Object *a, Object *b ){
	if( a->type->l_less != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_less(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_greater( Object *a, Object *b ){
	if( a->type->l_greater != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_greater(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_less_or_same( Object *a, Object *b ){
	if( a->type->l_less_or_same != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_less_or_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_greater_or_same( Object *a, Object *b ){
	if( a->type->l_greater_or_same != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_greater_or_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>=' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_or( Object *a, Object *b ){
	if( a->type->l_or != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '||' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_l_and( Object *a, Object *b ){
	if( a->type->l_and != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&&' operator for object type '%s'", ob_typename(a) );
	}
}

Object *ob_cl_push( Object *a, Object *b ){
	if( a->type->cl_push != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_push(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

Object *ob_cl_push_reference( Object *a, Object *b ){
	if( a->type->cl_push_reference != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_push_reference(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

Object *ob_cl_pop( Object *o ){
	if( o->type->cl_pop != HYB_UNIMPLEMENTED_FUNCTION ){
		Object *item = o->type->cl_pop(o);
		return item;
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(o) );
	}
}

Object *ob_cl_remove( Object *a, Object *b ){
	if( a->type->cl_remove != HYB_UNIMPLEMENTED_FUNCTION ){
		Object *item = a->type->cl_remove(a,b);
		return item;
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

Object *ob_cl_at( Object *a, Object *b ){
	if( a->type->cl_at != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_at(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable object type", ob_typename(a) );
	}
}

Object *ob_cl_set( Object *a, Object *b, Object *c ){
    if( a->type->cl_set != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_set(a,b,c);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

Object *ob_cl_set_reference( Object *a, Object *b, Object *c ){
    if( a->type->cl_set_reference != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_set_reference(a,b,c);
	}
	else{
		hyb_error( H_ET_SYNTAX, "'%s' not iterable or not editable object type", ob_typename(a) );
	}
}

void ob_define_attribute( Object *o, char *name, access_t a, bool is_static /*= false*/  ){
	if( o->type->define_attribute != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->define_attribute(o,name,a,is_static);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(o) );
	}
}

access_t ob_attribute_access( Object *o, char * a ){
	if( o->type->attribute_access != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->attribute_access(o,a);
	}
	else{
		return asPublic;
	}
}

bool ob_attribute_is_static( Object *o, char *a ){
	if( o->type->attribute_is_static != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->attribute_is_static(o,a);
	}
	else{
		return false;
	}
}

void ob_set_attribute_access( Object *o, char *name, access_t a ){
	if( o->type->set_attribute_access != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->set_attribute_access(o,name,a);
	}
}

void ob_add_attribute( Object *s, char *a ){
    if( s->type->add_attribute != HYB_UNIMPLEMENTED_FUNCTION ){
		return s->type->add_attribute(s,a);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

Object *ob_get_attribute( Object *s, char *a, bool with_descriptor /*= true*/ ){
    if( s->type->get_attribute != HYB_UNIMPLEMENTED_FUNCTION ){
		return s->type->get_attribute(s,a,with_descriptor);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

void ob_set_attribute( Object *s, char *a, Object *v ){
    if( s->type->set_attribute != HYB_UNIMPLEMENTED_FUNCTION ){
		return s->type->set_attribute(s,a,v);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

void ob_set_attribute_reference( Object *s, char *a, Object *v ){
    if( s->type->set_attribute_reference != HYB_UNIMPLEMENTED_FUNCTION ){
		return s->type->set_attribute_reference(s,a,v);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", ob_typename(s) );
	}
}

void ob_define_method( Object *c, char *name, Node *code ){
	if( c->type->define_method != HYB_UNIMPLEMENTED_FUNCTION ){
		c->type->define_method( c, name, code );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class", ob_typename(c) );
	}
}

Node *ob_get_method( Object *c, char *name, int argc /*= -1*/ ){
	if( c->type->get_method != HYB_UNIMPLEMENTED_FUNCTION ){
		return c->type->get_method( c, name, argc );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class", ob_typename(c) );
	}
}

Object *ob_call_method( engine_t *engine, vframe_t *frame, Object *owner, char *owner_id, char *method_id, Node *argv ){
	if( owner->type->call_method != HYB_UNIMPLEMENTED_FUNCTION ){
		return owner->type->call_method( engine, frame, owner, owner_id, method_id, argv );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class neither has builtin methods", ob_typename(owner) );
	}
}

Object *ob_call_method( vm_t *vm, Object *c, char *c_name, char *method_name, Object *argv ){
	Node    *method = H_UNDEFINED;
	vframe_t stack;
	Object  *value  = H_UNDEFINED,
			*result = H_UNDEFINED;
	IntegerObject index(0);
	size_t j, argc( ob_get_size(argv) );

	method = ob_get_method( c, method_name, 2 );
	if( method == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' does not name a method neither an attribute of '%s'", method_name, c_name );
	}
	stack.owner = string(c_name) + "::" + method_name;
	stack.insert( "me", c );
	for( ; (unsigned)index.value < argc; ++index.value ){
		value = ob_cl_at( argv, (Object *)&index );
		stack.insert( (char *)method->child(j)->value.m_identifier.c_str(), value  );
	}

	vm_add_frame( vm, &stack );

	/* call the method */
	result = engine_exec( vm->engine, &stack, method->callBody() );

	vm_pop_frame( vm );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm_frame( vm )->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

Object *ob_call_undefined_method( vm_t *vm, Object *c, char *c_name, char *method_name, Node *argv ){
	Node    *method = H_UNDEFINED;
	vframe_t stack,
			*frame;
	Object  *value  = H_UNDEFINED,
			*result = H_UNDEFINED;
	size_t i, argc(argv->children());

	method = ob_get_method( c, (char *)"__method", 2 );
	if( method == H_UNDEFINED ){
		return H_UNDEFINED;
	}

	frame = vm_frame(vm);

	VectorObject *args = gc_new_vector();

	stack.owner = string(c_name) + "::" + string("__method");

	stack.insert( "me", c );
	stack.add( "name", (Object *)gc_new_string(method_name) );
	for( i = 0; i < argc; ++i ){
		value = engine_exec( vm->engine, frame, argv->child(i) );

		engine_check_frame_exit(frame);

		ob_cl_push( (Object *)args, value );
	}
	stack.add( "argv", (Object *)args );

	vm_add_frame( vm, &stack );

	/* call the method */
	result = engine_exec( vm->engine, &stack, method->callBody() );

	vm_pop_frame( vm );

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm_frame( vm )->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

ob_type_builtin_method_t *ob_get_builtin_method( Object *c, char *method_id ){
	size_t 				  i;
	ob_builtin_methods_t *methods = c->type->builtin_methods;
	/*
	 * Builtin methods are supposed to be only a few, so a loop with a string
	 * comparision is faster than an hashmap/asciitree initialization and
	 * further search.
	 */
	for( i = 0; methods[i].method != NULL; ++i ){
		if( methods[i].name == method_id ){
			return methods[i].method;
		}
	}

	return NULL;
}
