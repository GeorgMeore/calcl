#ifndef ARENA_INCLUDED
#define ARENA_INCLUDED

#include <stddef.h>


typedef struct Arena Arena;

Arena *Arena_new(size_t page_size);
void  *Arena_alloc(Arena *self, size_t bytes);
void   Arena_reset(Arena *self);
void   Arena_drop(Arena *self);


#endif // ARENA_INCLUDED
