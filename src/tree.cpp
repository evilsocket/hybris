#include "tree.h"

HNodeList *Tree::createList(){
	return new HNodeList();
}

Node *Tree::addInt( int value ){
	Node *node     = new Node(H_NT_CONSTANT);
	node->_constant = new Object(value);
	return node;
}

Node *Tree::addFloat( double value ){
	Node *node     = new Node(H_NT_CONSTANT);
	node->_constant = new Object(value);
	return node;
}

Node *Tree::addChar( char value ){
	Node *node     = new Node(H_NT_CONSTANT);
	node->_constant = new Object(value);
	return node;
}

Node *Tree::addString( char *value ){
	Node *node     = new Node(H_NT_CONSTANT);
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
	Node *node = new Node(H_NT_IDENTIFIER);
	strncpy( node->_identifier, id, 0xFF );
	return node;
}

Node *Tree::addFunction( function_decl_t *declaration, int argc, ... ){
	Node *node = new Node(H_NT_FUNCTION);
	va_list ap;
	int i;

	strncpy( node->_function, declaration->function, 0xFF );

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

	strncpy( node->_function, name, 0xFF );

	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; i++ ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addCall( char *name, HNodeList *argv ){
	Node *node = new Node(H_NT_CALL);
	strncpy( node->_call, name, 0xFF );

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
			fwrite( &n, 1, sizeof(int), fp );
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
	fwrite( &children, 1, sizeof(int), fp );
	for( i = 0; i < children; i++ ){
		Tree::compile( node->child(i), fp );
	}
}

Node * Tree::load( FILE *fp ){
	return new Node(fp);
}

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
				clone = Tree::addIdentifier( root->_identifier );
				break;

			case H_NT_OPERATOR   :
				clone = Tree::addOperator( root->_operator, 0 );
				for( i = 0; i < root->children(); i++ ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
				break;

			case H_NT_FUNCTION   :
				clone = Tree::addFunction( root->_function, 0 );
				for( i = 0; i < root->children(); i++ ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
				break;

			case H_NT_CALL       :
				if( root->_aliascall == NULL ){
					clone = Tree::addCall( root->_call, NULL );
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
		/* ignore statically allocated identifiers nodes */

		delete node;
	}
}

