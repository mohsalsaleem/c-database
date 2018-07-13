HEADERS = input_buffer.h

default: salite

salite.o: main.c $(HEADERS)
	gcc -c main.c -o salite.o

salite: salite.o
	gcc salite.o -o salite

clean:
	-rm -f salite.o
	-rm -f salite