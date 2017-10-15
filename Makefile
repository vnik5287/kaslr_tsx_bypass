libkaslr.a: util.o kaslr.o
	ar rcs libkaslr.a util.o kaslr.o
	ranlib libkaslr.a

kaslr.o: kaslr.c
	gcc -D_DEBUG -c kaslr.c

util.o: util.c
	gcc -D_DEBUG -c util.c

clean:
	rm -fr *.o libkaslr.a example

example: libkaslr.a example.c
	gcc example.c -o example -static -L. -lpthread -lkaslr -lpthread -lm
