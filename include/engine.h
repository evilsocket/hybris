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

class VM;

/*
 * Tnis is the main class that will execute the script tree.
 */
class Engine {
    private :
		/*
		 * Main context.
		 */
        VM *vmachine;
        /*
         * Memory segment.
         */
        vmem_t  *vm;
        /*
         * Code segment
         */
        vcode_t *vc;
        /*
         * Type definitions segment.
         */
        vmem_t  *vt;
        /*
         * Find the entry point (address) of a given user defined function.
         */
        Node   *findEntryPoint( vframe_t *frame, Node *call );
        /*
         * Handle hybris builtin function call.
         */
        Object *onBuiltinFunctionCall( vframe_t *, Node * );
        /*
         * Handle user defined function call.
         */
        Object *onUserFunctionCall( vframe_t *, Node *, int threaded = 0 );
        /*
         * Handle dynamic loaded function call.
         */
        Object *onDllFunctionCall( vframe_t *, Node *, int threaded = 0 );

    public  :

        Engine( VM *context );
        ~Engine();
        /*
		 * Node handler dispatcher.
		 */
		Object *exec( vframe_t *frame, Node *node );
		/*
		 * Special case to handle threaded function calls.
		 */
		Object *onThreadedCall( string function_name, vframe_t *frame, vmem_t *argv );
        /*
         * Node handlers for each type of node (statement, expression, ...).
         *
         * The handler will recieve the current memory frame and the node
         * to execute or reduce.
         */

		/*
		 * Special nodes.
		 */
        Object *onIdentifier( vframe_t *, Node * );
        Object *onMemberRequest( vframe_t *, Node * );
        Object *onConstant( vframe_t *, Node * );
        Object *onFunctionDeclaration( vframe_t *, Node * );
        Object *onStructureDeclaration( vframe_t *, Node * );
        Object *onClassDeclaration( vframe_t *, Node * );
        Object *onFunctionCall( vframe_t *, Node *, int threaded = 0 );
        Object *onNewOperator( vframe_t *, Node * );
        /*
         * Statements.
         */
        Object *onReturn( vframe_t *, Node * );
        Object *onThrow( vframe_t *, Node * );
		Object *onTryCatch( vframe_t *, Node * );
		Object *onWhile( vframe_t *, Node * );
		Object *onDo( vframe_t *, Node * );
		Object *onFor( vframe_t *, Node * );
		Object *onForeach( vframe_t *, Node * );
		Object *onForeachMapping( vframe_t *, Node * );
		Object *onIf( vframe_t *, Node * );
		Object *onQuestion( vframe_t *, Node * );
		Object *onSwitch( vframe_t *, Node * );
		/*
		 * Expressions / Operators.
		 */
		/* misc */
        Object *onDollar( vframe_t *, Node * );
        Object *onRange( vframe_t *, Node * );
        Object *onSubscriptAdd( vframe_t *, Node * );
        Object *onSubscriptGet( vframe_t *, Node * );
        Object *onSubscriptSet( vframe_t *, Node * );
        Object *onEostmt( vframe_t *, Node * );
        Object *onDot( vframe_t *, Node * );
        Object *onInplaceDot( vframe_t *, Node * );
        /* arithmetic */
        Object *onAssign( vframe_t *, Node * );
        Object *onUminus( vframe_t *, Node * );
        Object *onRegex( vframe_t *, Node * );
        Object *onAdd( vframe_t *, Node * );
        Object *onInplaceAdd( vframe_t *, Node * );
        Object *onSub( vframe_t *, Node * );
        Object *onInplaceSub( vframe_t *, Node * );
        Object *onMul( vframe_t *, Node * );
        Object *onInplaceMul( vframe_t *, Node * );
        Object *onDiv( vframe_t *, Node * );
        Object *onInplaceDiv( vframe_t *, Node * );
        Object *onMod( vframe_t *, Node * );
        Object *onInplaceMod( vframe_t *, Node * );
        Object *onInc( vframe_t *, Node * );
        Object *onDec( vframe_t *, Node * );
        Object *onFact( vframe_t *, Node * );
        /* bitwise */
        Object *onXor( vframe_t *, Node * );
        Object *onInplaceXor( vframe_t *, Node * );
        Object *onAnd( vframe_t *, Node * );
        Object *onInplaceAnd( vframe_t *, Node * );
        Object *onOr( vframe_t *, Node * );
        Object *onInplaceOr( vframe_t *, Node * );
        Object *onShiftl( vframe_t *, Node * );
        Object *onInplaceShiftl( vframe_t *, Node * );
        Object *onShiftr( vframe_t *, Node * );
        Object *onInplaceShiftr( vframe_t *, Node * );
        Object *onNot( vframe_t *, Node * );
        /* logic */
        Object *onLnot( vframe_t *, Node * );
        Object *onLess( vframe_t *, Node * );
        Object *onGreater( vframe_t *, Node * );
        Object *onGe( vframe_t *, Node * );
        Object *onLe( vframe_t *, Node * );
        Object *onNe( vframe_t *, Node * );
        Object *onEq( vframe_t *, Node * );
        Object *onLand( vframe_t *, Node * );
        Object *onLor( vframe_t *, Node * );
};


#endif
