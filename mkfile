CC=gcc
LD=ld
CFLAGS=-g -Wall -Wextra
SRC=lex.c main.c node.c iter.c parse.c scanner.c eval.c debug.c
prog=toyexpr

OBJ=${SRC:%.c=%.o}

$prog: $OBJ
	$CC $CFLAGS -o $target $prereq

%.o: %.c
	$CC $CFLAGS -c $prereq

clean:V:
	rm -f $OBJ $prog
