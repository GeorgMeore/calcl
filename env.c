#include "env.h"

#include <stdlib.h>
#include <string.h>


#define INITIAL_TABLE_SIZE 512

typedef struct TableEntry TableEntry;

struct TableEntry {
	void *data;
	char *key;
	TableEntry *next;
};

struct Env {
	TableEntry **entries;
	int size;
	int taken;
};

static TableEntry *TableEntry_new(const char *key, void *data)
{
	TableEntry *entry = malloc(sizeof(*entry));
	entry->key = malloc(strlen(key) + 1);
	strcpy(entry->key, key);
	entry->data = data;
	entry->next = NULL;
	return entry;
}

static void TableEntry_delete(TableEntry *self)
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

Env *Env_new()
{
	Env *table = malloc(sizeof(*table));
	table->entries = calloc(INITIAL_TABLE_SIZE, sizeof(TableEntry));
	table->size = INITIAL_TABLE_SIZE;
	table->taken = 0;
	return table;
}

void Env_delete(Env *self)
{
	for (int i = 0; i < self->size; i++) {
		TableEntry *head = self->entries[i];
		while (head) {
			TableEntry *next = head->next;
			TableEntry_delete(head);
			head = next;
		}
	}
	free(self->entries);
	free(self);
}

static void Env_resize(Env *self, int new_size)
{
	TableEntry **old_entries = self->entries;
	int old_size = self->size;
	self->entries = calloc(new_size, sizeof(TableEntry));
	self->size = new_size;
	self->taken = 0;
	for (int i = 0; i < old_size; i++) {
		TableEntry *head = old_entries[i];
		while (head) {
			TableEntry *next = head->next;
			Env_add(self, head->key, head->data);
			TableEntry_delete(head);
			head = next;
		}
	}
	free(old_entries);
}

static TableEntry **find_entry(Env *self, const char *key)
{
	int index = dbj2_hash(key) % self->size;
	TableEntry **indirect = &(self->entries[index]);
	while (*indirect) {
		TableEntry *watched = *indirect;
		if (!strcmp(watched->key, key)) {
			break;
		}
		indirect = &watched->next;
	}
	return indirect;
}

void Env_add(Env *self, const char *key, void *data)
{
	TableEntry **indirect = find_entry(self, key);
	if (*indirect) {
		(*indirect)->data = data;
	} else {
		*indirect = TableEntry_new(key, data);
		self->taken += 1;
	}
	if (self->taken > self->size / 2) {
		Env_resize(self, self->size * 2);
	}
}

void *Env_remove(Env *self, const char *key)
{
	TableEntry **indirect = find_entry(self, key);
	if (*indirect) {
		TableEntry *target = *indirect;
		(*indirect) = target->next;
		void *data = target->data;
		TableEntry_delete(target);
		return data;
	}
	return NULL;
}

int Env_has(Env *self, const char *key)
{
	return *find_entry(self, key) == NULL;
}

void *Env_get(Env *self, const char *key)
{
	TableEntry *entry = *find_entry(self, key);
	if (entry) {
		return entry->data;
	}
	return NULL;
}
