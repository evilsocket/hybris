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
#ifndef _HNODE_H_
#	define _HNODE_H_


#include "common.h"
#include <vector>
#include <list>
#include <string>

using std::vector;
using std::list;
using std::string;

typedef unsigned short H_NODE_TYPE;

#define H_NT_NONE       0
#define H_NT_CONSTANT   1
#define H_NT_IDENTIFIER 2
#define H_NT_EXPRESSION 3
#define H_NT_STATEMENT  4
#define H_NT_FUNCTION   5
#define H_NT_CALL       6

/* pre declaration of class Node */
class  Node;

typedef list<Node *> NodeList;

/* possible values for a generic node */
class NodeValue {
    public :

        Object *m_constant;
        string  m_identifier;
        int     m_expression;
        int     m_statement;
        string  m_function;
        string  m_call;
        Node   *m_alias_call;

        NodeValue();
        ~NodeValue();
};

/* node base class */
class Node {

private :

    H_NODE_TYPE    m_type;
    vector<Node *> m_children;
    unsigned int   m_elements;

public  :

    Node();
    Node( H_NODE_TYPE type );
    ~Node();

    NodeValue value;

    inline H_NODE_TYPE type(){
        return m_type;
    }

    inline unsigned int children(){
        return m_elements;
    }

    inline Node *child( unsigned int i ){
        return m_children[i];
    }

    void addChild( Node *child );

    Node *clone();
};

/** specialized node classes **/

/* constants */
class ConstantNode : public Node {
    public :

        ConstantNode( long v );
        ConstantNode( double v );
        ConstantNode( char v );
        ConstantNode( char *v );
};

/* expressions */
class ExpressionNode : public Node {
    public :

        ExpressionNode( int expression );
        ExpressionNode( int expression, int argc, ... );
};

/* statements */
class StatementNode : public Node {
    public :

        StatementNode( int statement );
        StatementNode( int statement, int argc, ... );
};

/* identifiers */
class IdentifierNode : public Node {
    public :

        IdentifierNode( char *identifier );
};

/* function declarations */
class FunctionNode : public Node {
    public :

        FunctionNode( function_decl_t *declaration );
        FunctionNode( function_decl_t *declaration, int argc, ... );
        FunctionNode( const char *name );
};

/* function calls (a subset of StatementNode) */
class CallNode : public Node {
    public :

        CallNode( char *name, NodeList *argv );
        CallNode( Node *alias, NodeList *argv );
};

#endif

