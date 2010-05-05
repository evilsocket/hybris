CXX=g++
WFLAGS= -w
OPTIMIZATION= -O3 -pipe -fomit-frame-pointer -ffast-math
CFLAGS= -Iinclude/ $(WFLAGS) $(OPTIMIZATION) 
LFLAGS= -ldl -lpcre -lpthread
LIBXML_CFLAGS= `xml2-config --cflags`
LIBXML_LFLAGS= `xml2-config --libs`
LIBFFI_CFLAGS= `pkg-config libffi --cflags`
LIBFFI_LFLAGS= `pkg-config libffi --libs`
STDLIB_LFLAGS= -ldl -lpcre -lcurl -lpthread $(LIBXML_LFLAGS) $(LIBFFI_LFLAGS) 
STDLIB_CFLAGS= $(WFLAGS) -L. -I./include/ $(OPTIMIZATION) -fPIC $(LIBXML_CFLAGS) $(LIBFFI_CFLAGS)  -lhybris -lc -shared
PREFIX=/usr
TARGET=hybris

CHECK_FOR_ERRORS=if test -e hybris.tmp.errors; \
					then printf "\x1b[31;01m[ERRORS]\n\x1b[0m" && cat hybris.tmp.log && rm -f hybris.tmp.errors hybris.tmp.log && exit 1; \
				 elif test -s hybris.tmp.log; \
					then printf "\x1b[33;01m[WARNINGS]\n\x1b[0m" && cat hybris.tmp.log; \
				 else printf "\x1b[32;01m OK\n\x1b[0m"; \
				 fi && rm -f hybris.tmp.errors hybris.tmp.log
				 
LIBOBJ=src/vm.lo \
       src/gc.lo \
	   src/common.lo \
	   src/engine.lo \
	   src/node.lo \
	   src/types.lo \
	   src/types/binary.lo \
	   src/types/char.lo \
	   src/types/float.lo \
	   src/types/integer.lo \
	   src/types/map.lo \
	   src/types/matrix.lo \
	   src/types/string.lo \
	   src/types/structure.lo \
	   src/types/class.lo \
	   src/types/vector.lo \
	   src/types/reference.lo \
	   src/hashtable.lo \
	   src/memory.lo \
	   src/code.lo \
	   src/parser.lo \
	   src/lexer.lo

SOURCES=src/common.cpp \
		src/gc.cpp \
		src/lexer.cpp \
		src/vm.cpp \
		src/hashtable.cpp \
		src/memory.cpp \
		src/code.cpp \
		src/node.cpp \
		src/types.cpp \
		src/types/binary.cpp \
		src/types/char.cpp \
		src/types/float.cpp \
		src/types/integer.cpp \
		src/types/map.cpp \
		src/types/matrix.cpp \
		src/types/string.cpp \
		src/types/structure.cpp \
		src/types/class.cpp \
		src/types/vector.cpp \
	    src/types/reference.cpp \
	    src/engine.cpp \
	    src/parser.cpp \
		src/main.cpp

STDSRC=stdlib/std/hashing/md5.cc \
	   stdlib/std/hashing/crc32.cc \
	   stdlib/std/hashing/sha1.cc \
	   stdlib/std/hashing/sha2.cc \
	   stdlib/std/io/network/http.cc \
	   stdlib/std/io/network/tcp.cc \
	   stdlib/std/io/network/smtp.cc \
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
	   stdlib/std/gc.cc \
	   stdlib/std/pcre.cc \
	   stdlib/std/math.cc
		
OBJECTS=$(SOURCES:.cpp=.o)
LOBJECTS=$(SOURCES:.cpp=.lo)
STDOBJ=$(patsubst stdlib%.cc,build%.so, $(STDSRC))


debug: CFLAGS= -Iinclude/ $(WFLAGS) -g -pg
debug: all

all: src/lexer.cpp src/parser.cpp lib hybris stdlib

lib: $(LOBJECTS)	
	@printf "\x1b[33;01m@ Creating lib$(TARGET).so.1.0 ..."
	@$(CXX) -shared -Wl,-soname,libhybris.so.1 -o lib$(TARGET).so.1.0 $(LIBOBJ) $(STDLIB_LFLAGS) 2> hybris.tmp.log || touch hybris.tmp.errors
	@$(CHECK_FOR_ERRORS)
	@ln -sf lib$(TARGET).so.1.0 lib$(TARGET).so
	@ln -sf lib$(TARGET).so.1.0 lib$(TARGET).so.1
	
hybris: $(OBJECTS)
	@printf "\x1b[33;01m@ Creating $(TARGET) ..."
	@$(CXX) src/*.o src/types/*.o -o $(TARGET) $(CFLAGS) $(LFLAGS) 2> hybris.tmp.log || touch hybris.tmp.errors
	@$(CHECK_FOR_ERRORS)

stdlib: $(STDOBJ)
		
.cpp.o: 
	@printf "@ Compiling $< ..."
	@$(CXX) -c $< -o $@ $(CFLAGS) 2> hybris.tmp.log || touch hybris.tmp.errors
	@$(CHECK_FOR_ERRORS)
	
.cpp.lo:
	@printf "@ Compiling $< ..."
	@$(CXX) -c $< -o $@ $(CFLAGS) -fPIC 2> hybris.tmp.log || touch hybris.tmp.errors
	@$(CHECK_FOR_ERRORS)

build/%.so: stdlib/%.cc
	@printf "@ Compiling $< ..."
	@mkdir -p $(dir $@)
	@$(CXX) $< -o $@ $(STDLIB_CFLAGS) $(STDLIB_LFLAGS) 2> hybris.tmp.log || touch hybris.tmp.errors
	@$(CHECK_FOR_ERRORS)
	
src/parser.cpp: src/parser.y.cpp
	@printf "\x1b[33;01m@ Creating $@ ... "
	@bison -y -d -o $@ $? 2> /dev/null
	@$(CHECK_FOR_ERRORS)

src/lexer.cpp: src/lexer.l.cpp
	@printf "\x1b[33;01m@ Creating $@ ... "
	@flex -o$@ $? 2> /dev/null
	@$(CHECK_FOR_ERRORS)

clean:
	rm -f src/lexer.cpp include/lexer.h include/parser.h src/parser.hpp src/parser.cpp src/*.o src/*.lo src/types/*.o src/types/*.lo $(TARGET)
	rm -f lib$(TARGET).so.1.0
	rm -f lib$(TARGET).so
	rm -f lib$(TARGET).so.1
	rm -rf build
	rm -f hybris.tmp.*
	
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
	cp -rf build/* /usr/lib/hybris/library/
	cp -rf stdinc/* /usr/lib/hybris/include/

uninstall:
	rm -rf $(PREFIX)/bin/$(TARGET) 
	rm -rf $(PREFIX)/lib/$(TARGET)
	rm -rf $(PREFIX)/include/$(TARGET)
	rm -rf $(PREFIX)/lib/lib$(TARGET).so.1.0
	rm -rf $(PREFIX)/lib/lib$(TARGET).so
	rm -rf $(PREFIX)/lib/lib$(TARGET).so.1
