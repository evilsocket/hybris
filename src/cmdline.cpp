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
#include "cmdline.h"

CmdLine::CmdLine( int argc, char *argv[] ){
	unsigned int i;
	for( i = 0; i < argc; i++ ){
		m_argv.push_back(argv[i]);
	}
}

int CmdLine::isset( const char *flag ){
	unsigned int i;
	for( i = 0; i < m_argv.size(); i++ ){
		if( m_argv[i] == flag ){
			return 1;
		}
	}
	return 0;
}

void CmdLine::nonFlaggedArg( char *buffer ){
	if( m_argv.size() > 1 && m_argv[1][0] != '-' ){
		strncpy( buffer, m_argv[1].c_str(), 0xFF );
	}
}

string CmdLine::get( const char *flag ){
	unsigned int i;
	for( i = 0; i < m_argv.size(); i++ ){
		if( m_argv[i] == flag && (i + 1) < m_argv.size() ){
			return m_argv[i + 1];
		}
	}
	return "";
}

string CmdLine::get( const char *flag, char *buffer ){
	unsigned int i;
	for( i = 0; i < m_argv.size(); i++ ){
		if( m_argv[i] == flag && (i + 1) < m_argv.size() ){
			strncpy( buffer, m_argv[i + 1].c_str(), 0xFF );
			return m_argv[i + 1];
		}
	}
	return "";
}

