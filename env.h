#ifndef HASH_INCLUDED
#define HASH_INCLUDED

#include "object.h"

// NOTE: Env_add overwrites the existing value!
Env     *Env_new(Object *prev);
void    Env_drop(Env *self);
void    Env_add(Env *self, const char *key, Object *obj);
Object  *Env_remove(Env *self, const char *key);
int     Env_has(Env *self, const char *key);
Object  *Env_get(Env *self, const char *key);
void    Env_for_each(Env *self, void (*fn)(void *, Object *), void *param);

#endif // HASH_INCLUDED
