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

HYBRIS_DEFINE_FUNCTION(harray);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "array", harray, H_ANY_ARGC, { H_ANY_TYPE } },
	{ "", NULL }
};

HYBRIS_DEFINE_FUNCTION(harray){
	unsigned int i;
	Object *array = ob_dcast( gc_new_vector() );
	for( i = 0; i < data->size(); ++i ){
		ob_cl_push( array, ob_argv(i) );
	}
	return array;
}

