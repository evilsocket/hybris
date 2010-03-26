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

Node::Node( H_NODE_TYPE type ) :
    m_type(type),
    m_elements(0),

    _constant(NULL),
    _aliascall(NULL),
    _identifier(""),
    _function(""),
    _call("")
{

}

Node::~Node(){
	if( _constant ){
        delete _constant;
	}
}

void Node::addChild( Node *child ){
	m_children.push_back(child);
	m_elements++;
}

