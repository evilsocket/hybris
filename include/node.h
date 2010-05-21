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
    H_NT_STRUCT      = 7,
    H_NT_ATTRIBUTE   = 8,
    H_NT_METHOD_CALL = 9,
    H_NT_METHOD_DECL = 10,
    H_NT_CLASS		 = 11,
    H_NT_NEW	     = 12
};

#ifndef H_ACCESS_SPECIFIER
	enum access_t {
		asPublic = 0,
		asPrivate,
		asProtected
	};
#	define H_ACCESS_SPECIFIER
#endif

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
        bool	 m_vargs;
        access_t m_access;
        bool	 m_static;
        string   m_method;
        string   m_call;
        Node    *m_alias_call;
        Node    *m_owner;
        Node    *m_member;

        Node	*m_try_block;
        Node	*m_exp_id;
        Node    *m_catch_block;
        Node	*m_finally_block;

        NodeValue();
        ~NodeValue();
};

/* node base class */
class Node : public vector<Node *> {

protected :

    H_NODE_TYPE  m_type;
    size_t		 m_lineno;

public  :

    Node();
    Node( H_NODE_TYPE type, size_t lineno );
    ~Node();

    NodeValue value;

    __force_inline H_NODE_TYPE type(){
        return m_type;
    }

    __force_inline size_t lineno(){
		return m_lineno;
	}

    __force_inline unsigned int children(){
        return size();
    }

    __force_inline Node *child( unsigned int i ){
        return (*this)[i];
    }

    __force_inline char *id(){
    	return (char *)value.m_identifier.c_str();
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

    __force_inline size_t callDefinedArgc(){
    	size_t argc(0), i, sz( size() );
		for( i = 0; i < sz; ++i ){
			if( at(i)->type() == H_NT_IDENTIFIER ){
				++argc;
			}
		}
		return argc;
    }

    void addChild( Node *child );

    virtual Node *clone();
};

/** specialized node classes **/

/* constants */
class ConstantNode : public Node {
    public :

        ConstantNode( size_t lineno, long v );
        ConstantNode( size_t lineno, double v );
        ConstantNode( size_t lineno, char v );
        ConstantNode( size_t lineno, char *v );
        ConstantNode( size_t lineno, bool v );

        Node *clone();
};

/* expressions */
class ExpressionNode : public Node {
    public :

        ExpressionNode( size_t lineno, int expression );
        ExpressionNode( size_t lineno, int expression, NodeList *list );
        ExpressionNode( size_t lineno, int expression, int argc, ... );

        Node *clone();
};

/* statements */
class StatementNode : public Node {
    public :

        StatementNode( size_t lineno, int statement );
        StatementNode( size_t lineno, int statement, int argc, ... );
        StatementNode( size_t lineno, int statement, NodeList *identList, Node *expr );
        StatementNode( size_t lineno, int statement, Node *sw, NodeList *caselist );
        StatementNode( size_t lineno, int statement, Node *sw, NodeList *caselist, Node *deflt );

        Node *clone();
};

/* identifiers */
class IdentifierNode : public Node {
    public :

        IdentifierNode( size_t lineno, char *identifier );
        IdentifierNode( size_t lineno, access_t access, Node *i );
        IdentifierNode( size_t lineno, access_t access, char *identifier );
        IdentifierNode( size_t lineno, access_t access, bool is_static, char *identifier, Node *v );

        Node *clone();
};

/* attribute request expression */
class AttributeRequestNode : public Node {
    public :

		AttributeRequestNode( size_t lineno, Node *owner, Node *member );

		Node *clone();
};

/* class method call */
class MethodCallNode : public Node {
    public :

		MethodCallNode( size_t lineno, Node *owner, Node *call );

		Node *clone();
};

/* function declarations */
class FunctionNode : public Node {
    public :

        FunctionNode( size_t lineno, function_decl_t *declaration );
        FunctionNode( size_t lineno, function_decl_t *declaration, int argc, ... );
        FunctionNode( size_t lineno, const char *name );

        Node *clone();
};

/* function calls (a subset of StatementNode) */
class CallNode : public Node {
    public :

        CallNode( size_t lineno, char *name, NodeList *argv );
        CallNode( size_t lineno, Node *alias, NodeList *argv );

        Node *clone();
};

/* try/catch/finally statements */
class TryCatchNode : public Node {
public :

	TryCatchNode( size_t lineno, int statement, Node *try_block, Node	*exp_id, Node *catch_block, Node *finally_block );

	Node *clone();
};

/* structure or class creation */
class NewNode : public Node {
	public :

		NewNode( size_t lineno, char *type, NodeList *argv );

		Node *clone();
};

/* struct type definition */
class StructureNode : public Node {
    public :

        StructureNode( size_t lineno, char *s_name, NodeList *attributes );
};

/* method declarations */
class MethodDeclarationNode : public Node {
	public :
		MethodDeclarationNode( size_t lineno, access_t access, method_decl_t *declaration, int argc, ... );
		MethodDeclarationNode( size_t lineno, access_t access, method_decl_t *declaration, bool is_static, int argc, ... );
		MethodDeclarationNode( size_t lineno, const char *name, access_t access );

		Node *clone();
};

/* class type definition */
class ClassNode : public Node {
	public :
		NodeList m_extends;
		ClassNode( size_t lineno, char *classname, NodeList *extends, NodeList *members );
};

#endif

