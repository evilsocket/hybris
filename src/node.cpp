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
#include "engine.h"

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

Node::Node() : m_type(H_NT_NONE) {

}

Node::Node( H_NODE_TYPE type ) : m_type(type)
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
	printf( "%d\n", m_type );
	assert( false );
}

/* constants */
ConstantNode::ConstantNode( long v ) : Node(H_NT_CONSTANT) {
    value.m_constant = (Object *)gc_new_integer(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( double v ) : Node(H_NT_CONSTANT) {
    value.m_constant = (Object *)gc_new_float(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( char v ) : Node(H_NT_CONSTANT) {
    value.m_constant = (Object *)gc_new_char(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( char *v ) : Node(H_NT_CONSTANT) {
    value.m_constant = (Object *)gc_new_string(v);
    value.m_constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( bool v ) : Node(H_NT_CONSTANT) {
	value.m_constant = (Object *)gc_new_boolean(v);
	value.m_constant->attributes |= H_OA_CONSTANT;
}

Node *ConstantNode::clone() {
	if( ob_is_int(value.m_constant) ){
		return new ConstantNode( (ob_int_ucast(value.m_constant))->value );
	}
	else if( ob_is_float(value.m_constant) ){
		return new ConstantNode( ob_float_ucast(value.m_constant)->value );
	}
	else if( ob_is_char(value.m_constant) ){
		return new ConstantNode( ob_char_ucast(value.m_constant)->value );
	}
	else if( ob_is_string(value.m_constant) ){
		return new ConstantNode( (char *)ob_string_ucast(value.m_constant)->value.c_str() );
	}
	else if( ob_is_boolean(value.m_constant) ){
		return new ConstantNode( ob_bool_ucast(value.m_constant)->value );
	}
	else{
		/*
		 * THIS SHOULD NEVER HAPPEN!
		 */
		assert(false);
	}
}

/* expressions */
ExpressionNode::ExpressionNode( int expression ) : Node(H_NT_EXPRESSION) {
    value.m_expression = expression;
}

ExpressionNode::ExpressionNode( int expression, NodeList *list ) : Node(H_NT_EXPRESSION) {
    value.m_expression = expression;

    if( list != NULL ){
		reserve( list->size() );
		for( NodeList::iterator ni = list->begin(); ni != list->end(); ni++ ){
			push_back( *ni );
		}
		delete list;
	}
}

ExpressionNode::ExpressionNode( int expression, int argc, ... ) : Node(H_NT_EXPRESSION) {
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
	Node *clone = new ExpressionNode( value.m_expression, 0 );
	int   i, sz(size());
    for( int i = 0; i < sz; ++i ){
    	if( child(i) ){
    		clone->push_back( child(i)->clone() );
    	}
    }
    return clone;
}

/* statements */
StatementNode::StatementNode( int statement ) : Node(H_NT_STATEMENT) {
    value.m_statement = statement;
}

StatementNode::StatementNode( int statement, int argc, ... ) : Node(H_NT_STATEMENT) {
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

StatementNode::StatementNode( int statement, NodeList *identList, Node *expr ) : Node(H_NT_STATEMENT) {
    value.m_statement = statement;
    va_list ap;
	int i;

	reserve( identList->size() + 1 );

	push_back(expr);

	for( NodeList::iterator ni = identList->begin(); ni != identList->end(); ni++ ){
		push_back( *ni );
	}

	delete identList;
}

StatementNode::StatementNode( int statement, Node *sw, NodeList *caselist ) : Node(H_NT_STATEMENT) {
    value.m_statement = statement;
    value.m_switch    = sw;

    if( caselist != NULL ){
        reserve( caselist->size() );
		for( NodeList::iterator ni = caselist->begin(); ni != caselist->end(); ni++ ){
			push_back( *ni );
		}
		delete caselist;
	}
}

StatementNode::StatementNode( int statement, Node *sw, NodeList *caselist, Node *deflt ) : Node(H_NT_STATEMENT) {
    value.m_statement = statement;
    value.m_switch    = sw;
    value.m_default   = deflt;

    if( caselist != NULL ){
        reserve( caselist->size() );
		for( NodeList::iterator ni = caselist->begin(); ni != caselist->end(); ni++ ){
			push_back( *ni );
		}
		delete caselist;
	}
}

Node *StatementNode::clone(){
	Node *clone = new StatementNode( value.m_statement, 0 );
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
IdentifierNode::IdentifierNode( char *identifier ) : Node(H_NT_IDENTIFIER) {
    value.m_identifier = identifier;
}

IdentifierNode::IdentifierNode( access_t access, Node *i ) : Node(H_NT_IDENTIFIER) {
	assert( i->type() == H_NT_IDENTIFIER );

	value.m_access     = access;
	value.m_identifier = i->value.m_identifier;
}

IdentifierNode::IdentifierNode( access_t access, char *identifier ) : Node(H_NT_IDENTIFIER) {
    value.m_access     = access;
    value.m_static	   = false;
	value.m_identifier = identifier;
}

IdentifierNode::IdentifierNode( access_t access, bool is_static, char *identifier, Node *v ) : Node(H_NT_IDENTIFIER) {
    value.m_access     = access;
    value.m_static	   = is_static;
	value.m_identifier = identifier;

	push_back(v);
}

Node *IdentifierNode::clone(){
	Node *clone = new IdentifierNode( (char *)value.m_identifier.c_str() );
	int   i, sz(size());

	clone->value.m_access = value.m_access;
    clone->value.m_static = value.m_static;

    return clone;
}

/* attribute request expression */
AttributeRequestNode::AttributeRequestNode( Node *owner, Node *member ) : Node(H_NT_ATTRIBUTE) {
	value.m_owner  = owner;
	value.m_member = member;
}

Node *AttributeRequestNode::clone(){
	return new AttributeRequestNode( value.m_owner, value.m_member );
}

/* class method call */
MethodCallNode::MethodCallNode( Node *owner, Node *call ) : Node(H_NT_METHOD_CALL) {
	value.m_owner  = owner;
	value.m_member = call;
}

Node *MethodCallNode::clone(){
	return new MethodCallNode( value.m_owner, value.m_member );
}

/* functions */
FunctionNode::FunctionNode( function_decl_t *declaration ) : Node(H_NT_FUNCTION) {
    value.m_function = declaration->function;
    value.m_vargs 	 = declaration->vargs;

    reserve(declaration->argc);

	/* add function prototype args children */
	for( int i = 0; i < declaration->argc; ++i ){
		push_back( new IdentifierNode( declaration->argv[i] ) );
	}
}

FunctionNode::FunctionNode( function_decl_t *declaration, int argc, ... ) : Node(H_NT_FUNCTION) {
    value.m_function = declaration->function;
    value.m_vargs 	 = declaration->vargs;

    va_list ap;
	int i;

    reserve( declaration->argc + argc );

	/* add function prototype args children */
	for( i = 0; i < declaration->argc; ++i ){
		push_back( new IdentifierNode( declaration->argv[i] ) );
	}
	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

FunctionNode::FunctionNode( const char *name ) : Node(H_NT_FUNCTION) {
    value.m_function = name;
}

Node *FunctionNode::clone(){
	Node *clone = new FunctionNode( value.m_function.c_str() );
	int   i, sz(size());

	clone->value.m_vargs = value.m_vargs;
	for( i = 0; i < sz; ++i ){
		if( child(i) ){
			clone->push_back( child(i)->clone() );
		}
	}

	return clone;
}

/* function calls */
CallNode::CallNode( char *name, NodeList *argv ) : Node(H_NT_CALL) {
    value.m_call = name;
    if( argv != NULL ){
        reserve( argv->size() );
		for( NodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			push_back( *ni );
		}
		delete argv;
	}
}

CallNode::CallNode( Node *alias, NodeList *argv ) :  Node(H_NT_CALL) {
    value.m_alias_call = alias;
    if( argv != NULL ){
        reserve( argv->size() );
		for( NodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			push_back( *ni );
		}
		delete argv;
	}
}

Node *CallNode::clone(){
	Node *clone = H_UNDEFINED;
	int   i, sz(size());

    if( value.m_alias_call == NULL ){
        clone = new CallNode( (char *)value.m_call.c_str(), NULL );
    }
    else{
        clone = new CallNode( value.m_alias_call, NULL );
    }
    for( i = 0; i < sz; ++i ){
    	if( child(i) ){
    		clone->push_back( child(i)->clone() );
    	}
    }

    return clone;
}

/* try/catch/finally statements */
TryCatchNode::TryCatchNode( int statement, Node *try_block, Node *exp_id, Node *catch_block, Node *finally_block ) : Node(H_NT_STATEMENT) {
	value.m_statement 	  = statement;
	value.m_try_block     = try_block;
	value.m_exp_id	      = exp_id;
	value.m_catch_block   = catch_block;
	value.m_finally_block = finally_block;
}

Node *TryCatchNode::clone(){
	return new TryCatchNode( value.m_statement,
							 value.m_try_block,
							 value.m_exp_id,
							 value.m_catch_block,
							 value.m_finally_block );
}

/* structure or class creation */
NewNode::NewNode( char *type, NodeList *argv ) : Node(H_NT_NEW){
	value.m_identifier = type;
	if( argv != NULL ){
		reserve( argv->size() );
		for( NodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			push_back( *ni );
		}
		delete argv;
	}
}

Node *NewNode::clone(){
	Node *clone = new NewNode( (char *)value.m_identifier.c_str(), NULL );
	int   i, sz(size());

	for( i = 0; i < sz; ++i ){
		if( child(i) ){
			clone->push_back( child(i)->clone() );
		}
	}

	return clone;
}

/* struct type definition */
StructureNode::StructureNode( char *s_name, NodeList *attributes ) : Node(H_NT_STRUCT) {
    value.m_identifier = s_name;
    if( attributes != NULL ){
        reserve( attributes->size() );
		for( NodeList::iterator ni = attributes->begin(); ni != attributes->end(); ni++ ){
			push_back( *ni );
		}
		delete attributes;
	}
}

/* methods */
MethodDeclarationNode::MethodDeclarationNode( access_t access, method_decl_t *declaration, int argc, ... ) : Node(H_NT_METHOD_DECL) {
    value.m_method = declaration->method;
    value.m_vargs  = declaration->vargs;
    value.m_access = access;

    va_list ap;
	int i;

    reserve( declaration->argc + argc );
	/* add method prototype args children */
	for( i = 0; i < declaration->argc; ++i ){
		push_back( new IdentifierNode( declaration->argv[i] ) );
	}
	/* add method body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

MethodDeclarationNode::MethodDeclarationNode( access_t access, method_decl_t *declaration, bool is_static, int argc, ... ) : Node(H_NT_METHOD_DECL) {
    value.m_method = declaration->method;
    value.m_vargs  = declaration->vargs;
    value.m_access = access;
    value.m_static = is_static;

    va_list ap;
	int i;

    reserve( declaration->argc + argc );
	/* add method prototype args children */
	for( i = 0; i < declaration->argc; ++i ){
		push_back( new IdentifierNode( declaration->argv[i] ) );
	}
	/* add method body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		push_back( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

MethodDeclarationNode::MethodDeclarationNode( const char *name, access_t access ) : Node(H_NT_METHOD_DECL) {
	value.m_method = name;
	value.m_access = access;
}

Node *MethodDeclarationNode::clone(){
	Node *clone = new MethodDeclarationNode( value.m_method.c_str(), value.m_access );
	int   i, sz(size());

	clone->value.m_static = value.m_static;
	clone->value.m_vargs  = value.m_vargs;
	for( i = 0; i < sz; ++i ){
		if( child(i) ){
			clone->push_back( child(i)->clone() );
		}
	}
	return clone;
}

/* class type definition */
ClassNode::ClassNode( char *classname, NodeList *extends, NodeList *members ) : Node(H_NT_CLASS) {
	value.m_identifier = classname;
	if( extends != NULL ){
		for( NodeList::iterator ni = extends->begin(); ni != extends->end(); ni++ ){
			m_extends.push_back( *ni );
		}
	}
	if( members != NULL ){
		reserve( members->size() );
		for( NodeList::iterator ni = members->begin(); ni != members->end(); ni++ ){
			push_back( *ni );
		}
		delete members;
	}
}

