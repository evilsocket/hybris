xml  = http_get( "evilsocket.net", "/feed" );
root = xml_parse(xml);

println(root);
