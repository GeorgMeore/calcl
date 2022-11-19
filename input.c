#include "input.h"

#include <stdio.h>
#include <stdlib.h>


#define INITIAL_BUFFER_SIZE 1024

// TODO: intergrate input into CharIterator
char *get_line()
{
	static char *buffer;
	static int size;
	if (!buffer) {
		buffer = malloc(INITIAL_BUFFER_SIZE);
		size = INITIAL_BUFFER_SIZE;
	}
	for (int end = 0;; end++) {
		int c = getchar();
		// if found eof while the buffer is not empty
		// return the buffer first
		if (c == '\n' || (c == EOF && end > 0)) {
			buffer[end] = '\0';
			return buffer;
		}
		if (c == EOF) {
			free(buffer);
			return NULL;
		}
		// make buffer larger as needed
		if (end == size) {
			size *= 2;
			buffer = realloc(buffer, size);
		}
		buffer[end] = c;
	}
}
