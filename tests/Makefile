
CC=gcc
CFLAGS=-Wall -I. -I.. -g3 -O3 -std=c99
LDFLAGS=-lcheck
mains = $(patsubst %-check.c, %-check.out, $(wildcard ./*-check.c)) main.out 
define \n


endef

.PHONY: test 
all: $(mains)

%-check.c: %.check.c
	checkmk $< > $@

%.out: callback.o %.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^
	
main.out: callback.o main.o ../tmt.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

test: $(mains)
	$(foreach file, $?, ./$(file)${\n})

distclean:
	rm -f *.o ../tmt.o $(mains) 
