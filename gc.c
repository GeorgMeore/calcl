#include "gc.h"

#include <stdlib.h>
#include <string.h>

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
	if (obj->type == FN_OBJECT) {
		GC_mark(self, FnObj_env(obj));
	} else if (obj->type == THUNK_OBJECT) {
		if (ThunkObj_value(obj)) {
			GC_mark(self, ThunkObj_value(obj));
		} else if (ThunkObj_env(obj)) {
			GC_mark(self, ThunkObj_env(obj));
		}
	} else if (obj->type == ENV_OBJECT) {
		Env_for_each(EnvObj_env(obj), (void (*)(void *, Object*))GC_mark, self);
		if (EnvObj_prev(obj)) {
			GC_mark(self, EnvObj_prev(obj));
		}
	} else if (obj->type == STACK_OBJECT) {
		Stack_for_each(StackObj_stack(obj), (void (*)(void *, Object*))GC_mark, self);
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
	if (obj->type == NUM_OBJECT) {
		free(ObjToVal(obj, Num));
	} else if (obj->type == FN_OBJECT) {
		Node_drop(FnObj_body(obj));
		free(FnObj_arg(obj));
		free(ObjToVal(obj, Fn));
	} else if (obj->type == THUNK_OBJECT) {
		if (ThunkObj_body(obj)) {
			Node_drop(ThunkObj_body(obj));
		}
		free(ObjToVal(obj, Thunk));
	} else if (obj->type == ENV_OBJECT) {
		Env_drop(EnvObj_env(obj));
	} else if (obj->type == STACK_OBJECT) {
		Stack_drop(StackObj_stack(obj));
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

#define GC_init_object(self, val, otype) ({\
	Object *obj = ValToObj(val);\
	obj->mark = self->curr;\
	obj->type = otype;\
	GC_append_object(self, obj);\
	obj;\
})

Object *GC_alloc_env(GC *self, Object *prev)
{
	return GC_init_object(self, Env_new(prev), ENV_OBJECT);
}

Object *GC_alloc_fn(GC *self, Object *env, const Node *body, const char *arg)
{
	Fn *fn = malloc(sizeof(*fn));
	fn->env = env;
	fn->body = Node_copy(body);
	fn->arg = strdup(arg);
	return GC_init_object(self, fn, FN_OBJECT);
}

Object *GC_alloc_number(GC *self, double num)
{
	Num *n = malloc(sizeof(*n));
	n->num = num;
	return GC_init_object(self, n, NUM_OBJECT);
}

Object *GC_alloc_thunk(GC *self, Object *env, const Node *body)
{
	Thunk *th = malloc(sizeof(*th));
	th->env = env;
	th->body = Node_copy(body);
	th->value = NULL;
	return GC_init_object(self, th, THUNK_OBJECT);
}

Object *GC_alloc_stack(GC *self)
{
	return GC_init_object(self, Stack_new(), STACK_OBJECT);
}

void GC_dump_objects(GC *self)
{
	for (Object *obj = self->first; obj != NULL; obj = obj->next) {
		Object_println(obj);
	}
}
