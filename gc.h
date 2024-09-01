#ifndef GC_INCLUDED
#define GC_INCLUDED

#include "object.h"
#include "node.h"

#define GC_INITIAL_THRESHOLD 128

typedef struct {
	Object   *first;
	Object   *last;
	int      curr;
	unsigned count;
	unsigned thres;
} GC;

typedef enum {
	PTR_ADDR,
	PTR_OBJ,
} PtrType;

GC     *GC_new(void);
void   GC_drop(GC *self);
void   GC_collect(GC *self, Object *root, Object *stack);
void   GC_collect_comp(GC *self, Object *root, void *rsp, void *rbp);
Object *GC_alloc_env(GC *self, Object *prev);
Object *GC_alloc_fn(GC *self, Object *env, const Node *body, const char *arg);
Object *GC_alloc_compfn(GC *self, Object *env, void *text);
Object *GC_alloc_number(GC *self, double num);
Object *GC_alloc_thunk(GC *self, Object *env, const Node *body);
Object *GC_alloc_stack(GC *self);
void   GC_dump_objects(GC *self);

#endif // GC_INCLUDED
