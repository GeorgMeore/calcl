#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "node.h"
#include "object.h"
#include "context.h"

// NOTE: expr ownership is transferred here
Object *eval(Node *expr, Context *ctx);

#endif // EVAL_INCLUDED
