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
#ifndef _HVCODE_H_
#	define _HVCODE_H_

#include "node.h"
#include "itree.h"
#include "memory.h"

/*
 * This class represent user defined functions.
 */
class CodeSegment : public ITree<Node> {
    public :

        CodeSegment();
        ~CodeSegment();

        __force_inline Node *get( char *identifier ){
        	return find(identifier);
        }

        Node *add( char *identifier, Node *node );

        void release();
};

/* post type definitions */
typedef CodeSegment vcode_t;

#endif
