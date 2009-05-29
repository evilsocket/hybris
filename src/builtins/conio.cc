#include "common.h"
#include "vmem.h"
#include "builtin.h"

HYBRIS_BUILTIN(hprint){
    unsigned int i;
    for( i = 0; i < data->size(); i++ ){
        data->at(i)->print();
    }
    return NULL;
}

HYBRIS_BUILTIN(hprintln){
    if( data->size() ){
        unsigned int i;
        for( i = 0; i < data->size(); i++ ){
            data->at(i)->println();
        }
    }
    else{
        printf("\n");
    }
    return NULL;
}

HYBRIS_BUILTIN(hinput){
    Object *_return;
    if( data->size() == 2 ){
        data->at(0)->print();
        data->at(1)->input();
        _return = data->at(1);
    }
    else if( data->size() == 1 ){
        data->at(0)->input();
        _return = data->at(0);
    }
	else{
		hybris_syntax_error( "function 'input' requires 1 or 2 parameters (called with %d)", data->size() );
	}

    return _return;
}
