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
#include "builtin.h"

void hmodule_load( char *module ){
    void *hmodule = dlopen( module, RTLD_NOW );
    if( !hmodule ){
        char *error = dlerror();
        if( error == NULL ){
            hybris_generic_warning( "module '%s' could not be loaded", module );
        }
        else{
            hybris_generic_warning( "%s", error );
        }
        return;
    }
    /* load module name */
    char *modname = (char *)dlsym( hmodule, "hybris_module_name" );
    if( !modname ){
        dlclose(hmodule);
        hybris_generic_warning( "could not find module name symbol in '%s'", module );
        return;
    }
    /* load initialization routine, usually used for constants definition, etc */
    initializer_t initializer = (initializer_t)dlsym( hmodule, "hybris_module_init" );
    if(initializer){
        extern vmem_t  HVM;
        extern vcode_t HVC;
        initializer( &HVM, &HVC, &HDYNAMICMODULES );
    }

    /* exported functions vector */
    builtin_t *functions = (builtin_t *)dlsym( hmodule, "hybris_module_functions" );
    if(!functions){
        dlclose(hmodule);
        hybris_generic_warning( "could not find module '%s' functions pointer", module );
        return;
    }

    module_t *hmod   = new module_t;
    hmod->name        = modname;
    hmod->initializer = initializer;
    unsigned long i   = 0;

    while( functions[i].function != NULL ){
        builtin_t *function = new builtin_t;
        function->identifier = functions[i].identifier;
        function->function   = functions[i].function;
        hmod->functions.push_back(function);
        i++;
    }

    HDYNAMICMODULES.push_back(hmod);
    //dlclose(hmodule);
}

function_t hfunction_search( char *identifier ){
    unsigned int i, j, nbuiltins = NBUILTINS, ndyns = HDYNAMICMODULES.size(), nfuncs;
    /* firs search the function in builtins symbols */
    for( i = 0; i < nbuiltins; i++ ){
        if( HSTATICBUILTINS[i].identifier == identifier ){
            return HSTATICBUILTINS[i].function;
        }
    }

    /* then search it in dynamic loaded modules */
    for( i = 0; i < ndyns; i++ ){
        /* for each function of the module */
        nfuncs = HDYNAMICMODULES[i]->functions.size();
        for( j = 0; j < nfuncs; j++ ){
            if( HDYNAMICMODULES[i]->functions[j]->identifier == identifier ){
                if( HDYNAMICMODULES[i]->initializer ){
                    extern vmem_t  HVM;
                    extern vcode_t HVC;
                    HDYNAMICMODULES[i]->initializer( &HVM, &HVC, &HDYNAMICMODULES );
                }
                return HDYNAMICMODULES[i]->functions[j]->function;
            }
        }
    }
    return H_UNDEFINED;
}
