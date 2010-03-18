CC=g++
WFLAGS= -w
OPTIMIZATION= -O3
LIBXML_CFLAGS= `xml2-config --cflags`
LIBXML_LFLAGS= `xml2-config --libs`
CFLAGS= -Iinclude/ $(OPTIMIZATION) $(WFLAGS) $(LIBXML_CFLAGS) -funroll-loops -ffast-math -fno-stack-protector
LFLAGS= -ldl -lpcrecpp -lcurl -lpthread $(LIBXML_LFLAGS)
PREFIX=/usr
TARGET=hybris
LIBOBJ= src/builtin.o src/common.o src/hybris.o src/executors.o src/node.o src/object.o src/tree.o src/vmem.o src/builtins/*.o
SOURCES=src/common.cpp src/lexer.cpp src/builtin.cpp src/vmem.cpp src/node.cpp src/object.cpp src/tree.cpp \
		src/executors.cpp src/hybris.cpp src/parser.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: builtins hybris lib
	cd examples && make

lib: 
	ar rcs lib$(TARGET).a src/*.o $(LIBOBJ)

hybris: $(OBJECTS)
	$(CC) src/*.o src/builtins/*.o -o $(TARGET) $(CFLAGS) $(LFLAGS)
	
.cpp.o: 
	$(CC) -c $< -o $@ $(CFLAGS)
	
builtins: src/lexer.cpp src/parser.cpp
	cd src/builtins && make
	
src/parser.cpp: src/parser.y.cpp
	bison -y -d -o $@ $? 

src/lexer.cpp: src/lexer.l.cpp
	flex --header-file=include/lexer.h -o $@ $? 

clean:
	rm -f src/lexer.cpp include/lexer.h include/parser.h src/parser.hpp src/parser.cpp src/*.o $(TARGET)
	cd src/builtins && make clean
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
