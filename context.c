#include "context.h"

#include <stddef.h>

#include "gc.h"


Context Context_make()
{
	Context self = {0};
	self.gc = GC_new();
	self.root = GC_alloc_env(self.gc, NULL);
	self.stack = GC_alloc_stack(self.gc);
	return self;
}

void Context_destroy(Context self)
{
	GC_collect(self.gc, NULL, NULL);
	GC_drop(self.gc);
}
