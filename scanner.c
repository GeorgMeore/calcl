#include "scanner.h"
#include "token.h"

Token Scanner_next(Scanner *self)
{
	Token next = **self;
	*self += 1;
	return next;
}

Token Scanner_peek(Scanner *self)
{
	return **self;
}
