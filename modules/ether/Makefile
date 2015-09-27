CC?=gcc
CFLAGS:=-fPIC -I../../include $(CFLAGS)

TGT?=x86_64
OBJECTS=_$(TGT)_obs/ether.o
default: all
BINARY=../../plugins/ether.so

HACK := $(shell mkdir -p _$(TGT)_obs/)

_$(TGT)_obs/ether.o: ether.c
	$(CC) $(CFLAGS) -o $@ -c ether.c 

all: $(OBJECTS)
	$(CC) -o $(BINARY) -rdynamic -shared $(OBJECTS)

clean:
	rm -rf _$(TGT)_obs $(BINARY)
