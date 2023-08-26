#ifndef OBJECT_INCLUDED
#define OBJECT_INCLUDED

#include "node.h"


typedef struct Object Object;

typedef struct Binding Binding;

struct Binding {
	Object *obj;
	char *key;
	Binding *next;
};

typedef struct Env Env;

struct Env {
	Binding **entries;
	int size;
	int taken;
	Object *prev;
};

typedef enum {
	FN_OBJECT,
	ENV_OBJECT,
	NUM_OBJECT,
} ObjectType;

typedef struct {
	Object *env;
	Node *body;
	char *arg;
} FnObject;

typedef union {
	FnObject fn;
	Env *env;
	double num;
} ObjectValue;

struct Object {
	ObjectValue as;
	ObjectType type;
	Object *next;
	int mark;
};

void Object_println(Object *obj);

#endif // OBJECT_INCLUDED
