#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include "token.h"
#include "node.h"

void Token_print(Token token);
void Node_print(const Node *expr);

#endif // DEBUG_INCLUDED
