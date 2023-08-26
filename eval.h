#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "node.h"
#include "object.h"
#include "gc.h"

Object *eval(Node *expr, GC *gc, Object *env);

#endif // EVAL_INCLUDED
