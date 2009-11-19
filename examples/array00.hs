// inizializzo un array con valori di tipi misti, uno dei quali è un array a sua volta
a     = array( 0, 
			   1.0 * 1, 
			   array( 2, "due", 2.0 ), 
			   3 );
// la variabile intera 'elems' ora contiene il numero di elementi dell'array 'a'
elems = elements(a);

println( "L'array 'a' ha ".elems." elementi :\n" );
// stampo ogni elemento di 'a', nel caso dell'array all'indice 2 verrà stampata la sua struttura 
for( i = 0; i < elems; i++ ){
	print( "elemento ".i." : " );
	println( a[i] );	
}

// rimuovo il secondo elemento da 'a' e stampo nuovamente il contenuto
println( "\na - 2° elemento : \n" );
remove( a, 1 );
elems = elements(a);
for( i = 0; i < elems; i++ ){
	print( "elemento ".i." : " );
	println( a[i] );	
}

/* eseguo un loop con il costrutto 'foreach' dove la sintassi è :
		foreach( <elemento> of <array> )
*/
println( "\nTest foreach : \n" );
foreach( item of a ){
	print("item = ");
	print(item);	
	// stampo il tipo di ogni elemento
	println( "\t".typeof(item) );
}

// rimuovo gli elementi residui di a
println( "\nSvuoto 'a' :\n" );
for( i = 0; i < elems; i++ ){
	pop(a);
	println(a);	
}

// dichiaro un array di array che può essere considerato come una matrice
matrice = array( array( 0, 1 ), array( 2, 3 ) );
print( "\nMATRICE : " );
println( matrice );

