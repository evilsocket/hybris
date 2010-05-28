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
#include "hybris.h"
#include <getopt.h>

int hyb_banner(){
    fprintf( stdout, "Hybris %s (built: %s %s)\n"
            "Released under GPL v3.0 by %s\n"
            "* Compiled with :\n"
            " - Import path  : %s\n"
            " - Include path : %s\n",
            VERSION,
            __DATE__,
            __TIME__,
            AUTHOR,
            LIB_PATH,
            INC_PATH );
}

int hyb_usage( char *argvz ){
    hyb_banner();
    fprintf( stdout, "\nUsage: %s <options> <file>\n\n"
    		"Where <options> is one or more among followring values :\n"
    		"\t-h (--help)    : Print this menu and exit.\n"
    		"\t-m (--mem)     : Set the allocation memory limit expressed in bytes (see --gc, default is 128MB). \n"
    		"\t-g (--gc)      : Set the garbage collection memory threshold, expressed in bytes, \n"
    		"\t                 kilobytes (with K postfix) or megabytes (with M postfix).\n"
    		"\t                 i.e. -g 10K or -g 1024 or --gc=100M\n"
    		"\t-c (--cgi)     : Run in CGI mode (stderr will be redirected to stdout).\n"
            "\t-t (--time)    : Compute execution time and print it to stdout.\n"
            "\t-s (--trace)   : Enable stack trace report on errors .\n\n", argvz );
    return 0;
}

int main( int argc, char *argv[], char* envp[] ){
    static struct option options[] = {
    		{ "mem",     1, 0, 'm' },
            { "gc",      1, 0, 'g' },
            { "cgi",	 0, 0, 'c' },
            { "time",    0, 0, 't' },
            { "trace",   0, 0, 's' },
            { "help",    0, 0, 'h' },
            { 0, 0, 0, 0 }
    };

    extern vm_t *__hyb_vm;
    extern int yyparse(void);

    __hyb_vm = vm_create();

    int index = 0;
    char c, multiplier, *p;
    long gc_threshold,
		 mm_threshold;

    while( (c = getopt_long( argc, argv, "m:g:ctsh", options, &index)) != -1 ){
        switch (c) {
			/*
			 * Handle garbage collection threshold argument.
			 * Allowed values are :
			 *
			 * nnn  (bytes)
			 * nnnK (kilo bytes)
			 * nnnM (mega bytes)
			 *
			 * Where nnn is the number of bytes, K is the kilo multiplier (1024),
			 * and M the mega multiplier (1024^2).
			 */
			case 'g':
				p = optarg;
				/*
				 * Shift the pointer until first non digit character is reached
				 * or the end of the string is reached.
				 */
				while( *(++p) != 0x00 && *p >= '0' && *p <= '9' );

				multiplier = *p;
				/*
				 * Optarg now contains only the integer part of the argument.
				 */
				optarg[ p - optarg ] = 0x00;

				gc_threshold = atol(optarg);
				/*
				 * Check for valid integer values.
				 */
				if( gc_threshold == 0 ){
					hyb_error( H_ET_GENERIC, "Invalid memory size %s given.", optarg );
				}
				/*
				 * Check for a valid multiplier.
				 */
				else if( multiplier != 0x00 && strchr( "kKmM", multiplier ) == 0 ){
					hyb_error( H_ET_GENERIC, "Invalid multiplier %c given.", multiplier );
				}
				/*
				 * Perform multiplication if multiplier was specified (multiplier != 0x00)
				 */
				switch(multiplier){
					case 'K' :
					case 'k' :
						gc_threshold *= 1024;
					break;

					case 'M' :
					case 'm' :
						gc_threshold *= 1048576;
					break;
				}
				/*
				 * Check for integer overflow.
				 */
				if( gc_threshold <= 0 ){
					hyb_error( H_ET_GENERIC, "Memory limit is too high." );
				}
				/*
				 * Done, let's pass it to the virtual machine arguments structure.
				 */
				__hyb_vm->args.gc_threshold = gc_threshold;
			break;

			/*
			 * Handle memory usage threshold argument.
			 * Allowed values are :
			 *
			 * nnn  (bytes)
			 * nnnK (kilo bytes)
			 * nnnM (mega bytes)
			 *
			 * Where nnn is the number of bytes, K is the kilo multiplier (1024),
			 * and M the mega multiplier (1024^2).
			 */
			case 'm':
				p = optarg;
				/*
				 * Shift the pointer until first non digit character is reached
				 * or the end of the string is reached.
				 */
				while( *(++p) != 0x00 && *p >= '0' && *p <= '9' );

				multiplier = *p;
				/*
				 * Optarg now contains only the integer part of the argument.
				 */
				optarg[ p - optarg ] = 0x00;

				mm_threshold = atol(optarg);
				/*
				 * Check for valid integer values.
				 */
				if( mm_threshold == 0 ){
					hyb_error( H_ET_GENERIC, "Invalid memory size %s given.", optarg );
				}
				/*
				 * Check for a valid multiplier.
				 */
				else if( multiplier != 0x00 && strchr( "kKmM", multiplier ) == 0 ){
					hyb_error( H_ET_GENERIC, "Invalid multiplier %c given.", multiplier );
				}
				/*
				 * Perform multiplication if multiplier was specified (multiplier != 0x00)
				 */
				switch(multiplier){
					case 'K' :
					case 'k' :
						mm_threshold *= 1024;
					break;

					case 'M' :
					case 'm' :
						mm_threshold *= 1048576;
					break;
				}
				/*
				 * Check for integer overflow.
				 */
				if( mm_threshold <= 0 ){
					hyb_error( H_ET_GENERIC, "Memory limit is too high." );
				}
				/*
				 * Done, let's pass it to the virtual machine arguments structure.
				 */
				__hyb_vm->args.mm_threshold = mm_threshold;
			break;

        	case 't':
        		/*
        		 * Enable execution time measurement.
        		 */
        		__hyb_vm->args.tm_timer = 1;
        	break;

        	case 'c':
        		/*
        		 * Redirect stderr to stdout in CGI mode.
        		 */
        		__hyb_vm->args.cgi_mode = true;
        	break;

        	case 's':
        		/*
        		 * Enable stack trace printing upon error.
        		 */
        		__hyb_vm->args.stacktrace = 1;
        	break;

        	case 'h':
        		return hyb_usage(argv[0]);
            break;
        }
    }

    if( optind < argc ){
        strncpy( __hyb_vm->args.source, argv[optind], sizeof(__hyb_vm->args.source) );
		if( hyb_file_exists(__hyb_vm->args.source) == 0 ){
			hyb_error( H_ET_GENERIC, "'%s' no such file or directory", __hyb_vm->args.source );
		}
    }
    /*
     * vm_t will receive every argument starting from the script
     * name to build the script virtual argv.
     */
    vm_init( __hyb_vm, optind, &argc, &argv, envp );

    extern FILE *yyin;
    /*
     * At this point, yyin could be a file handle if a source was specified
     * or the stdin handle if not, in this case the interpreter will execute
     * user input.
     */
    yyin = vm_fopen( __hyb_vm );

    __hyb_vm->state = vmParsing;

    while( !feof(yyin) ){
        yyparse();
    }

    vm_fclose( __hyb_vm );
    vm_release( __hyb_vm );
    vm_free( __hyb_vm );

    return 0;
}

