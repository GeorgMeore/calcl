#ifndef HASH_INCLUDED
#define HASH_INCLUDED

#include "object.h"

typedef struct Binding Binding;

typedef struct Env Env;

struct Env {
	Binding **entries;
	int     size;
	int     taken;
	Object  *prev;
	Object  handle;
};

#define EnvObj_env(objptr) (ObjToVal(objptr, Env))
#define EnvObj_prev(objptr) (ObjToVal(objptr, Env)->prev)

// NOTE: Env_add overwrites the existing value!
Env     *Env_new(Object *prev);
void    Env_drop(Env *self);
void    Env_add(Env *self, const char *key, Object *obj);
Object  *Env_remove(Env *self, const char *key);
Object  *Env_get(const Env *self, const char *key);
void    Env_for_each(const Env *self, void (*fn)(void *, Object *), void *param);
void    Env_dump_objects(const Env *self);

#endif // HASH_INCLUDED
