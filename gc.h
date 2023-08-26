#ifndef GC_INCLUDED
#define GC_INCLUDED

#include "object.h"

typedef struct {
	Object *first;
	Object *last;
	int curr;
} GC;

GC     *GC_new();
void   GC_collect(GC *gc, Env *root);
Object *GC_alloc_env(GC *gc);
Object *GC_alloc_fn(GC *gc, Object *env, Node *body, char *arg);
Object *GC_alloc_number(GC *gc, double num);

#endif // GC_INCLUDED
