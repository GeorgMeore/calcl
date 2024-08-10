#ifndef INFER_INCLUDED
#define INFER_INCLUDED

#include "node.h"
#include "types.h"
#include "context.h"
#include "arena.h"

Type *infer(const Node *expr, Context *ctx, Arena *a);

#endif // INFER_INCLUDED
