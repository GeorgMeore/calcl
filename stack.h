#ifndef STACK_INCLUDED
#define STACK_INCLUDED

#include "object.h"

#define INITIAL_STACK_CAPACITY 100

Stack  *Stack_new();
void   Stack_drop(Stack *self);
void   Stack_push(Stack *self, Object *value);
Object *Stack_pop(Stack *self);
void   Stack_clear(Stack *self);
void   Stack_for_each(const Stack *self, void (*fn)(void *, Object *), void *param);

#endif // STACK_INCLUDED
