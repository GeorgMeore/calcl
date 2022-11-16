#ifndef ITER_INCLUDED
#define ITER_INCLUDED

typedef const char *CharIterator;

const char* CharIterator_cursor(CharIterator *self);
char CharIterator_peek(CharIterator *self);
char CharIterator_next(CharIterator *self);

#endif // ITER_INCLUDED
