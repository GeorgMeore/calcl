#include "iter.h"

#include <stdlib.h>

#include "annotations.h"


Iter Iter_make(passed FILE *file)
{
	Iter iter = {0};
	iter.file = file;
	iter.buff = malloc(INITIAL_BUFFER_SIZE);
	iter.end = iter.buff;
	iter.cursor = iter.buff;
	iter.size = INITIAL_BUFFER_SIZE;
	return iter;
}

void Iter_destroy(Iter iter)
{
	free(iter.buff);
	fclose(iter.file);
}

void Iter_reset(Iter *iter)
{
	iter->end = iter->buff;
	iter->cursor = iter->buff;
}

char *Iter_cursor(const Iter *iter)
{
	return iter->cursor;
}

static void Iter_getc(Iter *iter, int count)
{
	while (!Iter_eof(iter) && count > 0) {
		if (iter->end - iter->buff == iter->size - 1) {
			iter->size *= 2;
			iter->buff = realloc(iter->buff, iter->size);
		}
		int c = fgetc(iter->file);
		iter->end[0] = c == EOF ? '\0' : c;
		iter->end += 1;
		count -= 1;
	}
}

char Iter_peek(Iter *iter)
{
	Iter_getc(iter, 1 - (iter->end - iter->cursor));
	if (iter->cursor >= iter->end) {
		return '\0';
	}
	return iter->cursor[0];
}

char Iter_next(Iter *iter)
{
	char c = Iter_peek(iter);
	iter->cursor += 1;
	return c;
}

int Iter_eof(const Iter *iter)
{
	return feof(iter->file) || ferror(iter->file);
}
