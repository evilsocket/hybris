CXX=g++
WFLAGS= -w
OPTIMIZATION= -O3 -pipe -fomit-frame-pointer -ffast-math
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

NO_COLOR=\x1b[0m
OK_COLOR=\x1b[32;01m
ERROR_COLOR=\x1b[31;01m
WARN_COLOR=\x1b[33;01m
OK_STRING=$(OK_COLOR) OK\n$(NO_COLOR)
ERROR_STRING=$(ERROR_COLOR)[ERRORS]\n$(NO_COLOR)
WARN_STRING=$(WARN_COLOR)[WARNINGS]\n$(NO_COLOR)

LIBOBJ=src/context.lo \
       src/gc.lo \
	   src/common.lo \
	   src/engine.lo \
	   src/node.lo \
	   src/types.lo \
	   src/binary.lo \
	   src/char.lo \
	   src/float.lo \
	   src/integer.lo \
	   src/map.lo \
	   src/matrix.lo \
	   src/string.lo \
	   src/structure.lo \
	   src/vector.lo \
	   src/hashtable.lo \
	   src/vmem.lo \
	   src/vcode.lo \
	   src/parser.lo \
	   src/lexer.lo

SOURCES=src/common.cpp \
		src/gc.cpp \
		src/lexer.cpp \
		src/context.cpp \
		src/hashtable.cpp \
		src/vmem.cpp \
		src/vcode.cpp \
		src/node.cpp \
		src/types.cpp \
		src/binary.cpp \
		src/char.cpp \
		src/float.cpp \
		src/integer.cpp \
		src/map.cpp \
		src/matrix.cpp \
		src/string.cpp \
		src/structure.cpp \
		src/vector.cpp \
	    src/engine.cpp \
	    src/parser.cpp

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
	   stdlib/std/pcre.cc \
	   stdlib/std/math.cc
		
OBJECTS=$(SOURCES:.cpp=.o)
LOBJECTS=$(SOURCES:.cpp=.lo)
STDOBJ=$(patsubst stdlib%.cc,build%.so, $(STDSRC))

all: src/lexer.cpp src/parser.cpp lib hybris stdlib

lib: $(LOBJECTS)	
	@rm -f hybris.tmp.errors hybris.tmp.log
	@printf "\x1b[33;01m@ Creating lib$(TARGET).so.1.0 ..."
	@$(CXX) -shared -Wl,-soname,libhybris.so.1 -o lib$(TARGET).so.1.0 $(LIBOBJ) 2> hybris.tmp.log || touch hybris.tmp.errors
	@if test -e hybris.tmp.errors; \
		then printf "$(ERROR_STRING)" && cat hybris.tmp.log && exit 1; \
	elif test -s hybris.tmp.log; \
		then printf "$(WARN_STRING)" && cat hybris.tmp.log; \
	else printf "$(OK_STRING)"; \
	fi
	@rm -f hybris.tmp.errors hybris.tmp.log
	@ln -sf lib$(TARGET).so.1.0 lib$(TARGET).so
	@ln -sf lib$(TARGET).so.1.0 lib$(TARGET).so.1
	
hybris: $(OBJECTS)
	@rm -f hybris.tmp.errors hybris.tmp.log
	@printf "\x1b[33;01m@ Creating $(TARGET) ..."
	@$(CXX) src/*.o -o $(TARGET) $(CFLAGS) $(LFLAGS) 2> hybris.tmp.log || touch hybris.tmp.errors
	@if test -e hybris.tmp.errors; \
		then printf "$(ERROR_STRING)" && cat hybris.tmp.log && exit 1; \
	elif test -s hybris.tmp.log; \
		then printf "$(WARN_STRING)" && cat hybris.tmp.log; \
	else printf "$(OK_STRING)"; \
	fi
	@rm -f hybris.tmp.errors hybris.tmp.log

stdlib: $(STDOBJ)
		
.cpp.o: 
	@rm -f hybris.tmp.* && printf "@ Compiling $< ..."
	@$(CXX) -c $< -o $@ $(CFLAGS) 2> hybris.tmp.log || touch hybris.tmp.errors
	@if test -e hybris.tmp.errors; \
		then printf "$(ERROR_STRING)" && cat hybris.tmp.log && exit 1; \
	elif test -s hybris.tmp.log; \
		then printf "$(WARN_STRING)" && cat hybris.tmp.log; \
	else printf "$(OK_STRING)"; \
	fi
	@rm -f hybris.tmp.errors hybris.tmp.log
	
.cpp.lo:
	@rm -f hybris.tmp.* && printf "@ Compiling $< ..."
	@$(CXX) -c $< -o $@ $(CFLAGS) -fPIC 2> hybris.tmp.log || touch hybris.tmp.errors
	@if test -e hybris.tmp.errors; \
		then printf "$(ERROR_STRING)" && cat hybris.tmp.log && exit 1; \
	elif test -s hybris.tmp.log; \
		then printf "$(WARN_STRING)" && cat hybris.tmp.log; \
	else printf "$(OK_STRING)"; \
	fi
	@rm -f hybris.tmp.errors hybris.tmp.log

build/%.so: stdlib/%.cc
	@rm -f hybris.tmp.* && printf "@ Compiling $< ..."
	@mkdir -p $(dir $@)
	@$(CXX) $< -o $@ $(STDLIB_CFLAGS) $(STDLIB_LFLAGS) 2> hybris.tmp.log || touch hybris.tmp.errors
	@if test -e hybris.tmp.errors; \
		then printf "$(ERROR_STRING)" && cat hybris.tmp.log && exit 1; \
	elif test -s hybris.tmp.log; \
		then printf "$(WARN_STRING)" && cat hybris.tmp.log; \
	else printf "$(OK_STRING)"; \
	fi
	@rm -f hybris.tmp.errors hybris.tmp.log
	
src/parser.cpp: src/parser.y.cpp
	@printf "\x1b[33;01m@ Creating $@ ... "
	@bison -y -d -o $@ $? 2> /dev/null
	@printf "$(OK_STRING)";

src/lexer.cpp: src/lexer.l.cpp
	@printf "\x1b[33;01m@ Creating $@ ... "
	@flex -o $@ $? 2> /dev/null
	@printf "$(OK_STRING)";

clean:
	rm -f src/lexer.cpp include/lexer.h include/parser.h src/parser.hpp src/parser.cpp src/*.o src/*.lo $(TARGET)
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

uninstall:
	rm -rf $(PREFIX)/bin/$(TARGET) 
	rm -rf $(PREFIX)/lib/$(TARGET)
	rm -rf $(PREFIX)/include/$(TARGET)
	rm -rf $(PREFIX)/lib/lib$(TARGET).so.1.0
	rm -rf $(PREFIX)/lib/lib$(TARGET).so
	rm -rf $(PREFIX)/lib/lib$(TARGET).so.1
