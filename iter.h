#ifndef ITER_INCLUDED
#define ITER_INCLUDED

#include <stdio.h>

#define INITIAL_BUFFER_SIZE 1024

typedef struct {
	FILE *file;
	char *buff;
	char *cursor;
	char *end;
	int  size;
} Iter;

Iter Iter_make(FILE *file);
void Iter_destroy(Iter iter);
void Iter_reset(Iter *iter);
char *Iter_cursor(const Iter *iter);
char Iter_peek(Iter *iter);
char Iter_next(Iter *iter);
int  Iter_eof(const Iter *iter);

#endif // ITER_INCLUDED
