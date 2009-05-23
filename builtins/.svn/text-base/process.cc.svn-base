#include "../common.h"
#include "../vmem.h"
#include "../builtin.h"
#include <sys/types.h>
#include <sys/wait.h>


HYBRIS_BUILTIN(hexec){
	htype_assert( data->at(0), H_OT_STRING );
    Object *_return = NULL;
    if( data->size() ){
        _return = new Object( system( data->at(0)->xstring.c_str() ) );
    }
	else{
		hybris_syntax_error( "function 'exec' requires 1 parameter (called with %d)", data->size() );
	}
    return _return;
}

HYBRIS_BUILTIN(hfork){
    return new Object( (int)fork() );
}

HYBRIS_BUILTIN(hgetpid){
    return new Object( (int)getpid() );
}

HYBRIS_BUILTIN(hwait){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'wait' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_INT );
    return new Object( (int)wait( &data->at(0)->xint ) );
}

HYBRIS_BUILTIN(hpopen){
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );

    if( data->size() == 2 ){
        return new Object( (int)popen( data->at(0)->xstring.c_str(), data->at(1)->xstring.c_str() ) );
    }
	else{
		hybris_syntax_error( "function 'popen' requires 2 parameters (called with %d)", data->size() );
	}
}

HYBRIS_BUILTIN(hpclose){
	htype_assert( data->at(0), H_OT_INT );
    if( data->size() ){
		pclose( (FILE *)data->at(0)->xint );
    }
    return new Object(0);
}

HYBRIS_BUILTIN(hexit){
    int code = 0;
    if( data->size() > 0 ){
		htype_assert( data->at(0), H_OT_INT );
		code = data->at(0)->xint;
	}
	exit(code);

    return new Object(0);
}
