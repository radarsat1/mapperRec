
CC=gcc
CFLAGS=-O0 -g -Wall -Werror $(shell pkg-config --cflags libmapper-0)
LDLIBS=$(shell pkg-config --libs libmapper-0)

LIBEXT=$(suffix $(wildcard /usr/lib/libc.so)     \
                $(wildcard /usr/lib/libc.dylib))

ifeq ($(LIBEXT),.dylib)
CFLAGS+=-arch i386
endif

LIBOBJS=recmonitor.o recdevice.o command.o backend.o \
        backend_text.o backend_binary.o backend_oscstreamdb.o

all: mapperRec libmapperrec$(LIBEXT)

mapperRec: mapperRec.o libmapperrec.a
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

libmapperrec$(LIBEXT): $(LIBOBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(LDFLAGS) $^ $(LDLIBS)

libmapperrec.a: $(LIBOBJS)
	$(AR) -ruv $@ $^

.PHONY: clean all

clean:
	rm -rvf $(LIBOBJS) mapperRec.o mapperRec \
            libmapperrec.a libmapperrec$(LIBEXT)
