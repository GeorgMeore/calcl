#ifndef CONTEXT_INCLUDED
#define CONTEXT_INCLUDED

#include "object.h"
#include "gc.h"

typedef struct {
	GC     *gc;
	Object *root;
	Object *stack;
} Context;

#define Context_stack(ctx) (ctx->stack->as.stack)

Context Context_make();
void    Context_destroy(Context self);

#endif // CONTEXT_INCLUDED
