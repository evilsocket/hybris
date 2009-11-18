#!/usr/bin/hybris

str  = "Yes we cunt !\t^_^'";
urle = urlencode(str);
b64  = base64encode(str);

println( "Base string : ".str );
println( "Url encoded : ".urle );
println( "Url decoded : ".urldecode(urle) );
println( "B64 encoded : ".b64 );
println( "B64 decoded : ".base64decode(b64) );
