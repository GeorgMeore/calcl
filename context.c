#include "context.h"

#include <stddef.h>

#include "gc.h"
#include "types.h"


Context Context_make(void)
{
	Context self = {0};
	self.gc = GC_new();
	self.root = GC_alloc_env(self.gc, NULL);
	self.stack = GC_alloc_stack(self.gc);
	self.tenv = TYPEENV_EMPTY;
	return self;
}

void Context_destroy(Context self)
{
	GC_collect(self.gc, NULL, NULL);
	GC_drop(self.gc);
	TypeEnv_drop(self.tenv);
}
