#ifndef SCANNER_INCLUDED
#define SCANNER_INCLUDED

#include "token.h"
#include "iter.h"

typedef struct {
	Iter  iterator;
	Token next;
} Scanner;

Scanner Scanner_make(FILE *file);
void    Scanner_destroy(Scanner scanner);
void    Scanner_start(Scanner *scanner);
Token   Scanner_peek(Scanner *scanner);
Token   Scanner_next(Scanner *scanner);
void    Scanner_seek(Scanner *scanner, TokenType type);
#define Scanner_eof(scanner) (Iter_eof(&scanner.iterator))

#endif // SCANNER_INCLUDED
