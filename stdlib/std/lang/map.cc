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
#include <hybris.h>

HYBRIS_DEFINE_FUNCTION(hmap);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "map", hmap },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hmap){
	if( (ob_argc() % 2) != 0 ){
		hyb_error( H_ET_SYNTAX, "function 'map' requires an even number of parameters (called with %d)", ob_argc() );
	}
	unsigned int i;
	Object *map = ob_dcast( gc_new_map() );
	for( i = 0; i < data->size(); i += 2 ){
		ob_cl_set( map, ob_argv(i), ob_argv(i + 1) );
	}
	return map;
}
