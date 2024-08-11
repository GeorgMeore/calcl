#ifndef ARENA_INCLUDED
#define ARENA_INCLUDED

#include <stddef.h>


typedef struct Page Page;

typedef struct {
	Page   *first;
	size_t page_size;
} Arena;

Arena Arena_make(size_t page_size);
void  *Arena_alloc(Arena *self, size_t bytes);
void  Arena_reset(Arena *self);
void  Arena_destroy(Arena self);

#endif // ARENA_INCLUDED
