#include "../common.h"
#include "../vmem.h"
#include "../builtin.h"

HYBRIS_BUILTIN(harray){
	unsigned int i;
	Object *array = new Object();
	for( i = 0; i < data->size(); i++ ){
		array->push( data->at(i) );
	}
	return array;
}

HYBRIS_BUILTIN(helements){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'elements' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );

	return new Object( (int)data->at(0)->xarray.size() );
}

HYBRIS_BUILTIN(hpop){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'pop' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );

	return new Object( data->at(0)->pop() );
}

HYBRIS_BUILTIN(hremove){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'remove' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );

	return new Object( data->at(0)->remove(data->at(1)) );
}

HYBRIS_BUILTIN(hcontains){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'contains' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_ARRAY );
	Object *array = data->at(0),
		    *find  = data->at(1);
	unsigned int i;

	for( i = 0; i < array->xarray.size(); i++ ){
		Object *boolean = ((*find) == array->xarray[i]);
		if( boolean->lvalue() ){
			delete boolean;
			return new Object( (int)i );
		}
		delete boolean;
	}

	return new Object( (int)-1 );
}

