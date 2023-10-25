CC=gcc
CFLAGS=-I. -std=gnu11 -Wall
CFLAGS+=-O2 -march=native -fomit-frame-pointer

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

iis3dwb-fifo2xyzt: iis3dwb-fifo2xyzt.o
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf iis3dwb-fifo2xyzt *.o 
