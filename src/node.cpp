/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
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
			fread( &_operator, 1, sizeof(long), fp );
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

