#!/usr/bin/hybris

numero    = 123;
reference = &numero;
nuovo     = *reference;

print( "numero    : ".numero." (".typeof(numero).")\n" );
print( "reference : ".reference." (".typeof(reference).")\n" );
print( "nuovo     : ".nuovo." (".typeof(nuovo).")\n" );
