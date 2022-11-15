#ifndef SCANNER_INCLUDED
#define SCANNER_INCLUDED

#include "token.h"

typedef const Token *Scanner;

Token Scanner_peek(Scanner *self);
Token Scanner_next(Scanner *self);

#endif // SCANNER_INCLUDED
