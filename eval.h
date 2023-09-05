#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include "node.h"
#include "object.h"
#include "gc.h"

typedef struct {
	GC     *gc;
	Object *root;
	Object *stack;
} Context;

Context Context_make();
void    Context_destroy(Context self);

// strict evaluation
Object *seval(Node *expr, Context *ctx);

// lazy evaluation
Object *leval(Node *expr, GC *gc, Object *env);

#endif // EVAL_INCLUDED
