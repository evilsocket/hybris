all:
	gcc -w -c -fpic libtest.cc
	gcc -shared -lc -o libtest.so libtest.o
	
clean:
	rm -f libtest.o libtest.so
