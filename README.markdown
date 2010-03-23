###  Hybris
Copyleft by Simone Margaritelli <evilsocket@gmail.com> & The Hybris Dev Team
Infos & Documentation at <http://www.hybris-lang.org/>

### Cosa è Hybris ?

Hybris, acronimo (non molto fantasioso lo riconosco!) di hybrid scripiting language, è un linguaggio di programmazione, nella fattispecie di scripting, procedurale al quale sto lavorando da diversi mesi. Il tutto è nato come un semplice gioco, dal fascino che provavo nel pensare di poter scrivere un interprete, se pur rudimentale, di un qualche tipo di linguaggio, interprete che con il tempo ho sviluppato fino ad arrivare ad un “prodotto” perfettamente utilizzabile e con enormi vantaggi rispetto ad altre sintassi e/o funzionalità.

### Perchè dovrebbe essere più comodo di altri linguaggi ?

Innanzitutto, devo precisare che durante lo sviluppo e l’impostazione della grammatica di base, ho raccolto le caratteristiche, le funzionalità, ecc che reputavo positive e utili nei linguaggi di programmazione che attualmente conosco. Di conseguenza si può pensare ad Hybris come ad un ibrido (da li il nome) di vari linguaggi che prende la parte migliore di ognuno.
A partire dalla sintassi semplice e pulita del PHP, fino ad arrivare al supporto nativo per le espressioni regolari con relativo operatore del Perl o ad una certa interazione a basso livello come nel C .
Questo è il primo aspetto che già per definizione lo rende in un certo qual modo migliore di altri linguaggi.
Un altra delle caratteristiche alle quali ho pensato durante la sua creazione, è stata quella di includere nativamente alcune funzionalità che, se pur supportate da altri linguaggi, costituiscono delle porzioni “modulari” di questi altri interpreti, ovvero non risiedono nativamente nell’applicativo dell’interprete bensì in moduli dinamici appositamente installati per estendere le funzionalità del linguaggio.
Nonostante lo stesso Hybris supporti una certa modularità, ho pensato che alcune features come la parserizzazione nativa di flussi XML, il supporto alle espressioni regolari, metodi semplici ed intuitivi per effettuare connessioni HTTP e HTTPS, ecc dovessero essere integrate nel linguaggio stesso considerando che ormai sono realtà che fanno parte del mondo informatico di tutti i giorni.
Detto in parole povere, Hybris sarà in grado di parserizzare un flusso XML senza l’installazione di moduli aggiuntivi (vedere il sorgente di esempio xml.hs), così come sarà in grado di effettuare richieste http con una facilità estrema (esempio http.hs) o di sfruttare le espressioni regolari con un operatore =~ nativo (esempio pcre_operator.hs), il tutto, ribadisco, senza installare altro se non l’interprete principale.
Diciamo che come linguaggio segue una filosofia “AIO” (All In One) continuando tuttavia a supportare dei moduli esterni tramite i quali ognuno potrà espanderlo come preferisce …. comodo no ? ^^
