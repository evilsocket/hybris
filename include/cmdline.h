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
#ifndef _HCMDLINE_H_
#	define _HCMDLINE_H_

#include <string>
#include <vector>
#include <string.h>

using std::string;
using std::vector;

class CmdLine {
private :
    vector<string> m_argv;
public  :

    CmdLine( int argc, char *argv[] );

    int isset( const char *flag );
    void nonFlaggedArg( char *buffer );
    string get( const char *flag );
    string get( const char *flag, char *buffer );
};

#endif

