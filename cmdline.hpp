#ifndef _HCMDLINE_H_
#	define _HCMDLINE_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

class CmdLine {
private :
    vector<string> m_argv;
public  :
    CmdLine( int argc, char *argv[] ){
        unsigned int i;
        for( i = 0; i < argc; i++ ){
            m_argv.push_back(argv[i]);
        }
    }

    int isset( const char *flag ){
        unsigned int i;
        for( i = 0; i < m_argv.size(); i++ ){
            if( m_argv[i] == flag ){
                return 1;
            }
        }
        return 0;
    }

    void nonFlaggedArg( char *buffer ){
        if( m_argv.size() > 1 && m_argv[1][0] != '-' ){
            strncpy( buffer, m_argv[1].c_str(), 0xFF );
        }
    }

    string get( const char *flag ){
        unsigned int i;
        for( i = 0; i < m_argv.size(); i++ ){
            if( m_argv[i] == flag && (i + 1) < m_argv.size() ){
                return m_argv[i + 1];
            }
        }
        return "";
    }

    string get( const char *flag, char *buffer ){
        unsigned int i;
        for( i = 0; i < m_argv.size(); i++ ){
            if( m_argv[i] == flag && (i + 1) < m_argv.size() ){
                strncpy( buffer, m_argv[i + 1].c_str(), 0xFF );
                return m_argv[i + 1];
            }
        }
        return "";
    }
};

#endif

