SRC = ./src/
PREFIX = /usr/local/
CC = gcc
CFLAGS = -c -fpic -Os 

all: libheap.so

freelist.o: $(SRC)freelist.c
	$(CC) $(CFLAGS)  $<
libheap.o: $(SRC)libheap.c
	$(CC) $(CFLAGS) $<
pool.o: $(SRC)pool.c
	$(CC) $(CFLAGS) $<
libheap.so: freelist.o libheap.o pool.o
	$(CC) --shared freelist.o libheap.o pool.o -o $@
install: libheap.so
	cp $< $(PREFIX)lib
	cp include/heap.h $(PREFIX)include
	sudo cp libheap.so /usr/local/lib
	sudo ldconfig

clean:
	rm -rf libheap.so
	rm -rf *.o