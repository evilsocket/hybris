CC=g++
WFLAGS= -w
OPTIMIZATION= -O3
LIBXML_CFLAGS= `xml2-config --cflags`
LIBXML_LFLAGS= `xml2-config --libs`
LIBFFI_CFLAGS= `pkg-config libffi --cflags`
LIBFFI_LFLAGS= `pkg-config libffi --libs`
CFLAGS= -I../../include/ $(OPTIMIZATION) $(WFLAGS) $(LIBXML_CFLAGS) $(LIBFFI_CFLAGS)  -funroll-loops -fno-rtti -fomit-frame-pointer -ffast-math -fno-stack-protector -ffunction-sections
LFLAGS= -ldl -lpcrecpp -lcurl -lpthread $(LIBXML_LFLAGS) $(LIBFFI_LFLAGS) 
SOURCES=array.cc dll.cc fileio.cc math.cc netio.cc process.cc reflection.cc time.cc xml.cc \
		conio.cc encoding.cc http.cc map.cc matrix.cc pcre.cc pthreads.cc string.cc type.cc
OBJECTS=$(SOURCES:.cc=.o)
LOBJECTS=$(SOURCES:.cc=.lo)

all: dll.o $(SOURCES) $(OBJECTS)

lib: dll.lo $(SOURCES) $(LOBJECTS)

# dllcall could not be optimized due to stack preservation issue
dll.o: dll.cc
	$(CC) -c -I../../include/ $(LIBXML_CFLAGS) $(LIBFFI_CFLAGS)  $? -o $@

dll.lo: dll.cc
	$(CC) -c -I../../include/ -fPIC $(LIBXML_CFLAGS) $(LIBFFI_CFLAGS)  $? -o $@

.cc.o:
	$(CC) -c $< -o $@ $(CFLAGS)

.cc.lo:
	$(CC) -c $< -o $@ $(CFLAGS) -fPIC
	
clean:
	rm -f *.o *.lo
