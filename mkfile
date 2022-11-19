CC=gcc
LD=ld
CFLAGS=-g -lm -Wall -Wextra
SRC=debug.c eval.c input.c iter.c lex.c main.c node.c opts.c parse.c scanner.c

prog=toycalc

OBJ=${SRC:%.c=%.o}

$prog: $OBJ
	$CC $CFLAGS -o $target $prereq

%.o: %.c
	$CC $CFLAGS -c $stem.c

# generate dependency list
<|$CC -MM $SRC

clean:V:
	rm -f $OBJ $prog
