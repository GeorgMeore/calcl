#ifndef CODEGEN_INCLUDED
#define CODEGEN_INCLUDED

#include "node.h"

void compile(const Node *expr);
void compile_begin(void);
void compile_end(void);

#endif // CODEGEN_INCLUDED
