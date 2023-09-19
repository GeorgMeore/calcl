#ifndef OBJECT_INCLUDED
#define OBJECT_INCLUDED

#include "node.h"

typedef struct Object Object;

typedef struct {
	Object **objects;
	int    capacity;
	int    size;
} Stack;

typedef struct Binding Binding;

struct Binding {
	Object  *obj;
	char    *key;
	Binding *next;
};

typedef struct Env Env;

struct Env {
	Binding **entries;
	int     size;
	int     taken;
	Object  *prev;
};

typedef enum {
	FN_OBJECT,
	ENV_OBJECT,
	NUM_OBJECT,
	THUNK_OBJECT,
	STACK_OBJECT,
} ObjectType;

typedef struct {
	Object *env;
	Node   *body;
	char   *arg;
} FnObject;

#define FnObj_env(objptr) ((objptr)->as.fn.env)
#define FnObj_body(objptr) ((objptr)->as.fn.body)
#define FnObj_arg(objptr) ((objptr)->as.fn.arg)

typedef struct {
	Object *env;
	Node   *body;
	Object *value;
} ThunkObject;

#define ThunkObj_env(objptr) ((objptr)->as.thunk.env)
#define ThunkObj_body(objptr) ((objptr)->as.thunk.body)
#define ThunkObj_value(objptr) ((objptr)->as.thunk.value)
#define ThunkObj_set_value(objptr, value) ({ \
	(objptr)->as.thunk.body = NULL;          \
	(objptr)->as.thunk.env = NULL;           \
	(objptr)->as.thunk.value = (value);      \
})

typedef union {
	FnObject    fn;
	Env         *env;
	double      num;
	ThunkObject thunk;
	Stack       *stack;
} ObjectValue;

#define EnvObj_env(objptr) ((objptr)->as.env)
#define EnvObj_prev(objptr) ((objptr)->as.env->prev)
#define NumObj_num(objptr) ((objptr)->as.num)
#define StackObj_stack(objptr) ((objptr)->as.stack)

struct Object {
	ObjectValue as;
	ObjectType  type;
	Object      *next;
	int         mark;
};

void Object_print(const Object *obj);
void Object_println(const Object *obj);

#endif // OBJECT_INCLUDED
