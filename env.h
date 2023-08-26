#ifndef HASH_INCLUDED
#define HASH_INCLUDED

typedef struct Env Env;

// note: Env_add overwrites existing value!
Env   *Env_new();
void  Env_delete(Env *self);
void  Env_add(Env *self, const char *key, void *data);
void  *Env_remove(Env *self, const char *key);
int   Env_has(Env *self, const char *key);
void  *Env_get(Env *self, const char *key);

#endif // HASH_INCLUDED
