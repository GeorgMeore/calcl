#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "node.h"
#include "object.h"
#include "context.h"

Object *eval(Node *expr, Context *ctx);

#endif // EVAL_INCLUDED
