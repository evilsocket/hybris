CC=g++
WFLAGS= -w
OPTIMIZATION= -O3 -funroll-loops -fno-rtti -fomit-frame-pointer -ffast-math -fno-stack-protector -ffunction-sections
LIBXML_CFLAGS= `xml2-config --cflags`
LIBXML_LFLAGS= `xml2-config --libs`
LIBFFI_CFLAGS= `pkg-config libffi --cflags`
LIBFFI_LFLAGS= `pkg-config libffi --libs`
LFLAGS= -ldl -lpcrecpp -lcurl -lpthread $(LIBXML_LFLAGS) $(LIBFFI_LFLAGS) 
CFLAGS= $(WFLAGS) -I../include -L../ $(OPTIMIZATION) -fPIC $(LIBXML_CFLAGS) $(LIBFFI_CFLAGS)  -lhybris -lc -shared

SOURCES=std/io/network/http.cc \
		std/io/network/tcp.cc \
		std/io/file.cc \
		std/io/console.cc \
		std/io/xml.cc \
		std/type/reflection.cc \
		std/type/string.cc \
		std/type/matrix.cc \
		std/type/array.cc \
		std/type/map.cc \
		std/type/type.cc \
		std/type/binary.cc \
		std/encoding.cc \
		std/os/dll.cc \
		std/os/time.cc \
		std/os/threads.cc \
		std/os/process.cc \
		std/pcre.cc \
		std/math.cc

OBJECTS=$(SOURCES:.cc=.so)

all: $(OBJECTS)

.cc.so: 
	mkdir -p build/`dirname $<`
	g++ $< -o build/`dirname $<`/`basename $< .cc`.so $(CFLAGS) $(LFLAGS)

install:
	cp -rf build/* /usr/lib/hybris/library/

clean:
	rm -rf build
