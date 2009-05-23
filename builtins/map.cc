#include "../common.h"
#include "../vmem.h"
#include "../builtin.h"

HYBRIS_BUILTIN(hmap){
	if( (data->size() % 2) != 0 ){
		hybris_syntax_error( "function 'map' requires an even number of parameters (called with %d)", data->size() );
	}
	unsigned int i;
	Object *map = new Object();
	for( i = 0; i < data->size(); i += 2 ){
		map->map( data->at(i), data->at(i + 1) );
	}
	return map;
}

HYBRIS_BUILTIN(hmapelements){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'mapelements' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_MAP );

	return new Object( (int)data->at(0)->xmap.size() );
}

HYBRIS_BUILTIN(hmappop){
	if( data->size() != 1 ){
		hybris_syntax_error( "function 'mappop' requires 1 parameter (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_MAP );

	return new Object( data->at(0)->mapPop() );
}

HYBRIS_BUILTIN(hunmap){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'unmap' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_MAP );

	return new Object( data->at(0)->unmap(data->at(1)) );
}

HYBRIS_BUILTIN(hismapped){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'ismapped' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_MAP );
	Object *map   = data->at(0),
		    *find  = data->at(1);
	unsigned int i;

	for( i = 0; i < map->xarray.size(); i++ ){
		if( map->xarray[i]->equals(find) ){
			return new Object( (int)i );
		}
	}

	return new Object( (int)-1 );
}

