#ifndef OBJECT_INCLUDED
#define OBJECT_INCLUDED

#include "node.h"


typedef struct Object Object;

typedef struct Env Env; // implemented in env.c

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

#endif // OBJECT_INCLUDED
