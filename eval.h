#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "node.h"
#include "object.h"
#include "context.h"
#include "annotations.h"

Object *eval(passed Node *expr, Context *ctx);

#endif // EVAL_INCLUDED
