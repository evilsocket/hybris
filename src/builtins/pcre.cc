#include "common.h"
#include "vmem.h"
#include "builtin.h"
#include <pcrecpp.h>

#define H_PCRE_MAX_MATCHES 300

#define H_PCRE_BOOL_MATCH    0x0
#define H_PCRE_MULTI_MATCH   0x1
#define H_PCRE_REPLACE_MATCH 0x2

int classify_pcre_regex( string& r ){							
	pcrecpp::RE_Options  OPTS( PCRE_CASELESS | PCRE_EXTENDED );
	pcrecpp::RE          MULTI_REGEX( ".*[^" +
									  pcrecpp::RE::QuoteMeta("\\") + "]" + 
									  pcrecpp::RE::QuoteMeta("(") + "[^" +
									  pcrecpp::RE::QuoteMeta(")") + "]+[^" + 
									  pcrecpp::RE::QuoteMeta("\\") + "]" + 
									  pcrecpp::RE::QuoteMeta(")") + ".*", OPTS );
	
	if( MULTI_REGEX.PartialMatch(r.c_str()) ){
		return H_PCRE_MULTI_MATCH;
	}
	else if(0){
		return H_PCRE_REPLACE_MATCH;
	}
	else{
		return H_PCRE_BOOL_MATCH;
	}
}

void parse_pcre_regex( string& raw, string& regex, int& opts ){
	unsigned int i;
	vector<string> blocks;
	pcrecpp::RE_Options  OPTS( PCRE_CASELESS | PCRE_EXTENDED );
	pcrecpp::RE          REGEX( "^/(.*?)/([i|m|s|x|U]*)$", OPTS );
	pcrecpp::StringPiece SUBJECT( raw );
	string   rex, sopts;

    while( REGEX.FindAndConsume( &SUBJECT, &rex, &sopts ) == true ){
		blocks.push_back( rex );
		blocks.push_back( sopts );
	}

	if( blocks.size() ){
		if( blocks.size() != 2 ){
			hybris_syntax_error( "invalid pcre regular expression syntax" );
		}
		opts  = 0;
		regex = blocks[0];
		sopts = blocks[1];

		/* parse options */
		for( i = 0; i < sopts.size(); i++ ){
			switch( sopts[i] ){
				case 'i' : opts |= PCRE_CASELESS;  break;
				case 'm' : opts |= PCRE_MULTILINE; break;
				case 's' : opts |= PCRE_DOTALL;    break;
				case 'x' : opts |= PCRE_EXTENDED;  break;
				case 'U' : opts |= PCRE_UNGREEDY;  break;
			}
		}
	}
	else{
		opts  = 0;
		regex = raw;
	}
}

Object *hrex_operator( Object *o, Object *regexp ){
	htype_assert( o, 	  H_OT_STRING );
	htype_assert( regexp, H_OT_STRING );
	
	string rawreg  = regexp->xstring,
		   subject = o->xstring,
		   regex;
	int    opts;
	
	parse_pcre_regex( rawreg, regex, opts );
	
	if( classify_pcre_regex( regex ) == H_PCRE_BOOL_MATCH ){
		pcrecpp::RE_Options OPTS(opts);
		pcrecpp::RE         REGEX( regex.c_str(), OPTS );

		return new Object( REGEX.PartialMatch(subject.c_str()) );
	}
	else{
		pcrecpp::RE_Options  OPTS(opts);
		pcrecpp::RE          REGEX( regex.c_str(), OPTS );
		pcrecpp::StringPiece SUBJECT( subject.c_str() );
		string   match;
		Object *matches = new Object();
		int i = 0;

		while( REGEX.FindAndConsume( &SUBJECT, &match ) == true ){
			if( i++ > H_PCRE_MAX_MATCHES ){
				hybris_generic_error( "something of your regex is forcing infinite matches" );
			}
			matches->push( new Object((char *)match.c_str()) );
		}

		return matches;
	}
}

HYBRIS_BUILTIN(hrex_match){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'rex_match' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );

	string rawreg  = data->at(0)->xstring,
		   subject = data->at(1)->xstring,
		   regex;
	int    opts;

	parse_pcre_regex( rawreg, regex, opts );

	pcrecpp::RE_Options OPTS(opts);
	pcrecpp::RE         REGEX( regex.c_str(), OPTS );

	return new Object( REGEX.PartialMatch(subject.c_str()) );
}

HYBRIS_BUILTIN(hrex_matches){
	if( data->size() != 2 ){
		hybris_syntax_error( "function 'rex_matches' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );

	string rawreg  = data->at(0)->xstring,
		   subject = data->at(1)->xstring,
		   regex;
	int    opts, i = 0;

	parse_pcre_regex( rawreg, regex, opts );

	pcrecpp::RE_Options  OPTS(opts);
	pcrecpp::RE          REGEX( regex.c_str(), OPTS );
	pcrecpp::StringPiece SUBJECT( subject.c_str() );
	string   match;
	Object *matches = new Object();

    while( REGEX.FindAndConsume( &SUBJECT, &match ) == true ){
		if( i++ > H_PCRE_MAX_MATCHES ){
			hybris_generic_error( "something of your regex is forcing infinite matches" );
		}
		matches->push( new Object((char *)match.c_str()) );
	}

	return matches;
}

HYBRIS_BUILTIN(hrex_replace){
	if( data->size() != 3 ){
		hybris_syntax_error( "function 'rex_replace' requires 2 parameters (called with %d)", data->size() );
	}
	htype_assert( data->at(0), H_OT_STRING );
	htype_assert( data->at(1), H_OT_STRING );
	htype_assert( data->at(2), H_OT_STRING, H_OT_CHAR );

	string rawreg  = data->at(0)->xstring,
		   subject = data->at(1)->xstring,
		   replace = (data->at(2)->xtype == H_OT_STRING ? data->at(2)->xstring : string("") + data->at(2)->xchar),
		   regex;
	int    opts;

	parse_pcre_regex( rawreg, regex, opts );

	pcrecpp::RE_Options OPTS(opts);
	pcrecpp::RE         REGEX( regex.c_str(), OPTS );

	REGEX.GlobalReplace( replace.c_str(), &subject );

	return new Object( (char *)subject.c_str() );
}
