#include "gc.h"

#include <stdlib.h>

#include "node.h"
#include "object.h"
#include "values.h"
#include "env.h"
#include "stack.h"


GC *GC_new(void)
{
	GC *self = malloc(sizeof(*self));
	self->first = NULL;
	self->last = NULL;
	self->curr = 0;
	self->count = 0;
	self->thres = GC_INITIAL_THRESHOLD;
	return self;
}

void GC_drop(GC *self)
{
	free(self);
}

static void GC_mark(GC *self, Object *obj)
{
	if (obj->mark == self->curr) {
		return;
	}
	obj->mark = self->curr;
	switch (obj->type) {
		case NumObject:
			return;
		case FnObject:
			return GC_mark(self, FnObj_env(obj));
		case CompfnObject:
			return GC_mark(self, CompFnObj_env(obj));
		case ThunkObject:
			if (ThunkObj_value(obj)) {
				return GC_mark(self, ThunkObj_value(obj));
			} else {
				return GC_mark(self, ThunkObj_env(obj));
			}
		case CompthunkObject:
			if (CompThunkObj_value(obj)) {
				return GC_mark(self, CompThunkObj_value(obj));
			} else {
				return GC_mark(self, CompThunkObj_env(obj));
			}
		case EnvObject:
			if (EnvObj_prev(obj)) {
				GC_mark(self, EnvObj_prev(obj));
			}
			return Env_for_each(EnvObj_env(obj), (void (*)(void *, Object*))GC_mark, self);
		case StackObject:
			return Stack_for_each(StackObj_stack(obj), (void (*)(void *, Object*))GC_mark, self);
	}
}

static void GC_append_object(GC *self, Object *obj)
{
	self->count += 1;
	obj->next = NULL;
	if (!self->first) {
		self->first = obj;
		self->last = obj;
	} else {
		self->last->next = obj;
		self->last = obj;
	}
}

static void GC_reset(GC *self)
{
	self->first = NULL;
	self->last = NULL;
	self->count = 0;
}

static void GC_free_object(Object *obj)
{
	switch (obj->type) {
		case NumObject:
			return free(ObjToVal(obj, Num));
		case FnObject:
			return free(ObjToVal(obj, Fn));
		case CompfnObject:
			return free(ObjToVal(obj, CompFn));
		case ThunkObject:
			return free(ObjToVal(obj, Thunk));
		case CompthunkObject:
			return free(ObjToVal(obj, CompThunk));
		case EnvObject:
			return Env_drop(EnvObj_env(obj));
		case StackObject:
			return Stack_drop(StackObj_stack(obj));
	}
}

static void GC_sweep(GC *self)
{
	Object *obj = self->first;
	GC_reset(self);
	while (obj != NULL) {
		Object *next = obj->next;
		if (obj->mark != self->curr) {
			GC_free_object(obj);
		} else {
			GC_append_object(self, obj);
		}
		obj = next;
	}
}

void GC_collect(GC *self, Object *root, Object *stack)
{
	if (self->count < self->thres && (root || stack)) {
		self->thres >>= (self->count < self->thres/2);
		return;
	}
	self->curr = !self->curr;
	if (root) {
		GC_mark(self, root);
	}
	if (stack) {
		GC_mark(self, stack);
	}
	GC_sweep(self);
	if (self->count >= self->thres) {
		self->thres <<= 1;
	}
}

void GC_collect_comp(GC *self, Object *root, void *rsp, void *rbp)
{
	if (self->count < self->thres && root) {
		self->thres >>= (self->count < self->thres/2);
		return;
	}
	self->curr = !self->curr;
	if (root) {
		GC_mark(self, root);
	}
	for (size_t *v = rsp; v < (size_t *)rbp; v += 2) {
		if (v[0] == PTR_OBJ) {
			GC_mark(self, (Object *)v[1]);
		}
	}
	GC_sweep(self);
	if (self->count >= self->thres) {
		self->thres <<= 1;
	}
}

#define GC_init_object(self, val, otype) ({\
	Object *obj = ValToObj(val);\
	obj->mark = self->curr;\
	obj->type = otype;\
	GC_append_object(self, obj);\
	obj;\
})

Object *GC_alloc_env(GC *self, Object *prev)
{
	return GC_init_object(self, Env_new(prev), EnvObject);
}

Object *GC_alloc_fn(GC *self, Object *env, const Node *body, const char *arg)
{
	Fn *fn = malloc(sizeof(*fn));
	fn->env = env;
	fn->body = body;
	fn->arg = arg;
	return GC_init_object(self, fn, FnObject);
}

Object *GC_alloc_compfn(GC *self, Object *env, void *text)
{
	CompFn *cfn = malloc(sizeof(*cfn));
	cfn->env = env;
	cfn->text = text;
	return GC_init_object(self, cfn, CompfnObject);
}

Object *GC_alloc_number(GC *self, double num)
{
	Num *n = malloc(sizeof(*n));
	n->num = num;
	return GC_init_object(self, n, NumObject);
}

Object *GC_alloc_thunk(GC *self, Object *env, const Node *body)
{
	Thunk *th = malloc(sizeof(*th));
	th->env = env;
	th->body = body;
	th->value = NULL;
	return GC_init_object(self, th, ThunkObject);
}

Object *GC_alloc_compthunk(GC *self, Object *env, void *text)
{
	CompThunk *cth = malloc(sizeof(*cth));
	cth->env = env;
	cth->text = text;
	cth->value = NULL;
	return GC_init_object(self, cth, CompthunkObject);
}

Object *GC_alloc_stack(GC *self)
{
	return GC_init_object(self, Stack_new(), StackObject);
}

void GC_dump_objects(GC *self)
{
	for (Object *obj = self->first; obj != NULL; obj = obj->next) {
		Object_println(obj);
	}
}
