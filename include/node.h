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

#include "types.h"
#include "common.h"
#include <vector>
#include <list>
#include <string>

using std::vector;
using std::list;
using std::string;

enum H_NODE_TYPE {
    H_NT_NONE        = 0,
    H_NT_CONSTANT    = 1,
    H_NT_IDENTIFIER  = 2,
    H_NT_EXPRESSION  = 3,
    H_NT_STATEMENT   = 4,
    H_NT_FUNCTION    = 5,
    H_NT_CALL        = 6,
    H_NT_METHOD_CALL = 7,
    H_NT_STRUCT      = 8,
    H_NT_ATTRIBUTE   = 9,
    H_NT_METHOD		 = 10,
    H_NT_CLASS		 = 11,
    H_NT_NEW	     = 12
};

enum access_t {
	asPublic = 0,
	asPrivate,
	asProtected
};

/* pre declaration of class Node */
class  Node;

class NodeList : public list<Node *> {
    public :
        __force_inline void head( Node *a, Node *b ){
            NodeList::iterator i;

            push_front(a);
            i = begin();
            i++;
            insert( i, b );
        }

        __force_inline void head( Node *a ){
            push_front(a);
        }

        __force_inline void tail( Node *a, Node *b ){
            push_back(a);
            push_back(b);
        }

        __force_inline void tail( Node *a ){
            push_back(a);
        }
};

typedef NodeList::iterator NodeIterator;

/* possible values for a generic node */
class NodeValue {
    public :

        Object  *m_constant;
        string   m_identifier;
        int      m_expression;
        int      m_statement;
        Node    *m_switch;
        Node    *m_default;
        string   m_function;
        access_t m_access;
        string   m_method;
        string   m_call;
        NodeList m_method_call;
        Node    *m_alias_call;

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

    __force_inline H_NODE_TYPE type(){
        return m_type;
    }

    __force_inline unsigned int children(){
        return size();
    }

    __force_inline Node *child( unsigned int i ){
        return at(i);
    }

    __force_inline Node *callBody(){
    	int i, sz( size() );
    	for( i = 0; i < sz; ++i ){
    		if( at(i)->type() != H_NT_IDENTIFIER ){
    			return at(i);
    		}
    	}
    	return NULL;
    }

    __force_inline int callDefinedArgc(){
    	int argc(0), i, sz( size() );
		for( i = 0; i < sz; ++i ){
			if( at(i)->type() == H_NT_IDENTIFIER ){
				++argc;
			}
		}
		return argc;
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
        IdentifierNode( access_t access, char *identifier );
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

/* structure or class creation */
class NewNode : public Node {
	public :

		NewNode( char *type, NodeList *argv );
};

/* struct type definition */
class StructureNode : public Node {
    public :

        StructureNode( char *s_name, NodeList *attributes );
};

/* method declarations */
class MethodNode : public Node {
	public :
		MethodNode( access_t access, method_decl_t *declaration, int argc, ... );
		MethodNode( const char *name, access_t access );
};

/* class type definition */
class ClassNode : public Node {
	public :
		NodeList m_extends;
		ClassNode( char *classname, NodeList *extends, NodeList *members );
};

/* method calls (a subset of StatementNode) */
class MethodCallNode : public Node {
    public :
		MethodCallNode( NodeList *mcall, NodeList *argv );
};

#endif

