struct Persona {
	genere;
	nome, cognome;
	anni;
}

guy	   = Persona( 'M', "Mario", "Rossi", 24 );
packed = pack( guy, 1, 5, 5, 1 );

println(packed);
