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
#include "tree.h"

HNodeList *Tree::createList(){
	return new HNodeList();
}

Node *Tree::addInt( long value ){
	Node *node      = new Node(H_NT_CONSTANT);
	node->_constant = new Object(value);
	return node;
}

Node *Tree::addFloat( double value ){
	Node *node      = new Node(H_NT_CONSTANT);
	node->_constant = new Object(value);
	return node;
}

Node *Tree::addChar( char value ){
	Node *node      = new Node(H_NT_CONSTANT);
	node->_constant = new Object(value);
	return node;
}

Node *Tree::addString( char *value ){
	Node *node      = new Node(H_NT_CONSTANT);
	node->_constant = new Object(value);
	return node;
}

Node *Tree::addOperator( int op, int argc, ... ){
	Node *node = new Node(H_NT_OPERATOR);
	va_list ap;
	int i;

	node->_operator = op;

	va_start( ap, argc );
	for( i = 0; i < argc; i++ ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addIdentifier( char *id ){
	Node *node        = new Node(H_NT_IDENTIFIER);
	node->_identifier = id;
	return node;
}

Node *Tree::addFunction( function_decl_t *declaration, int argc, ... ){
	Node *node = new Node(H_NT_FUNCTION);
	va_list ap;
	int i;

	node->_function = declaration->function;

	/* add function prototype args children */
	for( i = 0; i < declaration->argc; i++ ){
		node->addChild( Tree::addIdentifier( declaration->argv[i] ) );
	}
	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; i++ ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addFunction( char *name, int argc, ... ){
	Node *node = new Node(H_NT_FUNCTION);
	va_list ap;
	int i;

	node->_function = name;

	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; i++ ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addCall( char *name, HNodeList *argv ){
	Node *node  = new Node(H_NT_CALL);
	node->_call = name;

	if( argv != NULL ){
		for( HNodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			node->addChild( *ni );
		}

		delete argv;
	}

	return node;
}

Node *Tree::addCall( Node *alias, HNodeList *argv ){
	Node *node = new Node(H_NT_CALL);
	node->_aliascall = alias;

	if( argv != NULL ){
		for( HNodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			node->addChild( *ni );
		}

		delete argv;
	}

	return node;
}

const char *Tree::type( Node *node ){
	switch(node->type()){
		case H_NT_CONSTANT   : return "CONSTANT";   break;
		case H_NT_IDENTIFIER : return "IDENTIFIER"; break;
		case H_NT_OPERATOR   : return "OPERATOR";   break;
		case H_NT_FUNCTION   : return "FUNCTION";   break;
		case H_NT_CALL       : return "CALL";       break;
	}
}

void Tree::print( Node *node, int tabs /*= 0*/ ){
	int t;
	for( t = 0; t < tabs; t++ ){
		printf("\t");
	}
	printf( "%s\n", Tree::type(node) );
	if( node->children() > 0 ){
		int i;
		for( i = 0; i < node->children(); i++ ){
			Tree::print( node->child(i), t + 1 );
		}
	}
}
/*
void Tree::compile( Node *node, FILE *fp ){
	H_NODE_TYPE type = node->type();
	fwrite( &type, 1, sizeof(H_NODE_TYPE), fp );
	unsigned int n;

	switch(type){
		case H_NT_CONSTANT   :
			node->_constant->compile(fp);
		break;
		case H_NT_IDENTIFIER :
			n = strlen(node->_identifier) + 1;
			fwrite( &n , 1, sizeof(unsigned int), fp );
			fwrite( node->_identifier, 1, n, fp );
		break;
		case H_NT_OPERATOR   :
			n = node->_operator;
			fwrite( &n, 1, sizeof(long), fp );
		break;
		case H_NT_FUNCTION   :
			n = strlen(node->_function) + 1;
			fwrite( &n , 1, sizeof(unsigned int), fp );
			fwrite( node->_function, 1, n, fp );
		break;
		case H_NT_CALL       :
			n = strlen(node->_call) + 1;
			fwrite( &n , 1, sizeof(unsigned int), fp );
			fwrite( node->_call, 1, n, fp );
		break;
	}

	unsigned int children = node->children(), i;
	fwrite( &children, 1, sizeof(long), fp );
	for( i = 0; i < children; i++ ){
		Tree::compile( node->child(i), fp );
	}
}

Node * Tree::load( FILE *fp ){
	return new Node(fp);
}
*/
Node *Tree::clone( Node *root, Node *clone ){
	if( root ){
		int i;
		Node node(root->type());
		switch( root->type() ){
			case H_NT_CONSTANT   :
				switch( root->_constant->xtype ){
					case H_OT_INT    : clone = Tree::addInt( root->_constant->xint ); break;
					case H_OT_FLOAT  : clone = Tree::addFloat( root->_constant->xfloat ); break;
					case H_OT_CHAR   : clone = Tree::addChar( root->_constant->xchar ); break;
					case H_OT_STRING : clone = Tree::addString( (char *)root->_constant->xstring.c_str() ); break;
				}
				break;

			case H_NT_IDENTIFIER :
				clone = Tree::addIdentifier( (char *)root->_identifier.c_str() );
				break;

			case H_NT_OPERATOR   :
				clone = Tree::addOperator( root->_operator, 0 );
				for( i = 0; i < root->children(); i++ ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
				break;

			case H_NT_FUNCTION   :
				clone = Tree::addFunction( (char *)root->_function.c_str(), 0 );
				for( i = 0; i < root->children(); i++ ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
				break;

			case H_NT_CALL       :
				if( root->_aliascall == NULL ){
					clone = Tree::addCall( (char *)root->_call.c_str(), NULL );
				}
				else{
					clone = Tree::addCall( root->_aliascall, NULL );
				}
				for( i = 0; i < root->children(); i++ ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
				break;
		}
	}
	return clone;
}

void Tree::release( Node *node ){
	if(node){
		int i;
		if( node->children() > 0 ){
			for( i = 0; i < node->children(); i++ ){
				Tree::release( node->child(i) );
			}
		}
	}
}

