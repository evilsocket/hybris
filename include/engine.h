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
#ifndef _HENGINE_H_
#   define _HENGINE_H_

#include "hybris.h"

/*
 * Max recursions or nested functions calls.
 */
#define MAX_RECURSION_THRESHOLD 10000

#define engine_on_break_state( frame ) frame->state.set(Break)
#define engine_on_next_state( frame )  frame->state.set(Next)

#define engine_check_frame_exit(frame) if( frame->state.is(Exception) ){ \
										   return frame->state.value; \
									   } \
									   else if( frame->state.is(Return) ){ \
										   return frame->state.value; \
									   }

typedef struct _named_function_t named_function_t;
typedef struct _vm_t 			 vm_t;

/*
 * The main structure holding infos to execute the script tree.
 */
typedef struct _engine_t {
	/*
	 * Virtual machine.
	 */
	vm_t 	*vm;
	/*
	 * Constants segment.
	 */
	vmem_t  *cnst;
	/*
	 * Memory segment.
	 */
	vmem_t  *mem;
	/*
	 * Code segment
	 */
	vcode_t *code;
	/*
	 * Type definitions segment.
	 */
	vmem_t  *types;
}
engine_t;

/*
 * Alloc an instance of the engine.
 */
engine_t *engine_create( vm_t* vm );
/*
 * Pretty self explainatory.
 */
#define   engine_free( e ) delete e
/*
 * Methods to initialize a stack given its owner, arguments, identifiers
 * and so on.
 *
 * Each prepare_stack method will increment by one the reference counter
 * of each value pushed/inserted into it, then the dismiss_stack method
 * will decrement them again.
 */
void 	  engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, Object *cobj, int argc, Node *prototype, Node *argv );
void 	  engine_prepare_stack( engine_t *engine, vframe_t &stack, string owner, Object *cobj, Node *ids, int argc, ... );
void 	  engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, vector<string> ids, vmem_t *argv );
void 	  engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, vector<string> ids, Node *argv );
void 	  engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, ExternObject *fn_pointer, Node *argv );
void 	  engine_prepare_stack( engine_t *engine, vframe_t *root, vframe_t &stack, string owner, Node *argv );
void 	  engine_prepare_stack( engine_t *engine, vframe_t *root, named_function_t *function, vframe_t &stack, string owner, Node *argv );
void 	  engine_dismiss_stack( engine_t *engine, vframe_t &stack );
/*
 * Find the entry point (address) of a given user defined function.
 */
Node     *engine_find_function( engine_t *engine, vframe_t *frame, Node *call );
/*
 * Handle hybris builtin function call.
 */
Object   *engine_on_builtin_function_call( engine_t *engine, vframe_t *, Node * );
/*
 * Handle user defined function call.
 */
Object   *engine_on_user_function_call( engine_t *engine, vframe_t *, Node * );
/*
 * Handle dynamic loaded function call.
 */
Object   *engine_on_dll_function_call( engine_t *engine, vframe_t *, Node * );
/*
 * Special case to handle threaded function calls by name and by alias.
 */
Object   *engine_on_threaded_call( engine_t *engine, string function_name, vframe_t *frame, vmem_t *argv );
Object   *engine_on_threaded_call( engine_t *engine, Node *function, vframe_t *frame, vmem_t *argv );
/*
 * Node handler dispatcher.
 */
Object 	 *engine_exec( engine_t *engine, vframe_t *frame, Node *node );
/*
 * Identifier found, do a memory lookup for it.
 */
Object 	 *engine_on_identifier( engine_t *engine, vframe_t *, Node * );
/*
 * expression->expression, evaluate first expression to find out if
 * it's a class or a structure, and then lookup inside it the second one.
 */
Object   *engine_on_attribute_request( engine_t *engine, vframe_t *, Node * );
/*
 * expression->expression(...), evaluate first expression to find out if
 * it's a class or a structure, and then lookup inside it the second one.
 */
Object   *engine_on_method_call( engine_t *engine, vframe_t *, Node * );
/*
 * Constant found, just return it from the evaluation tree.
 */
Object   *engine_on_constant( engine_t *engine, vframe_t *, Node * );
/*
 * Create an array.
 */
Object   *engine_on_array( engine_t *engine, vframe_t *, Node * );
/*
 * Create a map.
 */
Object   *engine_on_map( engine_t *engine, vframe_t *, Node * );
/*
 * Function declaration, add it to the code segment.
 */
Object   *engine_on_function_declaration( engine_t *engine, vframe_t *, Node * );
/*
 * Structure declaration, create the prototype instance and add it
 * to the types segment.
 */
Object   *engine_on_structure_declaration( engine_t *engine, vframe_t *, Node * );
/*
 * Class declaration, create the prototype instance, and add it
 * to the types segment.
 */
Object   *engine_on_class_declaration( engine_t *engine, vframe_t *, Node * );
/*
 * expression( ... ), evaluate each argument and the run the function.
 */
Object   *engine_on_function_call( engine_t *engine, vframe_t *, Node * );
/*
 * new type( ... ), execute type lookup, clone the instance and if it's
 * a class and has a constructor, execute it, otherwise do default attributes
 * initialization.
 */
Object   *engine_on_new_operator( engine_t *engine, vframe_t *, Node * );
/*
 * Statements.
 */
Object   *engine_on_explode( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_return( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_throw( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_try_catch( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_while( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_do( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_for( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_foreach( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_foreach_mapping( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_unless( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_if( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_question( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_switch( engine_t *engine, vframe_t *, Node * );
/*
 * Expressions and operators.
 */
/* misc */
Object 	 *engine_on_backtick( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_vargs( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_dollar( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_reference( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_range( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_subscript_push( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_subscript_get( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_subscript_set( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_eostmt( engine_t *engine, vframe_t *, Node * );
/* arithmetic */
Object   *engine_on_assign( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_uminus( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_regex( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_add( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_add( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_sub( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_sub( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_mul( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_mul( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_div( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_div( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_mod( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_mod( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inc( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_dec( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_fact( engine_t *engine, vframe_t *, Node * );
/* bitwise */
Object   *engine_on_xor( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_xor( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_and( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_and( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_or( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_or( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_shiftl( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_shiftl( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_shiftr( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_inplace_shiftr( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_not( engine_t *engine, vframe_t *, Node * );
/* logic */
Object   *engine_on_lnot( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_less( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_greater( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_ge( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_le( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_ne( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_eq( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_land( engine_t *engine, vframe_t *, Node * );
Object   *engine_on_lor( engine_t *engine, vframe_t *, Node * );

#endif
