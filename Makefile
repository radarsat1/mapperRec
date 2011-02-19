
CC=gcc
CFLAGS=-O0 -g -Wall -Werror $(shell pkg-config --cflags libmapper-0)
LDLIBS=$(shell pkg-config --libs libmapper-0)

mapperRec: mapperRec.o backend_oscstreamdb.o command.o recmonitor.o	\
           recdevice.o

.PHONY: clean
clean:
	rm -rvf *.o mapperRec
