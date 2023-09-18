#ifndef INFER_INCLUDED
#define INFER_INCLUDED

#include "node.h"
#include "types.h"
#include "context.h"

Type *infer(const Node *expr, Context *ctx);

#endif // INFER_INCLUDED
