
CC = gcc
STND=-ansi
CFLAGS = $(STND) -pedantic -O3 -g -Wall -Werror -Wextra -Wformat=2 -Wshadow \
  -Wno-long-long -Wno-overlength-strings -Wno-format-nonliteral -Wcast-align \
  -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls \
  -Wnested-externs -Wmissing-include-dirs -Wswitch-default -std=c99

TESTS = $(wildcard tests/*.c)
EXAMPLES = $(wildcard examples/*.c)
EXAMPLESEXE = $(EXAMPLES:.c=)

.PHONY:parsing

all: $(EXAMPLESEXE) check 

parsing:
	gcc -std=c99 -Wall parsing02.c mpc.c -o parsing02  -lm  -lreadline

check: $(TESTS) mpc.c
	$(CC) $(filter-out -Werror, $(CFLAGS)) $^ -lm -o test
	./test

examples/%: examples/%.c mpc.c
	$(CC) $(CFLAGS) $^ -lm -o $@
clean_p:
	rm parsing02
clean:
	rm -rf test examples/doge examples/lispy examples/maths examples/smallc \
	examples/foobar examples/tree_traversal 
