#include "node.h"

Node::Node( H_NODE_TYPE type ){
	m_type     = type;
	_constant  = NULL;
	_aliascall = NULL;
	memset( _identifier, 0x00, 0xFF );
	memset( _function, 0x00, 0xFF );
	memset( _call, 0x00, 0xFF );
}

Node::Node( FILE *fp ){
	_constant = NULL;
	fread( &m_type, sizeof(H_NODE_TYPE), 1, fp );
	int  i = 0, n;
	switch(m_type){
		case H_NT_CONSTANT   :
			_constant = new Object(fp);
		break;
		case H_NT_IDENTIFIER :
			memset( _identifier, 0x00, 0xFF );
			fread( &n, 1, sizeof(unsigned int), fp );
			fread( _identifier, n, sizeof(char), fp );
		break;
		case H_NT_OPERATOR   :
			fread( &_operator, 1, sizeof(int), fp );
		break;
		case H_NT_FUNCTION   :
			memset( _function, 0x00, 0xFF );
			fread( &n, 1, sizeof(unsigned int), fp );
			fread( _function, n, sizeof(char), fp );
		break;
		case H_NT_CALL       :
			memset( _call, 0x00, 0xFF );
			fread( &n, 1, sizeof(unsigned int), fp );
			fread( _call, n, sizeof(char), fp );
		break;

		default : return;
	}

	unsigned int children;
	fread( &children, 1, sizeof(unsigned int), fp );
	if( children > 0 ){
		for( i = 0; i < children; i++ ){
			addChild( new Node(fp) );
		}
	}
}

Node::~Node(){
	if( _constant ){
		delete _constant;
	}
}

H_NODE_TYPE Node::type(){
	return m_type;
}

unsigned int Node::children(){
	return m_children.size();
}

Node *Node::child( unsigned int i ){
	return m_children[i];
}

void Node::addChild( Node *child ){
	m_children.push_back(child);
}

