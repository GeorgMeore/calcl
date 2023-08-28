#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "node.h"
#include "object.h"
#include "gc.h"

// strict evaluation
Object *seval(Node *expr, GC *gc, Object *env);

// lazy evaluation
Object *leval(Node *expr, GC *gc, Object *env);

#endif // EVAL_INCLUDED
