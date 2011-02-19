
CC=gcc
CFLAGS=-O0 -g -Wall $(shell pkg-config --cflags libmapper-0)
LDLIBS=$(shell pkg-config --libs libmapper-0)

mapperRec: mapperRec.o backend_oscstreamdb.o command.o
