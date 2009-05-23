#!/home/evilsocket/Desktop/hybris/hybris -e
print( "TEST XML_LOAD :\n\n" );
map   = xml_load( "test.xml" );
foods = map["food"];

foreach( food of foods ){
	name  = food["name"][0]["<data>"];
	price = food["price"][0]["<data>"];
	print( "\t".name." : ".price."$\n" );	
}

print( "\nTEST XML_PARSE :\n\n" );
xml = "<root><child a=\"123\">1234\n<asd></asd></child></root>";

println( xml."\n" );
println( xml_parse(xml) );

print( "\nTEST XML_PARSE DA HTTP :\n\n" );
xml = http_get( "www.hackinthebox.org", "/backend.php" );
map = xml_parse(xml);

foreach( item of map["channel"][0]["item"] ){
	print( "\t".item["pubDate"][0]["<data>"]." : ".item["title"][0]["<data>"]."\n" );
} 
