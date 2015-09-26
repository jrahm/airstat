CC?=gcc
CFLAGS:=-fPIC -I../../include $(CFLAGS)

TGT?=x86_64
OBJECTS=_$(TGT)_obs/test.o
default: all
BINARY=../../plugins/test.so

HACK := $(shell mkdir -p _$(TGT)_obs/)

_$(TGT)_obs/test.o: test.c
	$(CC) $(CFLAGS) -o $@ -c test.c 

all: $(OBJECTS)
	$(CC) -o $(BINARY) -rdynamic -shared $(OBJECTS)

clean:
	rm -rf _$(TGT)_obs $(BINARY)
