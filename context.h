#ifndef CONTEXT_INCLUDED
#define CONTEXT_INCLUDED

#include "object.h"
#include "gc.h"
#include "types.h"

typedef struct {
	GC      *gc;
	Object  *root;
	Object  *stack;
	TypeEnv *tenv;
} Context;

#define Context_stack(ctx) (StackObj_stack((ctx)->stack))
#define Context_stack_push(ctx, v) (Stack_push(Context_stack(ctx), (v)))
#define Context_stack_pin(ctx, v) (Stack_pin(Context_stack(ctx), (v)))
#define Context_stack_pop(ctx) (Stack_pop(Context_stack(ctx)))

Context Context_make(void);
void    Context_destroy(Context self);

#endif // CONTEXT_INCLUDED
