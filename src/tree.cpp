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
    return new ConstantNode(value);
}

Node *Tree::addFloat( double value ){
    return new ConstantNode(value);
}

Node *Tree::addChar( char value ){
    return new ConstantNode(value);
}

Node *Tree::addString( char *value ){
    return new ConstantNode(value);
}

Node *Tree::addExpression( int expression, int argc, ... ){
    ExpressionNode *node = new ExpressionNode(expression);
    va_list ap;
	int i;

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addStatement( int statement, int argc, ... ){
    StatementNode *node = new StatementNode(statement);
    va_list ap;
	int i;

	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addIdentifier( char *id ){
	return new IdentifierNode(id);
}

Node *Tree::addFunction( function_decl_t *declaration, int argc, ... ){
	FunctionNode *node = new FunctionNode(declaration);
	va_list ap;
	int i;

	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addFunction( char *name, int argc, ... ){
	FunctionNode *node = new FunctionNode(name);
	va_list ap;
	int i;

	/* add function body statements node */
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		node->addChild( va_arg( ap, Node * ) );
	}
	va_end(ap);

	return node;
}

Node *Tree::addCall( char *name, HNodeList *argv ){
	CallNode *node  = new CallNode(name);

	if( argv != NULL ){
		for( HNodeList::iterator ni = argv->begin(); ni != argv->end(); ni++ ){
			node->addChild( *ni );
		}

		delete argv;
	}

	return node;
}

Node *Tree::addCall( Node *alias, HNodeList *argv ){
	CallNode *node = new CallNode(alias);

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
		case H_NT_EXPRESSION : return "EXPRESSION"; break;
		case H_NT_STATEMENT  : return "STATEMENT";  break;
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
		for( i = 0; i < node->children(); ++i ){
			Tree::print( node->child(i), t + 1 );
		}
	}
}

Node *Tree::clone( Node *root, Node *clone ){
	if( root ){
		int i;
		H_NODE_TYPE rtype(root->type());
		Node        node(rtype);
        int         children;

		switch( rtype ){
			case H_NT_CONSTANT   :
				switch( root->value.m_constant->xtype ){
					case H_OT_INT    : clone = Tree::addInt( root->value.m_constant->xint ); break;
					case H_OT_FLOAT  : clone = Tree::addFloat( root->value.m_constant->xfloat ); break;
					case H_OT_CHAR   : clone = Tree::addChar( root->value.m_constant->xchar ); break;
					case H_OT_STRING : clone = Tree::addString( (char *)root->value.m_constant->xstring.c_str() ); break;
				}
            break;

			case H_NT_IDENTIFIER :
				clone = Tree::addIdentifier( (char *)root->value.m_identifier.c_str() );
            break;

			case H_NT_EXPRESSION   :
				clone    = Tree::addExpression( root->value.m_expression, 0 );
				children = root->children();
				for( i = 0; i < children; ++i ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
            break;

            case H_NT_STATEMENT  :
				clone    = Tree::addStatement( root->value.m_statement, 0 );
				children = root->children();
				for( i = 0; i < children; ++i ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
            break;

			case H_NT_FUNCTION   :
				clone    = Tree::addFunction( (char *)root->value.m_function.c_str(), 0 );
				children = root->children();
				for( i = 0; i < children; ++i ){
					clone->addChild( Tree::clone( root->child(i), &node ) );
				}
            break;

			case H_NT_CALL       :
				if( root->value.m_alias_call == NULL ){
					clone = Tree::addCall( (char *)root->value.m_call.c_str(), NULL );
				}
				else{
					clone = Tree::addCall( root->value.m_alias_call, NULL );
				}
				children = root->children();
				for( i = 0; i < children; ++i ){
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
			for( i = 0; i < node->children(); ++i ){
				Tree::release( node->child(i) );
			}
		}
		delete node;
	}
}

