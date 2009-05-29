#ifndef _HNODE_H_
#	define _HNODE_H_

#include "common.h"
#include <vector>
#include <list>

using std::vector;
using std::list;

typedef unsigned short H_NODE_TYPE;

#define H_NT_NONE       0
#define H_NT_CONSTANT   1
#define H_NT_IDENTIFIER 2
#define H_NT_OPERATOR   3
#define H_NT_FUNCTION   4
#define H_NT_CALL       5

class Node {

private :

    H_NODE_TYPE     m_type;
    vector<Node *> m_children;

public  :

    Node( H_NODE_TYPE type );
    Node( FILE *fp );

    ~Node();

    H_NODE_TYPE type();

    Object       *_constant;
    char          _identifier[0xFF];
    int           _operator;
    char          _function[0xFF];
    char          _call[0xFF];
	Node         *_aliascall;

    unsigned int children();
    Node *child( unsigned int i );
    void addChild( Node *child );
};

#endif

