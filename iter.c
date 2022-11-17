#include "iter.h"


const char *CharIterator_cursor(CharIterator *self)
{
	return *self;
}

char CharIterator_next(CharIterator *self)
{
	char next = **self;
	*self += 1;
	return next;
}

char CharIterator_peek(CharIterator *self)
{
	return **self;
}
