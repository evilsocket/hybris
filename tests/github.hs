xml = http_get( "github.com", "/evilsocket.atom" );
map = xml_parse(xml);

println(map);
