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
    constant(NULL),
    identifier(""),
    function(""),
    method(""),
    access(asPublic),
    is_static(false),
    call(""),
    argc(0),
    alias(NULL),
    switch_block(NULL),
    default_block(NULL),
    owner(NULL),
    member(NULL),
    try_block(NULL),
    exception_id(""),
    catch_block(NULL),
    finally_block(NULL) {

}

Node::Node() : type(H_NT_NONE), lineno(0), body(NULL) {
	ll_init( &children );
}

Node::Node( H_NODE_TYPE type, size_t lineno ) : type(type), opcode(type), lineno(lineno), body(NULL) {
	ll_init( &children );
}

Node::~Node(){
	ll_foreach( &children, child ){
		delete ll_node( child );
	}
	ll_clear( &children );
}

Node *Node::clone(){
	/*
	 * THIS SHOULD NEVER HAPPEN!
	 */
	assert( false );
}

/* constants */
ConstantNode::ConstantNode( size_t lineno, long v ) : Node(H_NT_CONSTANT,lineno) {
    value.constant = (Object *)gc_new_integer(v);
    value.constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, double v ) : Node(H_NT_CONSTANT,lineno) {
    value.constant = (Object *)gc_new_float(v);
    value.constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, char v ) : Node(H_NT_CONSTANT,lineno) {
    value.constant = (Object *)gc_new_char(v);
    value.constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, char *v ) : Node(H_NT_CONSTANT,lineno) {
    value.constant = (Object *)gc_new_string(v);
    value.constant->attributes |= H_OA_CONSTANT;
}

ConstantNode::ConstantNode( size_t lineno, bool v ) : Node(H_NT_CONSTANT,lineno) {
	value.constant = (Object *)gc_new_boolean(v);
	value.constant->attributes |= H_OA_CONSTANT;
}

Node *ConstantNode::clone() {
	if( ob_is_int(value.constant) ){
		return new ConstantNode( lineno, (ob_int_ucast(value.constant))->value );
	}
	else if( ob_is_float(value.constant) ){
		return new ConstantNode( lineno, ob_float_ucast(value.constant)->value );
	}
	else if( ob_is_char(value.constant) ){
		return new ConstantNode( lineno, ob_char_ucast(value.constant)->value );
	}
	else if( ob_is_string(value.constant) ){
		return new ConstantNode( lineno, (char *)ob_string_ucast(value.constant)->value.c_str() );
	}
	else if( ob_is_boolean(value.constant) ){
		return new ConstantNode( lineno, ob_bool_ucast(value.constant)->value );
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
    opcode = expression;
}

ExpressionNode::ExpressionNode( size_t lineno, int expression, llist_t *list ) : Node(H_NT_EXPRESSION,lineno) {
    opcode = expression;

    if( list != NULL ){
    	ll_merge_destroy( &children, list );
	}
}

ExpressionNode::ExpressionNode( size_t lineno, int expression, int argc, ... ) : Node(H_NT_EXPRESSION,lineno) {
    opcode = expression;
    va_list ap;
	int i;

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

Node *ExpressionNode::clone(){
	Node *clone = new ExpressionNode( lineno, opcode, 0 ),
		 *node;

	ll_foreach( &children, nitem ){
		node = ll_node(nitem);
		clone->addChild( node ? node->clone() : node );
	}

    return clone;
}

/* statements */
StatementNode::StatementNode( size_t lineno, int statement ) : Node(H_NT_STATEMENT,lineno) {
    opcode = statement;
}

StatementNode::StatementNode( size_t lineno, int statement, int argc, ... ) : Node(H_NT_STATEMENT,lineno) {
    opcode = statement;
    va_list ap;
	int i;

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

StatementNode::StatementNode( size_t lineno, int statement, llist_t *identList, Node *expr ) : Node(H_NT_STATEMENT,lineno) {
    opcode = statement;

	addChild(expr);

	if( identList ){
		ll_merge_destroy( &children, identList );
	}
}

StatementNode::StatementNode( size_t lineno, int statement, Node *sw, llist_t *caselist ) : Node(H_NT_STATEMENT,lineno) {
    opcode 			   = statement;
    value.switch_block = sw;

    if( caselist != NULL ){
    	ll_merge_destroy( &children, caselist );
	}
}

StatementNode::StatementNode( size_t lineno, int statement, Node *sw, llist_t *caselist, Node *deflt ) : Node(H_NT_STATEMENT,lineno) {
    opcode 				  = statement;
    value.switch_block    = sw;
    value.default_block   = deflt;

    if( caselist != NULL ){
    	ll_merge_destroy( &children, caselist );
	}
}

Node *StatementNode::clone(){
	Node *clone = new StatementNode( lineno, opcode, 0 ),
		 *node;

	clone->value.switch_block  = ( value.switch_block  ? value.switch_block->clone() : NULL );
	clone->value.default_block = ( value.default_block ? value.default_block->clone() : NULL );

	ll_foreach( &children, nitem ){
		node = ll_node(nitem);
		clone->addChild( node ? node->clone() : node );
	}

    return clone;
}

/* identifiers */
IdentifierNode::IdentifierNode( size_t lineno, char *identifier ) : Node(H_NT_IDENTIFIER,lineno) {
    value.identifier = identifier;
}

IdentifierNode::IdentifierNode( size_t lineno, access_t access, Node *i ) : Node(H_NT_IDENTIFIER,lineno) {
	assert( i->type == H_NT_IDENTIFIER );

	value.access     = access;
	value.identifier = i->value.identifier;
}

IdentifierNode::IdentifierNode( size_t lineno, access_t access, char *identifier ) : Node(H_NT_IDENTIFIER,lineno) {
    value.access     = access;
    value.is_static	 = false;
	value.identifier = identifier;
}

IdentifierNode::IdentifierNode( size_t lineno, access_t access, bool is_static, char *identifier, Node *v ) : Node(H_NT_IDENTIFIER,lineno) {
    value.access     = access;
    value.is_static	 = is_static;
	value.identifier = identifier;

	addChild(v);
}

Node *IdentifierNode::clone(){
	Node *clone = new IdentifierNode( lineno, id() ),
		 *node;

	clone->value.access    = value.access;
    clone->value.is_static = value.is_static;

    ll_foreach( &children, nitem ){
    	node = ll_node(nitem);
   		clone->addChild( node ? node->clone() : node );
   	}

    return clone;
}

/* attribute request expression */
AttributeRequestNode::AttributeRequestNode( size_t lineno, Node *owner, Node *member ) : Node(H_NT_ATTRIBUTE,lineno) {
	value.owner  = owner;
	value.member = member;
}

Node *AttributeRequestNode::clone(){
	return new AttributeRequestNode( lineno, value.owner, value.member );
}

/* class method call */
MethodCallNode::MethodCallNode( size_t lineno, Node *owner, Node *call ) : Node(H_NT_METHOD_CALL,lineno) {
	value.owner  = owner;
	value.member = call;
}

Node *MethodCallNode::clone(){
	return new MethodCallNode( lineno, value.owner, value.member );
}

/* functions */
FunctionNode::FunctionNode( size_t lineno, function_decl_t *declaration ) : Node(H_NT_FUNCTION,lineno) {
    value.function = declaration->function;
    value.vargs    = declaration->vargs;
    value.argc	   = declaration->argc;

	/* add function prototype args children */
	for( size_t i = 0; i < value.argc; ++i ){
		addChild( new IdentifierNode( lineno, declaration->argv[i] ) );
	}
}

FunctionNode::FunctionNode( size_t lineno, function_decl_t *declaration, int argc, ... ) : Node(H_NT_FUNCTION,lineno) {
    va_list ap;
	size_t  i;

	value.function = declaration->function;
    value.vargs    = declaration->vargs;
    value.argc	   = declaration->argc;

	/* add function prototype args children */
	for( i = 0; i < value.argc; ++i ){
		addChild( new IdentifierNode( lineno, declaration->argv[i] ) );
	}
	/* add function body statements node */
	va_start( ap, argc );
	addChild( body = va_arg( ap, Node * ) );
	for( i = 1; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

FunctionNode::FunctionNode( size_t lineno, const char *name ) : Node(H_NT_FUNCTION,lineno) {
    value.function = name;
}

Node *FunctionNode::clone(){
	Node *clone = new FunctionNode( lineno, value.function.c_str() ),
		 *node,
		 *nclone;

	clone->value.argc  = value.argc;
	clone->value.vargs = value.vargs;

	ll_foreach( &children, nitem ){
		node   = ll_node( nitem );
		nclone = (node ? node->clone() : node);
		if( node == body ){
			clone->body = nclone;
		}
		clone->addChild( nclone );
	}

	return clone;
}

/* function calls */
CallNode::CallNode( size_t lineno, char *name, llist_t *argv ) : Node(H_NT_CALL,lineno) {
    value.call = name;
    if( argv != NULL ){
    	ll_merge_destroy( &children, argv );
	}
}

CallNode::CallNode( size_t lineno, Node *alias, llist_t *argv ) :  Node(H_NT_CALL,lineno) {
    value.alias = alias;
    if( argv != NULL ){
    	ll_merge_destroy( &children, argv );
	}
}

Node *CallNode::clone(){
	Node *clone = H_UNDEFINED,
		 *node;

    if( value.alias == NULL ){
        clone = new CallNode( lineno, (char *)value.call.c_str(), NULL );
    }
    else{
        clone = new CallNode( lineno, value.alias, NULL );
    }
	ll_foreach( &children, nitem ){
		node = ll_node(nitem);
		clone->addChild( node ? node->clone() : node );
	}

    return clone;
}

/* try/catch/finally statements */
TryCatchNode::TryCatchNode( size_t lineno, int statement, Node *try_block, char *exception_id, Node *catch_block, Node *finally_block ) : Node(H_NT_STATEMENT,lineno) {
	opcode 	    		= statement;
	value.try_block     = try_block;
	value.exception_id  = exception_id;
	value.catch_block   = catch_block;
	value.finally_block = finally_block;
}

Node *TryCatchNode::clone(){
	return new TryCatchNode( lineno,
							 opcode,
							 value.try_block,
							 (char *)value.exception_id.c_str(),
							 value.catch_block,
							 value.finally_block );
}

/* structure or class creation */
NewNode::NewNode( size_t lineno, char *type, llist_t *argv ) : Node(H_NT_NEW,lineno){
	value.identifier = type;
	if( argv != NULL ){
		ll_merge_destroy( &children, argv );
	}
}

Node *NewNode::clone(){
	Node *clone = new NewNode( lineno, id(), NULL ),
		 *node;

	ll_foreach( &children, nitem ){
		node = ll_node(nitem);
		clone->addChild( node ? node->clone() : node );
	}

	return clone;
}

/* struct type definition */
StructureNode::StructureNode( size_t lineno, char *s_name, llist_t *attributes ) : Node(H_NT_STRUCT,lineno) {
    value.identifier = s_name;
    if( attributes != NULL ){
    	ll_merge_destroy( &children, attributes );
	}
}

/* methods */
MethodDeclarationNode::MethodDeclarationNode( size_t lineno, access_t access, method_decl_t *declaration, int argc, ... ) : Node(H_NT_METHOD_DECL,lineno) {
    value.method = declaration->method;
    value.vargs  = declaration->vargs;
    value.argc   = declaration->argc;
    value.access = access;

    va_list ap;
	int i;

	/* add method prototype args children */
	for( i = 0; i < value.argc; ++i ){
		addChild( new IdentifierNode( lineno, declaration->argv[i] ) );
	}
	/* add method body statements node */
	va_start( ap, argc );
	addChild( body = va_arg( ap, Node * ) );
	for( i = 1; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

MethodDeclarationNode::MethodDeclarationNode( size_t lineno, access_t access, method_decl_t *declaration, bool is_static, int argc, ... ) : Node(H_NT_METHOD_DECL,lineno) {
    value.method 	= declaration->method;
    value.vargs  	= declaration->vargs;
    value.argc   	= declaration->argc;
    value.access 	= access;
    value.is_static = is_static;

    va_list ap;
	int i;

	/* add method prototype args children */
	for( i = 0; i < value.argc; ++i ){
		addChild( new IdentifierNode( lineno, declaration->argv[i] ) );
	}
	/* add method body statements node */
	va_start( ap, argc );
	addChild( body = va_arg( ap, Node * ) );
	for( i = 1; i < argc; ++i ){
		addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);
}

MethodDeclarationNode::MethodDeclarationNode( size_t lineno, const char *name, access_t access ) : Node(H_NT_METHOD_DECL,lineno) {
	value.method = name;
	value.access = access;
}

Node *MethodDeclarationNode::clone(){
	Node *clone = new MethodDeclarationNode( lineno, value.method.c_str(), value.access ),
		 *node,
		 *nclone;

	clone->value.is_static = value.is_static;
	clone->value.vargs     = value.vargs;
	clone->value.argc	   = value.argc;

	ll_foreach( &children, nitem ){
		node   = ll_node( nitem );
		nclone = node ? node->clone() : node;
		if( node == body ){
			clone->body = nclone;
		}
		clone->addChild( nclone );
	}

	return clone;
}

/* class type definition */
ClassNode::ClassNode( size_t lineno, char *classname, llist_t *extends, llist_t *members ) : Node(H_NT_CLASS,lineno) {
	value.identifier = classname;
	if( extends != NULL ){
		ll_init( &value.extends );
		ll_merge_destroy( &value.extends, extends );
	}
	if( members != NULL ){
		ll_merge_destroy( &children, members );
	}
}

