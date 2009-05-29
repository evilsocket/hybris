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
	
    static Node *addInt( int value );
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

