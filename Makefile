WFLAGS= -w
OPTIMIZATION= -O3
CFLAGS= -Iinclude/ $(OPTIMIZATION) $(WFLAGS) `xml2-config --cflags --libs` -funroll-loops -ffast-math -fno-stack-protector
LFLAGS= -ldl -lpcrecpp -lcurl -lpthread
LIBXML= `xml2-config --cflags --libs`
PREFIX=/usr
TARGET=hybris


all: hybris
	cd examples && make

hybris: builtins
	g++ -c src/common.cpp $(CFLAGS)
	g++ -c src/lexer.cpp $(CFLAGS)
	g++ -c src/builtin.cpp $(CFLAGS)
	g++ -c src/vmem.cpp $(CFLAGS)
	g++ -c src/cmdline.cpp $(CFLAGS)
	g++ -c src/node.cpp $(CFLAGS)
	g++ -c src/object.cpp $(CFLAGS)
	g++ -c src/tree.cpp $(CFLAGS)
	g++ -c src/node.cpp $(CFLAGS)
	g++ -c src/vmem.cpp $(CFLAGS)
	g++ -c src/hybris.cpp $(CFLAGS)
	g++ -c src/parser.cpp $(CFLAGS)
	g++ *.o src/builtins/*.o -o $(TARGET) $(CFLAGS) $(LFLAGS)
	ar rcs lib$(TARGET).a *.o src/builtins/*.o

builtins: parser
	g++ -c src/builtins/type.cc -o src/builtins/type.o $(CFLAGS)
	g++ -c src/builtins/math.cc -o src/builtins/math.o $(CFLAGS)
	g++ -c src/builtins/array.cc -o src/builtins/array.o $(CFLAGS)
	g++ -c src/builtins/map.cc -o src/builtins/map.o $(CFLAGS)
	g++ -c src/builtins/string.cc -o src/builtins/string.o $(CFLAGS)
	g++ -c src/builtins/pcre.cc -o src/builtins/pcre.o $(CFLAGS)
	g++ -c src/builtins/conio.cc -o src/builtins/conio.o $(CFLAGS)
	g++ -c src/builtins/process.cc -o src/builtins/process.o $(CFLAGS)
	g++ -c src/builtins/reflection.cc -o src/builtins/reflection.o $(CFLAGS)
	# dllcall could not be optimized due to stack preservation issue
	g++ -c -Iinclude/ src/builtins/dll.cc -o src/builtins/dll.o $(LIBXML)
	g++ -c src/builtins/time.cc -o src/builtins/time.o $(CFLAGS)
	g++ -c src/builtins/fileio.cc -o src/builtins/fileio.o $(CFLAGS)
	g++ -c src/builtins/netio.cc -o src/builtins/netio.o $(CFLAGS)
	g++ -c src/builtins/http.cc -o src/builtins/http.o $(CFLAGS)
	g++ -c src/builtins/xml.cc -o src/builtins/xml.o $(CFLAGS)
	g++ -c src/builtins/encoding.cc -o src/builtins/encoding.o $(CFLAGS)
	g++ -c src/builtins/pthreads.cc -o src/builtins/pthreads.o $(CFLAGS)
	g++ -c src/builtins/matrix.cc -o src/builtins/matrix.o $(CFLAGS)

parser: lexer
	bison -y -d -o src/parser.cpp src/parser.y.cpp

lexer:
	flex -o src/lexer.cpp src/lexer.l.cpp

clean:
	rm -f src/lexer.cpp src/parser.hpp src/parser.cpp *.o src/*.o src/builtins/*.o $(TARGET)
	rm -f lib$(TARGET).a
	cd examples && make clean

install:
	install -m 0755 $(TARGET) $(PREFIX)/bin/
	mkdir -p $(PREFIX)/include/$(TARGET)
	cp include/*.h $(PREFIX)/include/$(TARGET)/
	mkdir -p $(PREFIX)/lib/$(TARGET)
	mkdir -p $(PREFIX)/lib/$(TARGET)/libs
	mkdir -p $(PREFIX)/lib/$(TARGET)/modules
	chmod -R 777 $(PREFIX)/lib/$(TARGET)/
	install -m 0644 lib$(TARGET).a $(PREFIX)/lib
	ldconfig
