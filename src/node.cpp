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
    if( value.m_constant != NULL ){
        value.m_constant->release();
	}
}

void Node::addChild( Node *child ){
	m_children.push_back(child);
	m_elements++;
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

/* statements */
StatementNode::StatementNode( int statement ): Node(H_NT_STATEMENT) {
    value.m_statement = statement;
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

FunctionNode::FunctionNode( char *name ) : Node(H_NT_FUNCTION) {
    value.m_function = name;
}

/* function calls */
CallNode::CallNode( char *name ) : Node(H_NT_CALL) {
    value.m_call = name;
}

CallNode::CallNode( Node *alias ):  Node(H_NT_CALL) {
    value.m_alias_call = alias;
}




