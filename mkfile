CC=gcc
CFLAGS=-g -Wall -Wextra # -fsanitize=address,undefined
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

interp: interp.c $OBJ
	$CC $CFLAGS $LDFLAGS -o $target $prereq

comp: comp.c $OBJ
	$CC $CFLAGS $LDFLAGS -o $target $prereq

test.s: comp test.calcl
	./comp >test.s <test.calcl

test: test.s $OBJ
	$CC $CFLAGS $LDFLAGS -o $target $prereq

%.o: %.c mkfile
	$CC $CFLAGS -c $stem.c

# generate dependency list
<|$CC -MM $SRC

clean:V:
	rm -f $OBJ interp comp
