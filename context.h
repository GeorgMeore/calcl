#ifndef CONTEXT_INCLUDED
#define CONTEXT_INCLUDED

#include "object.h"
#include "gc.h"

typedef struct {
	GC     *gc;
	Object *root;
	Object *stack;
} Context;

#define Context_stack(ctx) ((ctx)->stack->as.stack)
#define Context_stack_push(ctx, v) (Stack_push(Context_stack(ctx), (v)))
#define Context_stack_pop(ctx) (Stack_pop(Context_stack(ctx)))

Context Context_make();
void    Context_destroy(Context self);

#endif // CONTEXT_INCLUDED
