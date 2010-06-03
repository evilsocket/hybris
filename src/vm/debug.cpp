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
#include <readline/readline.h>
#include <readline/history.h>

void dbg_quit( dbg_t *dbg, char *args, vframe_t *frame, Node *node ){
	exit(0);
}

void dbg_bp( dbg_t *dbg, char *args, vframe_t *frame, Node *node ){
	char  *file = NULL,
		  *line = NULL;
	size_t nline;


	if( args == NULL ){
		printf( "bp command needs two arguments, type help for more info.\n" );
		return;
	}

	file = strchr( args, ' ' );
	if( !file ){
		printf( "bp command needs two arguments, type help for more info.\n" );
		return;
	}

	*file = 0;
	line  = file + 1;
	file  = args;
	nline = atoi(line);

	if( nline == 0 ){
		printf( "Invalid line number given.\n" );
		return;
	}

	FILE *fp	  = fopen( file, "rt" );
	bpoint_t *bp  = new bpoint_t;
	char	 *sep = strrchr( file, '/' );

	if( !fp ){
		printf( "Could not open %s.\n", file );
		return;
	}

	/*
	 * Set line number.
	 */
	bp->lineno = nline;
	/*
	 * Loop until line number or end of file is reached
	 * and set the line buffer.
	 */
	while( nline-- && !feof(fp) ){
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
		strncpy( bp->source, file, 0xFF );
	}
	/*
	 * Finally append the new break point to the chain.
	 */
	ll_append( &dbg->bpoints, bp );

	printf( "Set breakpoint on %s:%s.\n", file, line );
}

void dbg_watch( dbg_t *dbg, char *args, vframe_t *frame, Node *node ){

}

static dbg_command_t dbg_commands[] = {
	{ "quit", "Exit program execution.", dbg_quit },
	{ "bp",   "Set a breakpoint, example :\n"
			  "\t\t\tbp foo.hy 21\n"
			  "\t\t  Will set a breakpoint on line 21 of the file 'foo.hy'.", dbg_bp },
	{ "watch", "Evaluate a variable contentent given its name.", dbg_watch },
	{ "", "", NULL }
};

void dbg_init( dbg_t *dbg, vm_t *vm ){
	ll_init( &dbg->bpoints );
	dbg->vm = vm;
}

void dbg_main( dbg_t *dbg ){
	char *line      = NULL,
		 *cmd_name  = NULL,
		 *cmd_args  = NULL;
	bool  cmd_found = false;
	dbg_command_t *cmd;

	printf( "Type 'help' for the command reference.\n\n" );

	while(1){
		cmd_found = false;
		line 	  = readline( "DEBUG> " );
		if( line ){
			add_history(line);

			cmd_name = strchr( line, ' ' );

			if( cmd_name ){
				*cmd_name = 0;
				cmd_args = cmd_name + 1;
				cmd_name = line;
			}
			else{
				cmd_name = line;
				cmd_args = NULL;
			}

			if( strcmp( cmd_name, "go" ) == 0 ){
				free(line);
				return;
			}
			else if( strcmp( cmd_name, "help" ) == 0 ){
				printf( "help\t\t: Show this menu.\n"
						"go\t\t: Start or continue normal program execution.\n" );
				for( cmd = &dbg_commands[0]; cmd->handler; ++cmd ){
					printf( "%s\t\t: %s\n", cmd->name.c_str(), cmd->desc.c_str() );
				}
				printf("\n");
			}
			else{
				for( cmd = &dbg_commands[0]; cmd->handler; ++cmd ){
					if( cmd_name == cmd->name ){
						cmd_found = true;
						cmd->handler( dbg, cmd_args, 0, 0 );
					}
				}

				if( !cmd_found ){
					printf( "'%s' unknown command.\n", cmd_name );
				}
			}

			free(line);
		}
	}
}

void dbg_trigger( dbg_t *dbg, vframe_t *frame, Node *node ){
	bpoint_t *bp;
	string    source = dbg->vm->source;
	size_t    lineno = dbg->vm->lineno;

	ll_foreach( &dbg->bpoints, llitem ){
		bp = ll_data( bpoint_t *, llitem );
		if( bp->lineno == lineno && source.find( bp->source ) != string::npos  ){
			printf( "BREAK :\n\t%s\n", bp->line );
			dbg_main(dbg);
		}
	}
}
