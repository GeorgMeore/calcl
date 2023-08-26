#ifndef GC_INCLUDED
#define GC_INCLUDED

#include "object.h"

typedef struct {
	Object *first;
	Object *last;
	int curr;
} GC;

GC     *GC_new();
void   GC_drop(GC *self);
void   GC_collect(GC *self, Object *root);
Object *GC_alloc_env(GC *self, Object *prev);
Object *GC_alloc_fn(GC *self, Object *env, Node *body, char *arg);
Object *GC_alloc_number(GC *self, double num);

#endif // GC_INCLUDED
