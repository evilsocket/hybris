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

class Context;

class Engine {
    private :

        Context *ctx;
        vmem_t  *vm;
        vcode_t *vc;
        vmem_t  *vt;

        Node * findEntryPoint( vframe_t *frame, Node *call, char *name );

        Object *onBuiltinFunctionCall( vframe_t *, Node * );
        Object *onUserFunctionCall( vframe_t *, Node *, int threaded = 0 );
        Object *onTypeCall( vframe_t *, Node * );
        Object *onDllFunctionCall( vframe_t *, Node *, int threaded = 0 );

    public  :

        Engine( Context *context );
        ~Engine();

        Object *onIdentifier( vframe_t *, Node * );
        Object *onAttribute( vframe_t *, Node * );
        Object *onConstant( vframe_t *, Node * );
        Object *onFunctionDeclaration( vframe_t *, Node * );
        Object *onStructureDeclaration( vframe_t *, Node * );
        Object *onFunctionCall( vframe_t *, Node *, int threaded = 0 );
        Object *onDollar( vframe_t *, Node * );
        Object *onPointer( vframe_t *, Node * );
    /** Object *onObject( vframe_t *, Node * ); **/
        Object *onReturn( vframe_t *, Node * );
        Object *onRange( vframe_t *, Node * );
        Object *onSubscriptAdd( vframe_t *, Node * );
        Object *onSubscriptGet( vframe_t *, Node * );
        Object *onSubscriptSet( vframe_t *, Node * );
        Object *onWhile( vframe_t *, Node * );
        Object *onDo( vframe_t *, Node * );
        Object *onFor( vframe_t *, Node * );
        Object *onForeach( vframe_t *, Node * );
        Object *onForeachm( vframe_t *, Node * );
        Object *onIf( vframe_t *, Node * );
        Object *onQuestion( vframe_t *, Node * );
        Object *onSwitch( vframe_t *, Node * );
        Object *onEostmt( vframe_t *, Node * );
        Object *onDot( vframe_t *, Node * );
        Object *onDote( vframe_t *, Node * );
        Object *onAssign( vframe_t *, Node * );
        Object *onUminus( vframe_t *, Node * );
        Object *onRegex( vframe_t *, Node * );
        Object *onPlus( vframe_t *, Node * );
        Object *onPluse( vframe_t *, Node * );
        Object *onMinus( vframe_t *, Node * );
        Object *onMinuse( vframe_t *, Node * );
        Object *onMul( vframe_t *, Node * );
        Object *onMule( vframe_t *, Node * );
        Object *onDiv( vframe_t *, Node * );
        Object *onDive( vframe_t *, Node * );
        Object *onMod( vframe_t *, Node * );
        Object *onMode( vframe_t *, Node * );
        Object *onInc( vframe_t *, Node * );
        Object *onDec( vframe_t *, Node * );
        Object *onXor( vframe_t *, Node * );
        Object *onXore( vframe_t *, Node * );
        Object *onAnd( vframe_t *, Node * );
        Object *onAnde( vframe_t *, Node * );
        Object *onOr( vframe_t *, Node * );
        Object *onOre( vframe_t *, Node * );
        Object *onShiftl( vframe_t *, Node * );
        Object *onShiftle( vframe_t *, Node * );
        Object *onShiftr( vframe_t *, Node * );
        Object *onShiftre( vframe_t *, Node * );
        Object *onFact( vframe_t *, Node * );
        Object *onNot( vframe_t *, Node * );
        Object *onLnot( vframe_t *, Node * );
        Object *onLess( vframe_t *, Node * );
        Object *onGreater( vframe_t *, Node * );
        Object *onGe( vframe_t *, Node * );
        Object *onLe( vframe_t *, Node * );
        Object *onNe( vframe_t *, Node * );
        Object *onEq( vframe_t *, Node * );
        Object *onLand( vframe_t *, Node * );
        Object *onLor( vframe_t *, Node * );

        Object *exec( vframe_t *frame, Node *node );
};


#endif
