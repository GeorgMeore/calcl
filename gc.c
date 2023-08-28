#include "gc.h"

#include <stdlib.h>

#include "object.h"
#include "env.h"


GC *GC_new()
{
	GC *self = malloc(sizeof(*self));
	self->first = NULL;
	self->last = NULL;
	self->curr = 0;
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
		GC_mark(self, obj->as.fn.env);
	} else if (obj->type == THUNK_OBJECT) {
		if (obj->as.thunk.value) {
			GC_mark(self, obj->as.thunk.value);
		} else {
			GC_mark(self, obj->as.thunk.env);
		}
	} else if (obj->type == ENV_OBJECT) {
		Env_for_each(obj->as.env, (void (*)(void *, Object*))GC_mark, self);
		if (obj->as.env->prev) {
			GC_mark(self, obj->as.env->prev);
		}
	}
}

static void GC_append_object(GC *self, Object *obj)
{
	obj->next = NULL;
	if (!self->first) {
		self->first = obj;
		self->last = obj;
	} else {
		self->last->next = obj;
		self->last = obj;
	}
}

static void GC_sweep(GC *self)
{
	Object *obj = self->first;
	self->first = self->last = NULL;
	while (obj != NULL) {
		Object *next = obj->next;
		if (obj->mark != self->curr) {
			if (obj->type == FN_OBJECT) {
				Node_drop(obj->as.fn.body);
				free(obj->as.fn.arg);
			} else if (obj->type == THUNK_OBJECT) {
				if (!obj->as.thunk.value) {
					Node_drop(obj->as.thunk.body);
				}
			}else if (obj->type == ENV_OBJECT) {
				Env_drop(obj->as.env);
			}
			free(obj);
		} else {
			GC_append_object(self, obj);
		}
		obj = next;
	}
}

void GC_collect(GC *self, Object *root)
{
	self->curr = !self->curr;
	if (root) {
		GC_mark(self, root);
	}
	GC_sweep(self);
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
