CC=gcc
CFLAGS=-g -Wall -Wextra
LDFLAGS=-lm
SRC=eval.c\
	iter.c\
	lex.c\
	token.c\
	node.c\
	opts.c\
	parse.c\
	scanner.c\
	gc.c\
	object.c\
	stack.c\
	context.c\
	infer.c\
	types.c\
	env.c\
	main.c

prog=calcl

OBJ=${SRC:%.c=%.o}

$prog: $OBJ
	$CC $LDFLAGS -o $target $prereq

%.o: %.c
	$CC $CFLAGS -c $stem.c

# generate dependency list
<|$CC -MM $SRC

clean:V:
	rm -f $OBJ $prog
