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

enum H_NODE_TYPE {
    H_NT_NONE       = 0,
    H_NT_CONSTANT,
    H_NT_IDENTIFIER,
    H_NT_EXPRESSION,
    H_NT_STATEMENT,
    H_NT_FUNCTION,
    H_NT_CALL,
    H_NT_STRUCT,
    H_NT_ATTRIBUTE
};

/* pre declaration of class Node */
class  Node;

class NodeList : public list<Node *> {
    public :
        inline void head( Node *a, Node *b ){
            NodeList::iterator i;

            push_front(a);
            i = begin();
            i++;
            insert( i, b );
        }

        inline void head( Node *a ){
            push_front(a);
        }

        inline void tail( Node *a, Node *b ){
            push_back(a);
            push_back(b);
        }

        inline void tail( Node *a ){
            push_back(a);
        }
};

/* possible values for a generic node */
class NodeValue {
    public :

        Object *m_constant;
        string  m_identifier;
        int     m_expression;
        int     m_statement;
        Node   *m_switch;
        Node   *m_default;
        string  m_function;
        string  m_call;
        Node   *m_alias_call;

        NodeValue();
        ~NodeValue();
};

/* node base class */
class Node : public vector<Node *> {

private :

    H_NODE_TYPE  m_type;

public  :

    Node();
    Node( H_NODE_TYPE type );
    ~Node();

    NodeValue value;

    inline H_NODE_TYPE type(){
        return m_type;
    }

    inline unsigned int children(){
        return size();
    }

    inline Node *child( unsigned int i ){
        return at(i);
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
        StatementNode( int statement, Node *sw, NodeList *caselist );
        StatementNode( int statement, Node *sw, NodeList *caselist, Node *deflt );
};

/* identifiers */
class IdentifierNode : public Node {
    public :

        IdentifierNode( char *identifier );
};

/* structure attribute expression */
class AttributeNode : public Node {
    public :

        AttributeNode( NodeList *attrlist );
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

/* struct type definition */
class StructureNode : public Node {
    public :

        StructureNode( char *s_name, NodeList *attributes );
};

#endif

