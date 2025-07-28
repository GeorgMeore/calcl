CC=gcc
CFLAGS=-g -Wall -Wextra -std=gnu99 # -fsanitize=address,undefined
LDFLAGS=-lm
SRC=eval.c\
	codegen.c\
	iter.c\
	lex.c\
	token.c\
	node.c\
	opts.c\
	parse.c\
	scanner.c\
	gc.c\
	arena.c\
	object.c\
	stack.c\
	context.c\
	infer.c\
	types.c\
	env.c

OBJ=${SRC:%.c=%.o}

all:V: interp comp runtime.o

interp: interp.c $OBJ
	$CC $CFLAGS $LDFLAGS -o $target $prereq

# TODO: link in only the necessary stuff
runtime.o: $OBJ
	ld -i -o $target $prereq

comp: comp.c $OBJ
	$CC $CFLAGS $LDFLAGS -o $target $prereq

%.o: %.c mkfile
	$CC $CFLAGS -c $stem.c

# generate dependency list
<|$CC -MM $SRC

clean:V:
	rm -f $OBJ interp comp runtime.o
