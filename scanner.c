#include "lex.h"

#include "iter.h"
#include "scanner.h"
#include "token.h"


Scanner Scanner_make(CharIterator *iterator)
{
	Scanner scanner;
	scanner.iterator = iterator;
	Token first = take_token(iterator);
	scanner.next = first;
	return scanner;
}

Token Scanner_next(Scanner *self)
{
	Token next = self->next;
	self->next = take_token(self->iterator);
	return next;
}

Token Scanner_peek(Scanner *self)
{
	return self->next;
}
