#ifndef PARSE_INCLUDED
#define PARSE_INCLUDED

#include "scanner.h"
#include "node.h"

Node *parse(Scanner *scanner, Arena *a);

#endif // PARSE_INCLUDED
