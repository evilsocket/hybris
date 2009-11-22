/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _HTREE_H_
#	define _HTREE_H_

#include <vector>
#include <list>
#include "node.h"

using std::vector;
using std::list;

typedef list<Node *> HNodeList;

class Tree {
public :

    static HNodeList *createList();

    static Node *addInt( long value );
    static Node *addFloat( double value );
    static Node *addChar( char value );
    static Node *addString( char *value );
    static Node *addOperator( int op, int argc, ... );
    static Node *addIdentifier( char *id );
    static Node *addFunction( function_decl_t *declaration, int argc, ... );
    static Node *addFunction( char *name, int argc, ... );
    static Node *addCall( char *name, HNodeList *argv );
	static Node *addCall( Node *alias, HNodeList *argv );

    static const char *type( Node *node );
    static void print( Node *node, int tabs = 0 );
    static void compile( Node *node, FILE *fp );
    static Node * load( FILE *fp );
    static Node *clone( Node *root, Node *clone );
    static void release( Node *node );
};

#endif

