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

HYBRIS_DEFINE_FUNCTION(hgc_collect);
HYBRIS_DEFINE_FUNCTION(hgc_mm_items);
HYBRIS_DEFINE_FUNCTION(hgc_mm_usage);
HYBRIS_DEFINE_FUNCTION(hgc_collect_threshold);

HYBRIS_EXPORTED_FUNCTIONS() {
	{ "gc_collect",	 hgc_collect },
    { "gc_mm_items", hgc_mm_items },
    { "gc_mm_usage", hgc_mm_usage },
    { "gc_collect_threshold", hgc_collect_threshold },
    { "", NULL }
};

HYBRIS_DEFINE_FUNCTION(hgc_collect){
	gc_collect( vm );
	return ob_dcast( H_DEFAULT_RETURN );
}

HYBRIS_DEFINE_FUNCTION(hgc_mm_items){
	return ob_dcast( gc_new_integer(gc_mm_items()) );
}

HYBRIS_DEFINE_FUNCTION(hgc_mm_usage){
	return ob_dcast( gc_new_integer(gc_mm_usage()) );
}

HYBRIS_DEFINE_FUNCTION(hgc_collect_threshold){
	return ob_dcast( gc_new_integer(gc_collect_threshold()) );
}
