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
#include "node.h"
#include "vmem.h"
#include "engine.h"

NodeValue::NodeValue() :
    m_constant(NULL),
    m_identifier(""),
    m_expression(0),
    m_statement(0),
    m_function(""),
    m_call(""),
    m_alias_call(NULL) {

}

NodeValue::~NodeValue(){

}

Node::Node() :
    m_type(H_NT_NONE),
    m_elements(0) {

}

Node::Node( H_NODE_TYPE type ) :
    m_type(type),
    m_elements(0)
{

}

Node::~Node(){
    if( m_elements > 0 ){
        int i;
        for( i = 0; i < m_elements; ++i ){
            Node * child = m_children[i];
            delete child;
        }
        m_children.clear();
    }
    if( value.m_constant != NULL ){
        value.m_constant->release();
	}
}

void Node::addChild( Node *child ){
	m_children.push_back(child);
	m_elements++;
}

Node *Node::clone(){
    int   i;
    Node *clone = H_UNDEFINED;

    switch( m_type ){
        case H_NT_CONSTANT   :
            switch( value.m_constant->xtype ){
                case H_OT_INT    : clone = new ConstantNode( value.m_constant->xint );                    break;
                case H_OT_FLOAT  : clone = new ConstantNode( value.m_constant->xfloat );                  break;
                case H_OT_CHAR   : clone = new ConstantNode( value.m_constant->xchar );                   break;
                case H_OT_STRING : clone = new ConstantNode( (char *)value.m_constant->xstring.c_str() ); break;
            }
        break;

        case H_NT_IDENTIFIER :
            clone = new IdentifierNode( (char *)value.m_identifier.c_str() );
        break;

        case H_NT_EXPRESSION   :
            clone = new ExpressionNode( value.m_expression, 0 );
            for( i = 0; i < m_elements; ++i ){
                clone->addChild( child(i)->clone() );
            }
        break;

        case H_NT_STATEMENT  :
            clone = new StatementNode( value.m_statement, 0 );
            for( i = 0; i < m_elements; ++i ){
                clone->addChild( child(i)->clone() );
            }
        break;

        case H_NT_FUNCTION   :
            clone = new FunctionNode( value.m_function.c_str() );
            for( i = 0; i < m_elements; ++i ){
                clone->addChild( child(i)->clone() );
            }
        break;

        case H_NT_CALL       :
            if( value.m_alias_call == NULL ){
                clone = new CallNode( (char *)value.m_call.c_str(), NULL );
            }
            else{
                clone = new CallNode( value.m_alias_call, NULL );
            }
            for( i = 0; i < m_elements; ++i ){
                clone->addChild( child(i)->clone() );
            }
        break;
    }

	return clone;
}

/* constants */
ConstantNode::ConstantNode( long v ) : Node(H_NT_CONSTANT) {
    value.m_constant = new Object(v);
    #ifdef GC_SUPPORT
    value.m_constant->attributes |= H_OA_CONSTANT;
    value.m_constant->attributes &= ~H_OA_GARBAGE;
	#endif
}

ConstantNode::ConstantNode( double v ) : Node(H_NT_CONSTANT) {
    value.m_constant = new Object(v);
    #ifdef GC_SUPPORT
    value.m_constant->attributes |= H_OA_CONSTANT;
    value.m_constant->attributes &= ~H_OA_GARBAGE;
	#endif
}

ConstantNode::ConstantNode( char v ) : Node(H_NT_CONSTANT) {
    value.m_constant = new Object(v);
    #ifdef GC_SUPPORT
    value.m_constant->attributes |= H_OA_CONSTANT;
    value.m_constant->attributes &= ~H_OA_GARBAGE;
	#endif
}

ConstantNode::ConstantNode( char *v ) : Node(H_NT_CONSTANT) {
    value.m_constant = new Object(v);
    #ifdef GC_SUPPORT
    value.m_constant->attributes |= H_OA_CONSTANT;
    value.m_constant->attributes &= ~H_OA_GARBAGE;
	#endif
}

/* expressions */
ExpressionNode::ExpressionNode( int expression ) : Node(H_NT_EXPRESSION) {
    value.m_expression = expression;
}

ExpressionNode::ExpressionNode( int expression, int argc, ... ) : Node(H_NT_EXPRESSION) {
    value.m_expression = expression;
    va_list ap;
	int i;

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

/* statements */
StatementNode::StatementNode( int statement ) : Node(H_NT_STATEMENT) {
    value.m_statement = statement;
}

StatementNode::StatementNode( int statement, int argc, ... ) : Node(H_NT_STATEMENT) {
    value.m_statement = statement;
    va_list ap;
	int i;

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

/* identifiers */
IdentifierNode::IdentifierNode( char *identifier ) : Node(H_NT_IDENTIFIER) {
    value.m_identifier = identifier;
}

/* functions */
FunctionNode::FunctionNode( function_decl_t *declaration ) : Node(H_NT_FUNCTION) {
    value.m_function = declaration->function;
	/* add function prototype args children */
	for( int i = 0; i < declaration->argc; ++i ){
		addChild( new IdentifierNode( declaration->argv[i] ) );
	}
}

FunctionNode::FunctionNode( function_decl_t *declaration, int argc, ... ) : Node(H_NT_FUNCTION) {
    value.m_function = declaration->function;
    va_list ap;
	int i;

	/* add function prototype args children */
	for( i = 0; i < declaration->argc; ++i ){
		addChild( new IdentifierNode( declaration->argv[i] ) );
	}
	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

FunctionNode::FunctionNode( const char *name ) : Node(H_NT_FUNCTION) {
    value.m_function = name;
}

/* function calls */
CallNode::CallNode( char *name, NodeList *argv ) : Node(H_NT_CALL) {
    value.m_call = name;
    if( argv != NULL ){
		for( NodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			addChild( *ni );
		}
		delete argv;
	}
}

CallNode::CallNode( Node *alias, NodeList *argv ) :  Node(H_NT_CALL) {
    value.m_alias_call = alias;
    if( argv != NULL ){
		for( NodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			addChild( *ni );
		}
		delete argv;
	}
}




