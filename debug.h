#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include "token.h"
#include "node.h"

void print_token(Token token);
void print_expr(const Node *expr);

#endif // DEBUG_INCLUDED
