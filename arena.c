#include "arena.h"

#include <stddef.h>
#include <stdlib.h>

// data alignment
#define WORDSIZE sizeof(size_t)
#define ALIGN(v) (v % WORDSIZE == 0 ? v : v + WORDSIZE - v % WORDSIZE)

typedef struct Page Page;

struct Page {
	Page   *next;
	char   *data;
	size_t taken;
};

static void Page_drop(Page *self)
{
	free(self->data);
	free(self);
}

static Page *Page_new(size_t page_size)
{
	Page *self = malloc(sizeof(*self));
	if (!self) {
		return NULL;
	}
	self->data = malloc(page_size);
	if (!self->data) {
		free(self);
		return NULL;
	}
	self->next = NULL;
	self->taken = 0;
	return self;
}

struct Arena {
	Page   *first;
	size_t page_size;
};

Arena *Arena_new(size_t page_size)
{
	page_size = ALIGN(page_size);
	Arena *self = malloc(sizeof(*self));
	if (!self) {
		return NULL;
	}
	self->first = Page_new(page_size);
	if (!self->first) {
		free(self);
		return NULL;
	}
	self->page_size = page_size;
	return self;
}

void *Arena_alloc(Arena *self, size_t size)
{
	size = ALIGN(size);
	if (size > self->page_size) {
		return NULL;
	}
	Page *p = self->first;
	while (p) {
		if (self->page_size - p->taken >= size) {
			char *mem = p->data + p->taken;
			p->taken += size;
			return mem;
		}
		if (p->next == NULL) {
			p->next = Page_new(self->page_size);
		}
		p = p->next;
	}
	return NULL;
}

void Arena_reset(Arena *self)
{
	Page *p = self->first;
	while (p) {
		p->taken = 0;
		p = p->next;
	}
}

void Arena_drop(Arena *self)
{
	Page *p = self->first;
	while (p) {
		Page *next = p->next;
		Page_drop(p);
		p = next;
	}
	free(self);
}
