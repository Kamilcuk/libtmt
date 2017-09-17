
CC=gcc
CFLAGS=-Wall -I. -I.. -O3 -g3
LDFLAGS=-lcheck
mains = main.out $(patsubst %.check.c, %-check.out, $(wildcard ./*.check.c))
define \n


endef

.PHONY: test 
all: $(mains)

%-check.c: %.check.c
	checkmk $< > $@

%.out: ../tmt.o callback.o %.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

test: $(mains)
	$(foreach file, $?, ./$(file)${\n})

distclean:
	rm -f $(mains) 
