WFLAGS= -w
OPTIMIZATION= -O3
CFLAGS= $(OPTIMIZATION) $(WFLAGS) `xml2-config --cflags --libs` -funroll-loops -fomit-frame-pointer -ffast-math -fno-stack-protector -ffunction-sections
LFLAGS= -ldl -lpcrecpp -lcurl
LIBXML= `xml2-config --cflags --libs`

all: builtins
	g++ -c common.cpp $(CFLAGS)
	g++ -c lexer.cpp $(CFLAGS)
	g++ -c builtin.cpp $(CFLAGS)
	g++ -c vmem.cpp $(CFLAGS)
	g++ parser.cpp builtins/*.o *.o -o hybris $(CFLAGS) $(LFLAGS)
debug: builtins_dbg
	g++ -c common.cpp -g3 -pg $(LIBXML)
	g++ -c lexer.cpp -g3 -pg $(LIBXML)
	g++ -c builtin.cpp -g3 -pg $(LIBXML)
	g++ -c vmem.cpp -g3 -pg $(LIBXML)
	g++ parser.cpp builtins/*.o *.o -o hybris -g3 -pg $(LFLAGS) $(LIBXML)
builtins: parser
	g++ -c builtins/type.cc -o builtins/type.o $(CFLAGS)
	g++ -c builtins/math.cc -o builtins/math.o $(CFLAGS)
	g++ -c builtins/array.cc -o builtins/array.o $(CFLAGS)
	g++ -c builtins/map.cc -o builtins/map.o $(CFLAGS)
	g++ -c builtins/string.cc -o builtins/string.o $(CFLAGS)
	g++ -c builtins/pcre.cc -o builtins/pcre.o $(CFLAGS)
	g++ -c builtins/conio.cc -o builtins/conio.o $(CFLAGS)
	g++ -c builtins/process.cc -o builtins/process.o $(CFLAGS)
	g++ -c builtins/reflection.cc -o builtins/reflection.o $(CFLAGS)
	# dllcall could not be optimized due to stack preservation issue
	g++ -c builtins/dll.cc -o builtins/dll.o $(LIBXML)
	g++ -c builtins/time.cc -o builtins/time.o $(CFLAGS)
	g++ -c builtins/fileio.cc -o builtins/fileio.o $(CFLAGS)
	g++ -c builtins/netio.cc -o builtins/netio.o $(CFLAGS)
	g++ -c builtins/http.cc -o builtins/http.o $(CFLAGS)
	g++ -c builtins/xml.cc -o builtins/xml.o $(CFLAGS)
builtins_dbg: parser
	g++ -c builtins/type.cc -o builtins/type.o -g3 -pg $(LIBXML)
	g++ -c builtins/math.cc -o builtins/math.o -g3 -pg $(LIBXML)
	g++ -c builtins/array.cc -o builtins/array.o -g3 -pg $(LIBXML)
	g++ -c builtins/map.cc -o builtins/map.o -g3 -pg $(LIBXML)
	g++ -c builtins/string.cc -o builtins/string.o -g3 -pg $(LIBXML)
	g++ -c builtins/pcre.cc -o builtins/pcre.o -g3 -pg $(LIBXML)
	g++ -c builtins/conio.cc -o builtins/conio.o -g3 -pg $(LIBXML)
	g++ -c builtins/process.cc -o builtins/process.o -g3 -pg $(LIBXML)
	g++ -c builtins/reflection.cc -o builtins/reflection.o -g3 -pg $(LIBXML)
	g++ -c builtins/dll.cc -o builtins/dll.o -g3 -pg $(LIBXML)
	g++ -c builtins/time.cc -o builtins/time.o -g3 -pg $(LIBXML)
	g++ -c builtins/fileio.cc -o builtins/fileio.o -g3 -pg $(LIBXML)
	g++ -c builtins/netio.cc -o builtins/netio.o -g3 -pg $(LIBXML)
	g++ -c builtins/http.cc -o builtins/http.o -g3 -pg $(LIBXML)
	g++ -c builtins/xml.cc -o builtins/xml.o -g3 -pg $(LIBXML)
lexer:
	flex -o lexer.cpp lexer.l.cpp
parser: lexer
	bison -y -d -o parser.cpp parser.y.cpp
clean:
	rm -f lexer.cpp parser.hpp parser.cpp *.o builtins/*.o hybris

