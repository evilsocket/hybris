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
#include "memory.h"

NodeValue::NodeValue() :
    m_constant(NULL),
    m_identifier(""),
    m_expression(0),
    m_statement(0),
    m_function(""),
    m_method(""),
    m_access(asPublic),
    m_static(false),
    m_call(""),
    m_argc(0),
    m_alias_call(NULL),
    m_switch(NULL),
    m_default(NULL),
    m_owner(NULL),
    m_member(NULL),
    m_try_block(NULL),
    m_exp_id(NULL),
    m_catch_block(NULL),
    m_finally_block(NULL) {

}

NodeValue::~NodeValue(){

}

Node::Node() : m_type(H_NT_NONE), m_lineno(0) {

}

Node::Node( H_NODE_TYPE type, size_t lineno ) : m_type(type), m_lineno(lineno)
{

}

Node::~Node(){
    int i, sz( size() );

    for( i = 0; i < sz; ++i ){
        Node * child = at(i);
        delete child;
    }
    clear();
}

void Node::addChild( Node *child ){
	push_back(child);
}

Node *Node::clone(){
	/*
	 * THIS SHOULD NEVER HAPPEN!
	 */
	assert( false );
}

/* constants */
ConstantNode::ConstantNode( size_t lineno, long v ) : Node(H_NT_CONSTANT,lineno) {
    value.m_constant = (Object *)gc_new_integer(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, double v ) : Node(H_NT_CONSTANT,lineno) {
    value.m_constant = (Object *)gc_new_float(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, char v ) : Node(H_NT_CONSTANT,lineno) {
    value.m_constant = (Object *)gc_new_char(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, char *v ) : Node(H_NT_CONSTANT,lineno) {
    value.m_constant = (Object *)gc_new_string(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, bool v ) : Node(H_NT_CONSTANT,lineno) {
	value.m_constant = (Object *)gc_new_boolean(v);
	value.m_constant->attributes |= H_OA_CONSTANT;
}

Node *ConstantNode::clone() {
	if( ob_is_int(value.m_constant) ){
		return new ConstantNode( m_lineno, (ob_int_ucast(value.m_constant))->value );
	}
	else if( ob_is_float(value.m_constant) ){
		return new ConstantNode( m_lineno, ob_float_ucast(value.m_constant)->value );
	}
	else if( ob_is_char(value.m_constant) ){
		return new ConstantNode( m_lineno, ob_char_ucast(value.m_constant)->value );
	}
	else if( ob_is_string(value.m_constant) ){
		return new ConstantNode( m_lineno, (char *)ob_string_ucast(value.m_constant)->value.c_str() );
	}
	else if( ob_is_boolean(value.m_constant) ){
		return new ConstantNode( m_lineno, ob_bool_ucast(value.m_constant)->value );
	}
	else{
		/*
		 * THIS SHOULD NEVER HAPPEN!
		 */
		assert(false);
	}
}

/* expressions */
ExpressionNode::ExpressionNode( size_t lineno, int expression ) : Node(H_NT_EXPRESSION,lineno) {
    value.m_expression = expression;
}

ExpressionNode::ExpressionNode( size_t lineno, int expression, NodeList *list ) : Node(H_NT_EXPRESSION,lineno) {
    value.m_expression = expression;

    if( list != NULL ){
		reserve( ll_size(&list->llist) );
		ll_foreach( &list->llist, node ){
			push_back( (Node *)node->data );
		}
		delete list;
	}
}

ExpressionNode::ExpressionNode( size_t lineno, int expression, int argc, ... ) : Node(H_NT_EXPRESSION,lineno) {
    value.m_expression = expression;
    va_list ap;
	int i;

	reserve(argc);

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

Node *ExpressionNode::clone(){
	Node *clone = new ExpressionNode( m_lineno, value.m_expression, 0 );
	int  i, sz(size());
    for( i = 0; i < sz; ++i ){
    	if( child(i) ){
    		clone->push_back( child(i)->clone() );
    	}
    }
    return clone;
}

/* statements */
StatementNode::StatementNode( size_t lineno, int statement ) : Node(H_NT_STATEMENT,lineno) {
    value.m_statement = statement;
}

StatementNode::StatementNode( size_t lineno, int statement, int argc, ... ) : Node(H_NT_STATEMENT,lineno) {
    value.m_statement = statement;
    va_list ap;
	int i;

    reserve(argc);

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

StatementNode::StatementNode( size_t lineno, int statement, NodeList *identList, Node *expr ) : Node(H_NT_STATEMENT,lineno) {
    value.m_statement = statement;

    reserve( ll_size(&identList->llist) + 1 );

	push_back(expr);

	ll_foreach( &identList->llist, node ){
		push_back( (Node *)node->data );
	}

	delete identList;
}

StatementNode::StatementNode( size_t lineno, int statement, Node *sw, NodeList *caselist ) : Node(H_NT_STATEMENT,lineno) {
    value.m_statement = statement;
    value.m_switch    = sw;

    if( caselist != NULL ){
        reserve( ll_size(&caselist->llist) );
        ll_foreach( &caselist->llist, node ){
			push_back( (Node *)node->data );
		}
		delete caselist;
	}
}

StatementNode::StatementNode( size_t lineno, int statement, Node *sw, NodeList *caselist, Node *deflt ) : Node(H_NT_STATEMENT,lineno) {
    value.m_statement = statement;
    value.m_switch    = sw;
    value.m_default   = deflt;

    if( caselist != NULL ){
        reserve( ll_size(&caselist->llist) );
        ll_foreach( &caselist->llist, node ){
			push_back( (Node *)node->data );
		}
		delete caselist;
	}
}

Node *StatementNode::clone(){
	Node *clone = new StatementNode( m_lineno, value.m_statement, 0 );
	int   i, sz(size());

	clone->value.m_switch  = ( value.m_switch  ? value.m_switch->clone() : NULL );
	clone->value.m_default = ( value.m_default ? value.m_default->clone() : NULL );
	for( i = 0; i < sz; ++i ){
		if( child(i) ){
			clone->push_back( child(i)->clone() );
		}
	}

    return clone;
}

/* identifiers */
IdentifierNode::IdentifierNode( size_t lineno, char *identifier ) : Node(H_NT_IDENTIFIER,lineno) {
    value.m_identifier = identifier;
}

IdentifierNode::IdentifierNode( size_t lineno, access_t access, Node *i ) : Node(H_NT_IDENTIFIER,lineno) {
	assert( i->type() == H_NT_IDENTIFIER );

	value.m_access     = access;
	value.m_identifier = i->value.m_identifier;
}

IdentifierNode::IdentifierNode( size_t lineno, access_t access, char *identifier ) : Node(H_NT_IDENTIFIER,lineno) {
    value.m_access     = access;
    value.m_static	   = false;
	value.m_identifier = identifier;
}

IdentifierNode::IdentifierNode( size_t lineno, access_t access, bool is_static, char *identifier, Node *v ) : Node(H_NT_IDENTIFIER,lineno) {
    value.m_access     = access;
    value.m_static	   = is_static;
	value.m_identifier = identifier;

	push_back(v);
}

Node *IdentifierNode::clone(){
	Node *clone = new IdentifierNode( m_lineno, (char *)value.m_identifier.c_str() );

	clone->value.m_access = value.m_access;
    clone->value.m_static = value.m_static;

    return clone;
}

/* attribute request expression */
AttributeRequestNode::AttributeRequestNode( size_t lineno, Node *owner, Node *member ) : Node(H_NT_ATTRIBUTE,lineno) {
	value.m_owner  = owner;
	value.m_member = member;
}

Node *AttributeRequestNode::clone(){
	return new AttributeRequestNode( m_lineno, value.m_owner, value.m_member );
}

/* class method call */
MethodCallNode::MethodCallNode( size_t lineno, Node *owner, Node *call ) : Node(H_NT_METHOD_CALL,lineno) {
	value.m_owner  = owner;
	value.m_member = call;
}

Node *MethodCallNode::clone(){
	return new MethodCallNode( m_lineno, value.m_owner, value.m_member );
}

/* functions */
FunctionNode::FunctionNode( size_t lineno, function_decl_t *declaration ) : Node(H_NT_FUNCTION,lineno) {
    value.m_function = declaration->function;
    value.m_vargs 	 = declaration->vargs;
    value.m_argc	 = declaration->argc;

    reserve(value.m_argc);

	/* add function prototype args children */
	for( int i = 0; i < value.m_argc; ++i ){
		push_back( new IdentifierNode( m_lineno, declaration->argv[i] ) );
	}
}

FunctionNode::FunctionNode( size_t lineno, function_decl_t *declaration, int argc, ... ) : Node(H_NT_FUNCTION,lineno) {
    value.m_function = declaration->function;
    value.m_vargs 	 = declaration->vargs;
    value.m_argc	 = declaration->argc;

    va_list ap;
	int i;

    reserve( value.m_argc + argc );

	/* add function prototype args children */
	for( i = 0; i < value.m_argc; ++i ){
		push_back( new IdentifierNode( m_lineno, declaration->argv[i] ) );
	}
	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

FunctionNode::FunctionNode( size_t lineno, const char *name ) : Node(H_NT_FUNCTION,lineno) {
    value.m_function = name;
}

Node *FunctionNode::clone(){
	Node *clone = new FunctionNode( m_lineno, value.m_function.c_str() );
	int   i, sz(size());

	clone->value.m_argc  = value.m_argc;
	clone->value.m_vargs = value.m_vargs;
	for( i = 0; i < sz; ++i ){
		if( child(i) ){
			clone->push_back( child(i)->clone() );
		}
	}

	return clone;
}

/* function calls */
CallNode::CallNode( size_t lineno, char *name, NodeList *argv ) : Node(H_NT_CALL,lineno) {
    value.m_call = name;
    if( argv != NULL ){
        reserve( ll_size(&argv->llist) );
        ll_foreach( &argv->llist, node ){
			push_back( (Node *)node->data );
		}
		delete argv;
	}
}

CallNode::CallNode( size_t lineno, Node *alias, NodeList *argv ) :  Node(H_NT_CALL,lineno) {
    value.m_alias_call = alias;
    if( argv != NULL ){
        reserve( ll_size(&argv->llist) );
        ll_foreach( &argv->llist, node ){
			push_back( (Node *)node->data );
		}
		delete argv;
	}
}

Node *CallNode::clone(){
	Node *clone = H_UNDEFINED;
	int   i, sz(size());

    if( value.m_alias_call == NULL ){
        clone = new CallNode( m_lineno, (char *)value.m_call.c_str(), NULL );
    }
    else{
        clone = new CallNode( m_lineno, value.m_alias_call, NULL );
    }
    for( i = 0; i < sz; ++i ){
    	if( child(i) ){
    		clone->push_back( child(i)->clone() );
    	}
    }

    return clone;
}

/* try/catch/finally statements */
TryCatchNode::TryCatchNode( size_t lineno, int statement, Node *try_block, Node *exp_id, Node *catch_block, Node *finally_block ) : Node(H_NT_STATEMENT,lineno) {
	value.m_statement 	  = statement;
	value.m_try_block     = try_block;
	value.m_exp_id	      = exp_id;
	value.m_catch_block   = catch_block;
	value.m_finally_block = finally_block;
}

Node *TryCatchNode::clone(){
	return new TryCatchNode( m_lineno,
							 value.m_statement,
							 value.m_try_block,
							 value.m_exp_id,
							 value.m_catch_block,
							 value.m_finally_block );
}

/* structure or class creation */
NewNode::NewNode( size_t lineno, char *type, NodeList *argv ) : Node(H_NT_NEW,lineno){
	value.m_identifier = type;
	if( argv != NULL ){
        reserve( ll_size(&argv->llist) );
        ll_foreach( &argv->llist, node ){
			push_back( (Node *)node->data );
		}
		delete argv;
	}
}

Node *NewNode::clone(){
	Node *clone = new NewNode( m_lineno, (char *)value.m_identifier.c_str(), NULL );
	int   i, sz(size());

	for( i = 0; i < sz; ++i ){
		if( child(i) ){
			clone->push_back( child(i)->clone() );
		}
	}

	return clone;
}

/* struct type definition */
StructureNode::StructureNode( size_t lineno, char *s_name, NodeList *attributes ) : Node(H_NT_STRUCT,lineno) {
    value.m_identifier = s_name;
    if( attributes != NULL ){
        reserve( ll_size(&attributes->llist) );
        ll_foreach( &attributes->llist, node ){
			push_back( (Node *)node->data );
		}
		delete attributes;
	}
}

/* methods */
MethodDeclarationNode::MethodDeclarationNode( size_t lineno, access_t access, method_decl_t *declaration, int argc, ... ) : Node(H_NT_METHOD_DECL,lineno) {
    value.m_method = declaration->method;
    value.m_vargs  = declaration->vargs;
    value.m_argc   = declaration->argc;
    value.m_access = access;

    va_list ap;
	int i;

    reserve( value.m_argc + argc );
	/* add method prototype args children */
	for( i = 0; i < value.m_argc; ++i ){
		push_back( new IdentifierNode( m_lineno, declaration->argv[i] ) );
	}
	/* add method body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

MethodDeclarationNode::MethodDeclarationNode( size_t lineno, access_t access, method_decl_t *declaration, bool is_static, int argc, ... ) : Node(H_NT_METHOD_DECL,lineno) {
    value.m_method = declaration->method;
    value.m_vargs  = declaration->vargs;
    value.m_argc   = declaration->argc;
    value.m_access = access;
    value.m_static = is_static;

    va_list ap;
	int i;

    reserve( value.m_argc + argc );
	/* add method prototype args children */
	for( i = 0; i < value.m_argc; ++i ){
		push_back( new IdentifierNode( m_lineno, declaration->argv[i] ) );
	}
	/* add method body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

MethodDeclarationNode::MethodDeclarationNode( size_t lineno, const char *name, access_t access ) : Node(H_NT_METHOD_DECL,lineno) {
	value.m_method = name;
	value.m_access = access;
}

Node *MethodDeclarationNode::clone(){
	Node *clone = new MethodDeclarationNode( m_lineno, value.m_method.c_str(), value.m_access );
	int   i, sz(size());

	clone->value.m_static = value.m_static;
	clone->value.m_vargs  = value.m_vargs;
	clone->value.m_argc	  = value.m_argc;
	for( i = 0; i < sz; ++i ){
		if( child(i) ){
			clone->push_back( child(i)->clone() );
		}
	}
	return clone;
}

/* class type definition */
ClassNode::ClassNode( size_t lineno, char *classname, NodeList *extends, NodeList *members ) : Node(H_NT_CLASS,lineno) {
	value.m_identifier = classname;
	if( extends != NULL ){
        ll_foreach( &extends->llist, node ){
			m_extends.tail( (Node *)node->data );
		}
        delete extends;
	}
	if( members != NULL ){
        reserve( ll_size(&members->llist) );
        ll_foreach( &members->llist, node ){
			push_back( (Node *)node->data );
		}
		delete members;
	}
}

