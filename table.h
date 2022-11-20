#ifndef HASH_INCLUDED
#define HASH_INCLUDED

typedef struct HashTable HashTable;

// note: HashTable_add overwrites existing value!
HashTable *HashTable_new();
void      HashTable_delete(HashTable *self);
void      HashTable_add(HashTable *self, const char *key, void *data);
void      *HashTable_remove(HashTable *self, const char *key);
int       HashTable_has(HashTable *self, const char *key);
void      *HashTable_get(HashTable *self, const char *key);

#endif // HASH_INCLUDED
