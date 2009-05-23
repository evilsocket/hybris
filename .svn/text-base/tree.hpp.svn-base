#ifndef _HTREE_H_
#	define _HTREE_H_

#include "common.h"
#include <vector>
#include <list>

using std::vector;
using std::list;

typedef unsigned short H_NODE_TYPE;

#define H_NT_NONE       0
#define H_NT_CONSTANT   1
#define H_NT_IDENTIFIER 2
#define H_NT_OPERATOR   3
#define H_NT_FUNCTION   4
#define H_NT_CALL       5

class Node {

private :

    H_NODE_TYPE     m_type;
    vector<Node *> m_children;

public  :

    Node( H_NODE_TYPE type ){
        m_type     = type;
        _constant  = NULL;
		_aliascall = NULL;
        memset( _identifier, 0x00, 0xFF );
        memset( _function, 0x00, 0xFF );
        memset( _call, 0x00, 0xFF );
    }

    Node( FILE *fp ){
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

    ~Node(){
        if( _constant ){
            delete _constant;
        }
    }

    inline H_NODE_TYPE type(){
        return m_type;
    }

    Object       *_constant;
    char          _identifier[0xFF];
    int           _operator;
    char          _function[0xFF];
    char          _call[0xFF];
	Node         *_aliascall;

    inline unsigned int children(){
        return m_children.size();
    }

    inline Node *child( unsigned int i ){
        return m_children[i];
    }

    inline void addChild( Node *child ){
        m_children.push_back(child);
    }
};

typedef list<Node *> HNodeList;

class Tree {
public :

    inline static HNodeList *createList(){
        return new HNodeList();
    }

    inline static Node *addInt( int value ){
        Node *node     = new Node(H_NT_CONSTANT);
        node->_constant = new Object(value);
        return node;
    }

    inline static Node *addFloat( double value ){
        Node *node     = new Node(H_NT_CONSTANT);
        node->_constant = new Object(value);
        return node;
    }

    inline static Node *addChar( char value ){
        Node *node     = new Node(H_NT_CONSTANT);
        node->_constant = new Object(value);
        return node;
    }

    inline static Node *addString( char *value ){
        Node *node     = new Node(H_NT_CONSTANT);
        node->_constant = new Object(value);
        return node;
    }

    inline static Node *addOperator( int op, int argc, ... ){
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

    inline static Node *addIdentifier( char *id ){
        Node *node = new Node(H_NT_IDENTIFIER);
        strncpy( node->_identifier, id, 0xFF );
        return node;
    }

    inline static Node *addFunction( function_decl_t *declaration, int argc, ... ){
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

    inline static Node *addFunction( char *name, int argc, ... ){
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

    inline static Node *addCall( char *name, HNodeList *argv ){
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

	inline static Node *addCall( Node *alias, HNodeList *argv ){
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

    inline static const char *type( Node *node ){
        switch(node->type()){
            case H_NT_CONSTANT   : return "CONSTANT";   break;
            case H_NT_IDENTIFIER : return "IDENTIFIER"; break;
            case H_NT_OPERATOR   : return "OPERATOR";   break;
            case H_NT_FUNCTION   : return "FUNCTION";   break;
            case H_NT_CALL       : return "CALL";       break;
        }
    }

    inline static void print( Node *node, int tabs = 0 ){
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

    inline static void compile( Node *node, FILE *fp ){
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

    inline static Node * load( FILE *fp ){
        return new Node(fp);
    }

    inline static Node *clone( Node *root, Node *clone ){
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

    inline static void release( Node *node ){
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

};

#endif

