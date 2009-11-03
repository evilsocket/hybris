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
    initializer_t initializer = (initializer_t)( hmodule, "hybris_module_init" );
    if(initializer){
        extern vmem_t  HVM;
        extern vcode_t HVC;
        initializer( &HVM, &HVC );
    }
    /* number of functions exported */
    unsigned int nfunctions = (unsigned int)( hmodule, "hybris_module_nfunctions" );
    if(!nfunctions){
        dlclose(hmodule);
        hybris_generic_warning( "could not find number of functions exported by '%s'", module );
        return;
    }
    /* exported functions vector */
    builtin_t *functions = (builtin_t *)( hmodule, "hybris_module_functions" );
    if(!functions){
        dlclose(hmodule);
        hybris_generic_warning( "could not find module '%s' functions pointer", module );
        return;
    }

    module_t *hmod   = new module_t;
    hmod->name        = modname;
    hmod->initializer = initializer;
    unsigned int i;
    for( i = 0; i < nfunctions; i++ ){
        builtin_t *function = new builtin_t;
        function->identifier = functions[i].identifier;
        function->function   = functions[i].function;
        hmod->functions.push_back(function);
    }

    HDYNAMICMODULES.push_back(hmod);
    dlclose(hmodule);
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
                return HDYNAMICMODULES[i]->functions[j]->function;
            }
        }
    }
    return H_UNDEFINED;
}
