#include "gc.h"

#include <stdlib.h>

#include "node.h"
#include "object.h"
#include "env.h"
#include "stack.h"


GC *GC_new()
{
	GC *self = malloc(sizeof(*self));
	self->first = NULL;
	self->last = NULL;
	self->curr = 0;
	self->count = 0;
	self->thres = GC_INITIAL_THRESHOLD;
	return self;
}

void GC_drop(passed GC *self)
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
	if (obj->type == FN_OBJECT) {
		Node_drop(FnObj_body(obj));
		free(FnObj_arg(obj));
	} else if (obj->type == THUNK_OBJECT) {
		if (ThunkObj_body(obj)) {
			Node_drop(ThunkObj_body(obj));
		}
	} else if (obj->type == ENV_OBJECT) {
		Env_drop(EnvObj_env(obj));
	} else if (obj->type == STACK_OBJECT) {
		Stack_drop(StackObj_stack(obj));
	}
	free(obj);
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

static Object *GC_alloc_empty_object(GC *self)
{
	Object *obj = malloc(sizeof(*obj));
	obj->mark = self->curr;
	GC_append_object(self, obj);
	return obj;
}

Object *GC_alloc_env(GC *self, Object *prev)
{
	Object *obj = GC_alloc_empty_object(self);
	obj->type = ENV_OBJECT;
	obj->as.env = Env_new(prev);
	return obj;
}

Object *GC_alloc_fn(GC *self, Object *env, Node *body, char *arg)
{
	Object *obj = GC_alloc_empty_object(self);
	obj->type = FN_OBJECT;
	obj->as.fn.env = env;
	obj->as.fn.body = body;
	obj->as.fn.arg = arg;
	return obj;
}

Object *GC_alloc_number(GC *self, double num)
{
	Object *obj = GC_alloc_empty_object(self);
	obj->type = NUM_OBJECT;
	obj->as.num = num;
	return obj;
}

Object *GC_alloc_thunk(GC *self, Object *env, Node *body)
{
	Object *obj = GC_alloc_empty_object(self);
	obj->type = THUNK_OBJECT;
	obj->as.thunk.env = env;
	obj->as.thunk.body = body;
	obj->as.thunk.value = NULL;
	return obj;
}

Object *GC_alloc_stack(GC *self)
{
	Object *obj = GC_alloc_empty_object(self);
	obj->type = STACK_OBJECT;
	obj->as.stack = Stack_new();
	return obj;
}

void GC_dump_objects(GC *self)
{
	for (Object *obj = self->first; obj != NULL; obj = obj->next) {
		Object_println(obj);
	}
}
