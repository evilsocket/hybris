// dichiaro una funzione che stampa il valore che riceve come argomento
function a_function( a ){
	print( "a_function::a = ".a."\n" );	
}

// questa funzione, sfruttando l'alias 'f' lo usa come fosse una funzione
function docall( f, arg ){
	f( arg );
}

// creo la variabile alias 'f' che punta alla funzione 'a_function'
f = a_function;

// eseguo la chiamata, passando a docall l'alias 'f'
docall( f, "10" );

// tramite l'operatore $ è possibile richiamare un alias con una stringa che contiene il suo nome
($"f")( "composta" );
