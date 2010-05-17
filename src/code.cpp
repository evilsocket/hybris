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
#include "code.h"

CodeSegment::CodeSegment() : HashMap<Node>() {

}

CodeSegment::~CodeSegment(){
    release();
}

Node *CodeSegment::add( char *identifier, Node *node ){
    char *function_name = (char *)node->value.m_function.c_str();

    /*
     * Functions can be defined only once!
     */
    if( get( function_name ) == H_UNDEFINED ){
        return insert( function_name, node->clone() );
    }
    else{
        hyb_error( H_ET_SYNTAX, "Function %s already defined", identifier );
    }

    return node;
}

void CodeSegment::release(){
    unsigned int i;

    for( i = 0; i < m_elements; ++i ){
        Node *item = at(i);
        delete item;
    }
    clear();
}
