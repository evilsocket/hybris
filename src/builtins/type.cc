#include "builtin.h"

HYBRIS_BUILTIN(hisint){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isint' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)(data->at(0)->xtype == H_OT_INT) );
}

HYBRIS_BUILTIN(hisfloat){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isfloat' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)(data->at(0)->xtype == H_OT_FLOAT) );
}

HYBRIS_BUILTIN(hischar){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'ischar' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)(data->at(0)->xtype == H_OT_CHAR) );
}

HYBRIS_BUILTIN(hisstring){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isstring' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)(data->at(0)->xtype == H_OT_STRING) );
}

HYBRIS_BUILTIN(hisarray){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isarray' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)(data->at(0)->xtype == H_OT_ARRAY) );
}

HYBRIS_BUILTIN(hismap){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'ismap' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)(data->at(0)->xtype == H_OT_MAP) );
}

HYBRIS_BUILTIN(hisalias){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'isalias' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)(data->at(0)->xtype == H_OT_ALIAS) );
}

HYBRIS_BUILTIN(htypeof){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'typeof' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (char *)Object::type(data->at(0)) );
}

HYBRIS_BUILTIN(hsizeof){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'sizeof' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)data->at(0)->xsize );
}

HYBRIS_BUILTIN(htoint){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'toint' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( (int)data->at(0)->lvalue() );
}

HYBRIS_BUILTIN(htostring){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'tostring' requires 1 parameter (called with %d)", data->size() );
	}
	return new Object( data->at(0)->toString() );
}

HYBRIS_BUILTIN(htoxml){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'toxml' requires 1 parameter (called with %d)", data->size() );
	}

	return new Object( (char *)data->at(0)->toxml().c_str() );
}

HYBRIS_BUILTIN(hfromxml){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'fromxml' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );

	return Object::fromxml( (char *)data->at(0)->xstring.c_str() );
}
