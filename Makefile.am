CC=g++
WFLAGS= -w
OPTIMIZATION= -O3
LIBXML_CFLAGS= `xml2-config --cflags`
LIBXML_LFLAGS= `xml2-config --libs`
LIBFFI_CFLAGS= `pkg-config libffi --cflags`
LIBFFI_LFLAGS= `pkg-config libffi --libs`
CFLAGS= -Iinclude/ $(OPTIMIZATION) $(WFLAGS) $(LIBXML_CFLAGS) $(LIBFFI_CFLAGS) -funroll-loops -fno-rtti -fomit-frame-pointer -ffast-math -fno-stack-protector -ffunction-sections
LFLAGS= -ldl -lpcrecpp -lcurl -lpthread $(LIBXML_LFLAGS) $(LIBFFI_LFLAGS)
PREFIX=/usr
TARGET=hybris

LIBOBJ=src/context.lo \
	   src/common.lo \
	   src/engine.lo \
	   src/node.lo \
	   src/object.lo \
	   src/hashtable.lo \
	   src/vmem.lo \
	   src/vcode.lo \
	   src/builtins/*.lo

SOURCES=src/common.cpp \
		src/lexer.cpp \
		src/context.cpp \
		src/hashtable.cpp \
		src/vmem.cpp \
		src/vcode.cpp \
		src/node.cpp \
		src/object.cpp \
	    src/engine.cpp \
	    src/parser.cpp

OBJECTS=$(SOURCES:.cpp=.o)
LOBJECTS=$(SOURCES:.cpp=.lo)

all: lib builtins hybris
	cd examples && make

lib: lib_builtins $(LOBJECTS)
	ar rcs lib$(TARGET).a src/*.lo $(LIBOBJ)

hybris: $(OBJECTS)
	$(CC) src/*.o src/builtins/*.o -o $(TARGET) $(CFLAGS) $(LFLAGS)
	
.cpp.o: 
	$(CC) -c $< -o $@ $(CFLAGS)
	
.cpp.lo:
	$(CC) -c $< -o $@ $(CFLAGS) -fPIC

lib_builtins: src/lexer.cpp src/parser.cpp
	cd src/builtins && make lib

builtins: src/lexer.cpp src/parser.cpp
	cd src/builtins && make
	
src/parser.cpp: src/parser.y.cpp
	bison -y -d -o $@ $? 

src/lexer.cpp: src/lexer.l.cpp
	flex -o $@ $? 

clean:
	rm -f src/lexer.cpp include/lexer.h include/parser.h src/parser.hpp src/parser.cpp src/*.o src/*.lo $(TARGET)
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
