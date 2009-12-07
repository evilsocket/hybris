/*
 * This file is part of hybris.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hybris.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _HCONFIG_H_
#   define _HCONFIG_H_

#define AUTHOR  "The Hybris Dev Team [http://www.hybris-lang.org/]"
#define VERSION "0.6b"

// system path for libraries (.so dynamic libs)
#define LIBS_PATH "/usr/lib/hybris/libs/"
// system path for modules (hybris global scripts)
#define MODS_PATH "/usr/lib/hybris/modules/"

// memory allocation debug messages
// #define MEM_DEBUG 1

// garbage collector, *** VERY EXPERIMENTAL FEATURE ***
// #define GC_SUPPORT 1

// security check on bounds for array/map/matrix access
#define BOUNDS_CHECK 1

// support for regular expressions
#define PCRE_SUPPORT 1
// get/post http(s) methos support
#define HTTP_SUPPORT 1
// xml parsing support
#define XML_SUPPORT  1


#endif

