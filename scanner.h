#ifndef SCANNER_INCLUDED
#define SCANNER_INCLUDED

#include "token.h"
#include "iter.h"

typedef struct {
	CharIterator *iterator;
	Token next;
} Scanner;

Scanner Scanner_make(CharIterator *iterator);
Token   Scanner_peek(Scanner *self);
Token   Scanner_next(Scanner *self);

#endif // SCANNER_INCLUDED
