CC=g++
WFLAGS= -w
OPTIMIZATION= -O3 -funroll-loops -fno-rtti -fomit-frame-pointer -ffast-math -fno-stack-protector -ffunction-sections
CFLAGS= -Iinclude/ $(WFLAGS) $(OPTIMIZATION) 
LFLAGS= -ldl -lpcrecpp -lpthread
LIBXML_CFLAGS= `xml2-config --cflags`
LIBXML_LFLAGS= `xml2-config --libs`
LIBFFI_CFLAGS= `pkg-config libffi --cflags`
LIBFFI_LFLAGS= `pkg-config libffi --libs`
STDLIB_LFLAGS= -ldl -lpcrecpp -lcurl -lpthread $(LIBXML_LFLAGS) $(LIBFFI_LFLAGS) 
STDLIB_CFLAGS= $(WFLAGS) -L. -I./include/ $(OPTIMIZATION) -fPIC $(LIBXML_CFLAGS) $(LIBFFI_CFLAGS)  -lhybris -lc -shared
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
	   src/parser.lo \
	   src/lexer.lo

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

STDSRC=stdlib/std/hashing/md5.cc \
	   stdlib/std/hashing/crc32.cc \
	   stdlib/std/hashing/sha1.cc \
	   stdlib/std/hashing/sha2.cc \
	   stdlib/std/io/network/http.cc \
	   stdlib/std/io/network/tcp.cc \
	   stdlib/std/io/file.cc \
	   stdlib/std/io/console.cc \
	   stdlib/std/io/xml.cc \
	   stdlib/std/type/reflection.cc \
	   stdlib/std/type/string.cc \
	   stdlib/std/type/matrix.cc \
	   stdlib/std/type/array.cc \
	   stdlib/std/type/map.cc \
	   stdlib/std/type/type.cc \
	   stdlib/std/type/binary.cc \
	   stdlib/std/encoding.cc \
	   stdlib/std/os/dll.cc \
	   stdlib/std/os/time.cc \
	   stdlib/std/os/threads.cc \
	   stdlib/std/os/process.cc \
	   stdlib/std/pcre.cc \
	   stdlib/std/math.cc
		
OBJECTS=$(SOURCES:.cpp=.o)
LOBJECTS=$(SOURCES:.cpp=.lo)
STDOBJ=$(STDSRC:.cc=.so)

all: src/lexer.cpp src/parser.cpp lib hybris stdlib

lib: $(LOBJECTS)
	$(CC) -shared -Wl,-soname,libhybris.so.1 -o lib$(TARGET).so.1.0 $(LIBOBJ)
	ln -sf lib$(TARGET).so.1.0 lib$(TARGET).so
	ln -sf lib$(TARGET).so.1.0 lib$(TARGET).so.1
	
hybris: $(OBJECTS)
	$(CC) src/*.o -o $(TARGET) $(CFLAGS) $(LFLAGS)

stdlib: $(STDOBJ)
		
.cpp.o: 
	$(CC) -c $< -o $@ $(CFLAGS)
	
.cpp.lo:
	$(CC) -c $< -o $@ $(CFLAGS) -fPIC

.cc.so: 
	mkdir -p `dirname $< | sed -e 's/stdlib/stdlib\/build/'`
	g++ $< -o `dirname $< | sed -e 's/stdlib/stdlib\/build/'`/`basename $< .cc`.so $(STDLIB_CFLAGS) $(STDLIB_LFLAGS)
	
src/parser.cpp: src/parser.y.cpp
	bison -y -d -o $@ $? 

src/lexer.cpp: src/lexer.l.cpp
	flex -o $@ $? 

clean:
	rm -f src/lexer.cpp include/lexer.h include/parser.h src/parser.hpp src/parser.cpp src/*.o src/*.lo $(TARGET)
	rm -f lib$(TARGET).so.1.0
	rm -f lib$(TARGET).so
	rm -f lib$(TARGET).so.1
	rm -rf stdlib/build
	
install:
	install -m 0755 $(TARGET) $(PREFIX)/bin/
	mkdir -p $(PREFIX)/include/$(TARGET)
	cp include/*.h $(PREFIX)/include/$(TARGET)/
	mkdir -p $(PREFIX)/lib/$(TARGET)
	mkdir -p $(PREFIX)/lib/$(TARGET)/include
	mkdir -p $(PREFIX)/lib/$(TARGET)/library
	chmod -R 777 $(PREFIX)/lib/$(TARGET)/
	install -m 0644 lib$(TARGET).so.1.0 $(PREFIX)/lib
	ln -sf $(PREFIX)/lib/lib$(TARGET).so.1.0 $(PREFIX)/lib/lib$(TARGET).so
	ln -sf $(PREFIX)/lib/lib$(TARGET).so.1.0 $(PREFIX)/lib/lib$(TARGET).so.1
	ldconfig
	cp -rf stdlib/build/* /usr/lib/hybris/library/

uninstall:
	rm -rf $(PREFIX)/bin/$(TARGET) 
	rm -rf $(PREFIX)/lib/$(TARGET)
	rm -rf $(PREFIX)/include/$(TARGET)
	rm -rf $(PREFIX)/lib/lib$(TARGET).so.1.0
	rm -rf $(PREFIX)/lib/lib$(TARGET).so
	rm -rf $(PREFIX)/lib/lib$(TARGET).so.1
