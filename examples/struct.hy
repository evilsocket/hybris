struct Carattere {
	simpatico;
	sveglio;
	modesto;
}

struct Persona {
	genere;
	nome, cognome;
	anni;
	
	carattere;
}

guy		  	   = Persona( 'M', "Mario", "Rossi", 24 );
guy->carattere = Carattere( "Si", "No", "Si" );

println(guy);

if( guy->anni >= 18 ){
	println( guy->nome." ".guy->cognome." è maggiorenne." );
}
else{
	println( guy->nome." ".guy->cognome." è minorenne." );
}

if( guy->carattere->sveglio == "Si" ){
	if( guy->carattere->modesto == "No" ){
		println( guy->nome." ".guy->cognome." è una persona arrogante." );
	}
	else{
		println( guy->nome." ".guy->cognome." è un tipo sveglio." );
	}
}
else{
	println( guy->nome." ".guy->cognome." è un po tonto." );
}


