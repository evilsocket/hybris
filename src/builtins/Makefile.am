CC=g++
WFLAGS= -w
OPTIMIZATION= -O3
LIBXML_CFLAGS= `xml2-config --cflags`
LIBXML_LFLAGS= `xml2-config --libs`
CFLAGS= -I../../include/ $(OPTIMIZATION) $(WFLAGS) $(LIBXML_CFLAGS) -funroll-loops -ffast-math -fno-stack-protector
LFLAGS= -ldl -lpcrecpp -lcurl -lpthread $(LIBXML_LFLAGS)
SOURCES=array.cc dll.cc fileio.cc math.cc netio.cc process.cc reflection.cc time.cc xml.cc \
		conio.cc encoding.cc http.cc map.cc matrix.cc pcre.cc pthreads.cc string.cc type.cc
OBJECTS=$(SOURCES:.cc=.o)

all: dll.o $(SOURCES) $(OBJECTS)
# dllcall could not be optimized due to stack preservation issue
dll.o: dll.cc
	$(CC) -c -I../../include/ $(LIBXML_CFLAGS) $? -o $@

.cc.o:
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o
