#include "object.h"

#include <stdio.h>


void Object_print(const Object *obj)
{
	switch (obj->type) {
		case NUM_OBJECT:
			printf("%lf", NumObj_num(obj));
			return;
		case FN_OBJECT:
			printf("<fn %s>", FnObj_arg(obj));
			return;
		case ENV_OBJECT:
			printf("<env-%p>", obj);
			return;
		case THUNK_OBJECT:
			printf("<thunk-%p>", obj);
			return;
		case STACK_OBJECT:
			printf("<stack-%p>", obj);
			return;
	}
}

void Object_println(const Object *obj)
{
	Object_print(obj);
	putchar('\n');
}
