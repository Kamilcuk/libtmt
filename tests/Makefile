#!/usr/bin/make

define \n


endef

CC=gcc
CFLAGS=-Wall -I. -I.. -g3 -O0 -std=c99
LDFLAGS=-lcheck
mains = $(patsubst %-check.c, %-check.out, $(wildcard ./*-check.c)) main.out 

ifdef NDEBUG
CFLAGS+=-g0
endif
ifdef OPTIMIZE
CFLAGS+=-O3
endif

.PHONY: test 

all: $(mains)

-include $(wildcard *.d) $(wildcard ../*.d)

%.d: %.c %.h
	$(CC) $(CFLAGS) -MM $*.c > $*.d

%.o: %.c %.h %.d
	$(CC) $(CFLAGS) -c -o $*.o $*.c

%.out: callback.o %.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^
	
main.out: callback.o main.o ../tmt.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o *.d ../*.o ../*.d
	rm -f *.out 

test: $(mains)
	$(foreach file, $?, ./$(file)${\n})

