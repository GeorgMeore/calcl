#include "env.h"

#include <stdlib.h>
#include <string.h>

#include "object.h"


#define INITIAL_TABLE_SIZE 512

static Binding *Binding_new(const char *key, Object *obj)
{
	Binding *entry = malloc(sizeof(*entry));
	entry->key = strdup(key);
	entry->obj = obj;
	entry->next = NULL;
	return entry;
}

static void Binding_drop(Binding *self)
{
	free(self->key);
	free(self);
}

static unsigned long dbj2_hash(const char *str)
{
	unsigned long hash = 5381;
	while (*str++) {
		hash = ((hash << 5) + hash) + *str;
	}
	return hash;
}

Env *Env_new(Object *prev)
{
	Env *self = malloc(sizeof(*self));
	self->entries = calloc(INITIAL_TABLE_SIZE, sizeof(Binding));
	self->size = INITIAL_TABLE_SIZE;
	self->taken = 0;
	self->prev = prev;
	return self;
}

void Env_drop(passed Env *self)
{
	for (int i = 0; i < self->size; i++) {
		Binding *head = self->entries[i];
		while (head) {
			Binding *next = head->next;
			Binding_drop(head);
			head = next;
		}
	}
	free(self->entries);
	free(self);
}

static void Env_resize(Env *self, int new_size)
{
	Binding **old_entries = self->entries;
	int old_size = self->size;
	self->entries = calloc(new_size, sizeof(Binding));
	self->size = new_size;
	self->taken = 0;
	for (int i = 0; i < old_size; i++) {
		Binding *head = old_entries[i];
		while (head) {
			Binding *next = head->next;
			Env_add(self, head->key, head->obj);
			Binding_drop(head);
			head = next;
		}
	}
	free(old_entries);
}

static Binding **find_entry(const Env *self, const char *key)
{
	int index = dbj2_hash(key) % self->size;
	Binding **indirect = &(self->entries[index]);
	while (*indirect) {
		Binding *watched = *indirect;
		if (!strcmp(watched->key, key)) {
			break;
		}
		indirect = &watched->next;
	}
	return indirect;
}

void Env_add(Env *self, const char *key, Object *obj)
{
	Binding **indirect = find_entry(self, key);
	if (*indirect) {
		(*indirect)->obj = obj;
	} else {
		*indirect = Binding_new(key, obj);
		self->taken += 1;
	}
	if (self->taken > self->size / 2) {
		Env_resize(self, self->size * 2);
	}
}

Object *Env_remove(Env *self, const char *key)
{
	Binding **indirect = find_entry(self, key);
	if (*indirect) {
		Binding *target = *indirect;
		(*indirect) = target->next;
		Object *obj = target->obj;
		Binding_drop(target);
		return obj;
	}
	return NULL;
}

int Env_has(const Env *self, const char *key)
{
	return *find_entry(self, key) == NULL;
}

Object *Env_get(const Env *self, const char *key)
{
	Binding *entry = *find_entry(self, key);
	if (entry) {
		return entry->obj;
	}
	if (self->prev) {
		return Env_get(EnvObj_env(self->prev), key);
	}
	return NULL;
}

void Env_for_each(const Env *self, void (*fn)(void *, Object *), void *param)
{
	for (int i = 0; i < self->size; i++) {
		for (Binding *entry = self->entries[i]; entry != NULL; entry = entry->next) {
			fn(param, entry->obj);
		}
	}
}

void Env_dump_objects(const Env *self)
{
	for (int i = 0; i < self->size; i++) {
		for (Binding *entry = self->entries[i]; entry != NULL; entry = entry->next) {
			Object_println(entry->obj);
		}
	}
	if (self->prev) {
		Env_dump_objects(EnvObj_env(self->prev));
	}
}
