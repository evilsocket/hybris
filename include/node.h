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

/*
 * Cast a linked list item to a Node pointer
 */
#define ll_node(ll_node_item) ll_data( Node *, ll_node_item )

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

/* possible values for a generic node */
class NodeValue {
    public :

        Object  *constant;
        string   identifier;

        Node    *switch_block;
        Node    *default_block;

        string   function;
        bool	 vargs;

        access_t access;
        bool	 is_static;
        string   method;

        string   call;
        Node    *alias;
        size_t	 argc;

        Node    *owner;
        Node    *member;

        Node	*try_block;
        string	 exception_id;
        Node    *catch_block;
        Node	*finally_block;

        llist_t  extends;

        NodeValue();
};

/* node base class */
class Node {

public  :

	size_t		 lineno;
	H_NODE_TYPE  type;
	int      	 opcode;
    Node		*body;
    llist_t		 children;
    NodeValue 	 value;

    Node();
    Node( H_NODE_TYPE type, size_t lineno );
    ~Node();

    INLINE void addChild( Node *child ){
    	ll_append( &children, child );
    }

    INLINE Node *child( size_t i ){
        switch(i){
			case 0 : return (Node *)children.head->data;
			case 1 : return (Node *)children.head->next->data;
			case 2 : return (Node *)children.head->next->next->data;
			case 3 : return (Node *)children.head->next->next->next->data;

			default :
				/*
				 * THIS SHOULD NEVER HAPPEN!
				 */
				assert( false );
        }
    }

    INLINE char *id(){
    	return (char *)value.identifier.c_str();
    }

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
        ExpressionNode( size_t lineno, int expression, llist_t *list );
        ExpressionNode( size_t lineno, int expression, int argc, ... );

        Node *clone();
};

/* statements */
class StatementNode : public Node {
    public :

        StatementNode( size_t lineno, int statement );
        StatementNode( size_t lineno, int statement, int argc, ... );
        StatementNode( size_t lineno, int statement, llist_t *identList, Node *expr );
        StatementNode( size_t lineno, int statement, Node *sw, llist_t *caselist );
        StatementNode( size_t lineno, int statement, Node *sw, llist_t *caselist, Node *deflt );

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

        CallNode( size_t lineno, char *name, llist_t *argv );
        CallNode( size_t lineno, Node *alias, llist_t *argv );

        Node *clone();
};

/* try/catch/finally statements */
class TryCatchNode : public Node {
public :

	TryCatchNode( size_t lineno, int statement, Node *try_block, char *exception_id, Node *catch_block, Node *finally_block );

	Node *clone();
};

/* structure or class creation */
class NewNode : public Node {
	public :

		NewNode( size_t lineno, char *type, llist_t *argv );

		Node *clone();
};

/* struct type definition */
class StructureNode : public Node {
    public :

        StructureNode( size_t lineno, char *s_name, llist_t *attributes );
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

		ClassNode( size_t lineno, char *classname, llist_t *extends, llist_t *members );
};

#endif

