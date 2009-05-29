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

