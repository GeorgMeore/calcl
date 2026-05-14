#!/bin/sh -ex

CFLAGS='-g -Wall -Wextra -std=gnu99' # -fsanitize=address,undefined'

cc $CFLAGS -c -o runtime.o runtime.c &
cc $CFLAGS -c -o common.o common.c &
wait
cc $CFLAGS -o interp -lm context.c eval.c interp.c runtime.o common.o &
cc $CFLAGS -o comp codegen.c comp.c runtime.o common.o &
wait
