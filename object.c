#include "object.h"

#include <stdio.h>

#include "values.h"


void Object_print(const Object *obj)
{
	switch (obj->type) {
		case NumObject:
			printf("%lf", NumObj_num(obj));
			return;
		case FnObject:
			printf("<fn %s>", FnObj_arg(obj));
			return;
		case CompfnObject:
			printf("<compfn %p>", CompFnObj_text(obj));
			return;
		case EnvObject:
			printf("<env-%p>", obj);
			return;
		case ThunkObject:
			printf("<thunk-%p>", obj);
			return;
		case CompthunkObject:
			printf("<compthunk-%p>", CompThunkObj_text(obj));
			return;
		case StackObject:
			printf("<stack-%p>", obj);
			return;
	}
}

void Object_println(const Object *obj)
{
	Object_print(obj);
	putchar('\n');
}
