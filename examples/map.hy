// creo una mappa con elementi e chiavi di tipologia mista e la stampo
m = map( "zero" -> 0, 
		 "uno" -> 2 - 1, 
		 "due" -> 2.0, 
		 "tre" -> array( 0, 1, 2, 3 ) );
print(m);

// stampo l'elemento con chiave 'due'
println( m["due"] );

// in questo contesto, 144 non è considerato un indice come nel caso degli array, ma una chiave a sua volta
// di conseguenza in questo modo sto mappando la stringa "lulz" sulla chiave 144 
m[144] = "lulz";
print(m);

// rimuovo l'elemento mappato con la chiave "uno"
unmap(m,"uno");
print(m);

// anche un decimale può essere usato come chiave
m[12.4] = "dodici virgola quattro";

// così come un elemento della mappa può essere una mappa a sua volta
m["lol"] = map( "Ciao" -> "mao",
				"Bau"  -> array( m ) );
print(m);

/* nel caso delle mappe, il costrutto foreach ha la seguende sintassi :
		foreach( <chiave> -> <valore> of <mappa> )	
*/					
foreach( key -> value of m ){
	// se il valore è una mappa o un array, stampo la sua struttura
	if( isarray(value) || ismap(value) ){
		print( key." : " );
		print(value);
	}
	// se altrimenti è un tipo semplice, stampo direttamente il suo valore
	else{
		print( key." : ".value."\n" );	
	}
}

