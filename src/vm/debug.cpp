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
#include "vm.h"
#include "debug.h"

void dbg_init( dbg_t *dbg, vm_t *vm ){
	ll_init( &dbg->bpoints );
	dbg->vm = vm;
}

void dbg_add_bpoint( dbg_t *dbg, char *filename, size_t lineno ){
	FILE *fp	  = fopen( filename, "rt" );
	bpoint_t *bp  = new bpoint_t;
	char	 *sep = strrchr( filename, '/' );
	/*
	 * Set line number.
	 */
	bp->lineno = lineno;
	/*
	 * Loop until line number or end of file is reached
	 * and set the line buffer.
	 */
	while( lineno-- && !feof(fp) ){
		fgets( bp->line, 0xFF, fp );
	}

	fclose(fp);
	/*
	 * Set source file name, without the path.
	 */
	if(sep){
		strncpy( bp->source, sep + 1, 0xFF );
	}
	else{
		strncpy( bp->source, filename, 0xFF );
	}
	/*
	 * Finally append the new break point to the chain.
	 */
	ll_append( &dbg->bpoints, bp );
}

void dbg_trigger( dbg_t *dbg, vframe_t *frame, Node *node ){
	bpoint_t *bp;
	string    source = dbg->vm->source;
	size_t    lineno = dbg->vm->lineno;

	ll_foreach( &dbg->bpoints, llitem ){
		bp = ll_data( bpoint_t *, llitem );
		if( bp->lineno == lineno && bp->source == source ){
			printf( "BREAK :\n\t%s\n", bp->line );
			exit(0);
		}
	}
}
