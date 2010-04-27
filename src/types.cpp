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
#include "mseg.h"
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
	if( o->type->type_name != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->type_name(o);
	}
}

void ob_set_references( Object *o, int ref ){
    if( o->type->set_references != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->set_references(o,ref);
    }
}

Object* ob_clone( Object *o ){
    if( o->type->clone != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->clone(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't copy the object type '%s'", o->type->name );
    }
}

bool ob_free( Object *o ){
    /*
     * In any case, decrement object reference counter.
     */
    ob_set_references( o, -1 );

    if( o->type->free != HYB_UNIMPLEMENTED_FUNCTION ){
    	/*
    	 * The free function is defined only for collection types or
    	 * types that handles pointers anyway.
    	 * Basically, instead of licterally freeing its items, the object
    	 * will just decrement their reference counters.
    	 */
        o->type->free(o);
        return true;
    }
    return false;
}

size_t ob_get_size( Object *o ){
	if( o->type->get_size != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->get_size(o);
	}
	/*
	 * Every type should define a get_size function.
	 * Should we trig an error?
	 */
	return 0;
}

byte * ob_serialize( Object *o, size_t size ){
	if( o->type->serialize != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->serialize(o,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't serialize '%s'", o->type->name );
}

Object *ob_deserialize( Object *o, byte *buffer, size_t size ){
	if( o->type->deserialize != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->deserialize(o,buffer,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't deserialize '%s'", o->type->name );
}

Object *ob_to_fd( Object *o, int fd, size_t size ){
	if( o->type->to_fd != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->to_fd(o,fd,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't write object '%s' to file descriptor", o->type->name );
}

Object *ob_from_fd( Object *o, int fd, size_t size ){
	if( o->type->from_fd != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->from_fd(o,fd,size);
	}
	hyb_error( H_ET_SYNTAX, "couldn't read object '%s' from file descriptor", o->type->name );
}

int ob_cmp( Object *o, Object * cmp ){
    if( o->type->cmp != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->cmp(o,cmp);
    }
    hyb_error( H_ET_SYNTAX, "couldn't compare '%s' object with '%s' object", o->type->name, cmp->type->name );
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
        hyb_error( H_ET_SYNTAX, "couldn't get the integer value of type '%s'", o->type->name );
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
        hyb_error( H_ET_SYNTAX, "couldn't get the float value of type '%s'", o->type->name );
    }
    return 0.0;
}

bool ob_lvalue( Object *o ){
    if( o->type->lvalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->lvalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the logical value of the object type '%s'", o->type->name );
    }
}

string ob_svalue( Object *o ){
    if( o->type->svalue != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->svalue(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't get the string rapresentation of the object type '%s'", o->type->name );
    }
}

void ob_print( Object *o, int tabs /* = 0 */){
    if( o->type->print != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->print(o,tabs);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't print the object type '%s'", o->type->name );
    }
}

void ob_input( Object *o ){
    if( o->type->scanf != HYB_UNIMPLEMENTED_FUNCTION ){
        return o->type->scanf(o);
    }
    else{
        hyb_error( H_ET_SYNTAX, "couldn't read the object type '%s' from stdin", o->type->name );
    }
}

Object *ob_to_string( Object *o ){
	if( o->type->to_string != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->to_string(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' to string", o->type->name );
	}
}

Object *ob_to_int( Object *o ){
	if( o->type->to_int != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->to_int(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' to int", o->type->name );
	}
}

Object *ob_from_int( Object *o ){
	if( o->type->from_int != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->from_int(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' from int", o->type->name );
	}
}

Object *ob_from_float( Object *o ){
    if( o->type->from_float != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->from_float(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "couldn't convert object type '%s' from float", o->type->name );
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
        hyb_error( H_ET_SYNTAX, "invalid type '%s' for left operand of '..'", a->type->name );
    }
    else if( ob_is_type_in( b, &Char_Type, &Integer_Type, NULL ) == false ){
        hyb_error( H_ET_SYNTAX, "invalid type %s for right operand '..'", a->type->name );
    }
    else if( ob_same_type(a,b) == false ){
		hyb_error( H_ET_SYNTAX, "types must be the same for '..' operator" );
	}

	if( a->type->range != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->range(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '..' operator for object type '%s'", a->type->name );
	}
}

Object *ob_apply_regexp( Object *a, Object *b ){
	if( a->type->regexp != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->regexp(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '~=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_assign( Object *a, Object *b ){
	if( a->type->assign != HYB_UNIMPLEMENTED_FUNCTION ){
	    /**
	     * NOTE: If 'a' is already defined, the assign method will just replace
		 * its value, then the VM will decrement old value reference counter.
	     */
		return a->type->assign(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '=' for object type '%s'", a->type->name );
	}
}

Object *ob_factorial( Object *o ){
	if( o->type->factorial != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->factorial(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!' operator for object type '%s'", o->type->name );
	}
}

Object *ob_increment( Object *o ){
	if( o->type->increment != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->increment(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '++' operator for object type '%s'", o->type->name );
	}
}

Object *ob_decrement( Object *o ){
	if( o->type->decrement != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->decrement(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '--' operator for object type '%s'", o->type->name );
	}
}

Object *ob_uminus( Object *o ){
	if( o->type->minus != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->minus(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-' operator for object type '%s'", o->type->name );
	}
}

Object *ob_add( Object *a, Object *b ){
	if( a->type->add != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->add(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '+' operator for object type '%s'", a->type->name );
	}
}

Object *ob_sub( Object *a, Object *b ){
	if( a->type->sub != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->sub(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-' operator for object type '%s'", a->type->name );
	}
}

Object *ob_mul( Object *a, Object *b ){
	if( a->type->mul != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->mul(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '*' operator for object type '%s'", a->type->name );
	}
}

Object *ob_div( Object *a, Object *b ){
	if( a->type->div != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->div(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '/' operator for object type '%s'", a->type->name );
	}
}

Object *ob_mod( Object *a, Object *b ){
	if( a->type->mod != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->mod(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '%' operator for object type '%s'", a->type->name );
	}
}

Object *ob_inplace_add( Object *a, Object *b ){
	if( a->type->inplace_add != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_add(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '+=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_inplace_sub( Object *a, Object *b ){
	if( a->type->inplace_sub != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_sub(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '-=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_inplace_mul( Object *a, Object *b ){
	if( a->type->inplace_mul != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_mul(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '*=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_inplace_div( Object *a, Object *b ){
	if( a->type->inplace_div != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_div(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '/=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_inplace_mod( Object *a, Object *b ){
	if( a->type->inplace_mod != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->inplace_mod(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '%=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_and( Object *a, Object *b ){
	if( a->type->bw_and != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_or( Object *a, Object *b ){
	if( a->type->bw_or != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '|' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_not( Object *o ){
	if( o->type->bw_not != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->bw_not(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '~' operator for object type '%s'", o->type->name );
	}
}

Object *ob_bw_xor( Object *a, Object *b ){
	if( a->type->bw_xor != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_xor(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '^' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_lshift( Object *a, Object *b ){
	if( a->type->bw_lshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_lshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<<' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_rshift( Object *a, Object *b ){
	if( a->type->bw_rshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_rshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>>' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_inplace_and( Object *a, Object *b ){
	if( a->type->bw_inplace_and != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_inplace_or( Object *a, Object *b ){
	if( a->type->bw_inplace_or != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '|=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_inplace_xor( Object *a, Object *b ){
	if( a->type->bw_inplace_xor != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_xor(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '^=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_inplace_lshift( Object *a, Object *b ){
	if( a->type->bw_inplace_lshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_lshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<<=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_bw_inplace_rshift( Object *a, Object *b ){
	if( a->type->bw_inplace_rshift != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->bw_inplace_rshift(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>>=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_not( Object *o ){
	if( o->type->l_not != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->l_not(o);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!' operator for object type '%s'", o->type->name );
	}
}

Object *ob_l_same( Object *a, Object *b ){
	if( a->type->l_same != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '==' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_diff( Object *a, Object *b ){
	if( a->type->l_diff != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_diff(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '!=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_less( Object *a, Object *b ){
	if( a->type->l_less != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_less(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_greater( Object *a, Object *b ){
	if( a->type->l_greater != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_greater(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_less_or_same( Object *a, Object *b ){
	if( a->type->l_less_or_same != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_less_or_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '<=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_greater_or_same( Object *a, Object *b ){
	if( a->type->l_greater_or_same != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_greater_or_same(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '>=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_or( Object *a, Object *b ){
	if( a->type->l_or != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_or(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '||' operator for object type '%s'", a->type->name );
	}
}

Object *ob_l_and( Object *a, Object *b ){
	if( a->type->l_and != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->l_and(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '&&' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_concat( Object *a, Object *b ){
	if( a->type->cl_concat != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_concat(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '.' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_inplace_concat( Object *a, Object *b ){
	if( a->type->cl_inplace_concat != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_inplace_concat(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '.=' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_push( Object *a, Object *b ){
	if( a->type->cl_push != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_push(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '[]' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_push_reference( Object *a, Object *b ){
	if( a->type->cl_push_reference != HYB_UNIMPLEMENTED_FUNCTION ){
	    ob_set_references( b, +1 );
		return a->type->cl_push_reference(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '[]' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_pop( Object *o ){
	if( o->type->cl_pop != HYB_UNIMPLEMENTED_FUNCTION ){
		Object *item = o->type->cl_pop(o);
		ob_set_references( item, -1 );
		return item;
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '[]' operator for object type '%s'", o->type->name );
	}
}

Object *ob_cl_remove( Object *a, Object *b ){
	if( a->type->cl_remove != HYB_UNIMPLEMENTED_FUNCTION ){
		Object *item = a->type->cl_remove(a,b);
		ob_set_references( item, -1 );
		return item;
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '[]' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_at( Object *a, Object *b ){
	if( a->type->cl_at != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_at(a,b);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '[]' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_set( Object *a, Object *b, Object *c ){
    if( a->type->cl_set != HYB_UNIMPLEMENTED_FUNCTION ){
		return a->type->cl_set(a,b,c);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '[] =' operator for object type '%s'", a->type->name );
	}
}

Object *ob_cl_set_reference( Object *a, Object *b, Object *c ){
    if( a->type->cl_set_reference != HYB_UNIMPLEMENTED_FUNCTION ){
        ob_set_references( c, +1 );
		return a->type->cl_set_reference(a,b,c);
	}
	else{
		hyb_error( H_ET_SYNTAX, "invalid '[] =' operator for object type '%s'", a->type->name );
	}
}

void ob_define_attribute( Object *o, char *name, access_t a ){
	if( o->type->define_attribute != HYB_UNIMPLEMENTED_FUNCTION ){
		return o->type->define_attribute(o,name,a);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", o->type->name );
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
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", s->type->name );
	}
}

Object *ob_get_attribute( Object *s, char *a, bool with_descriptor /*= true*/ ){
    if( s->type->get_attribute != HYB_UNIMPLEMENTED_FUNCTION ){
		return s->type->get_attribute(s,a,with_descriptor);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", s->type->name );
	}
}

void ob_set_attribute( Object *s, char *a, Object *v ){
    if( s->type->set_attribute != HYB_UNIMPLEMENTED_FUNCTION ){
		return s->type->set_attribute(s,a,v);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", s->type->name );
	}
}

void ob_set_attribute_reference( Object *s, char *a, Object *v ){
    if( s->type->set_attribute_reference != HYB_UNIMPLEMENTED_FUNCTION ){
        ob_set_references( v, +1 );
		return s->type->set_attribute_reference(s,a,v);
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a structure nor a class", s->type->name );
	}
}

void ob_define_method( Object *c, char *name, Node *code ){
	if( c->type->define_method != HYB_UNIMPLEMENTED_FUNCTION ){
		c->type->define_method( c, name, code );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class", c->type->name );
	}
}

Node *ob_get_method( Object *c, char *name, int argc /*= -1*/ ){
	if( c->type->get_method != HYB_UNIMPLEMENTED_FUNCTION ){
		return c->type->get_method( c, name, argc );
	}
	else{
		hyb_error( H_ET_SYNTAX, "object type '%s' does not name a class", c->type->name );
	}
}

Object *ob_call_undefined_method( VM *vm, Object *c, char *c_name, char *method_name, Node *argv ){
	Node    *identifier = H_UNDEFINED,
		    *method     = H_UNDEFINED;
	vframe_t stack;
	Object  *value  = H_UNDEFINED,
			*result = H_UNDEFINED;
	unsigned int i, j, argc(argv->children());

	method = ob_get_method( c, "__method", 2 );
	if( method == H_UNDEFINED ){
		return H_UNDEFINED;
	}

	VectorObject *args = gc_new_vector();

	stack.owner = string(c_name) + "::" + method_name;
	stack.insert( "me", c );
	stack.insert( "name", (Object *)gc_new_string(method_name) );
	for( i = 0; i < argc; ++i ){
		ob_cl_push( (Object *)args, vm->engine->exec( vm->vframe, argv->child(i) ) );
	}
	stack.insert( "argv", (Object *)args );

	vm->addFrame( &stack );

	/* call the method */
	result = vm->engine->exec( &stack, method->callBody() );

	vm->popFrame();

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm->frame()->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

Object *ob_call_method( VM *vm, Object *c, char *c_name, char *method_name, Object *argv ){
	Node    *identifier = H_UNDEFINED,
		    *method     = H_UNDEFINED;
	vframe_t stack;
	Object  *value  = H_UNDEFINED,
			*result = H_UNDEFINED;
	IntegerObject index(0);
	unsigned int i, j, argc( ob_get_size(argv) );

	method = ob_get_method( c, method_name, 2 );
	if( method == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "'%s' does not name a method neither an attribute of '%s'", method_name, c_name );
	}
	stack.owner = string(c_name) + "::" + method_name;
	stack.insert( "me", c );
	for( ; index.value < argc; ++index.value ){
		stack.insert( (char *)method->child(j)->value.m_identifier.c_str(), ob_cl_at( argv, (Object *)&index ) );
	}

	vm->addFrame( &stack );

	/* call the method */
	result = vm->engine->exec( &stack, method->callBody() );

	vm->popFrame();

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state.is(Exception) ){
		vm->frame()->state.set( Exception, stack.state.value );
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

