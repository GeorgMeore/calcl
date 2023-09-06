#include "object.h"

#include <stdio.h>


void Object_println(Object *obj)
{
	switch (obj->type) {
		case NUM_OBJECT:
			printf("%lf\n", NumObj_num(obj));
			return;
		case FN_OBJECT:
			printf("<fn %s>\n", FnObj_arg(obj));
			return;
		case ENV_OBJECT:
			printf("<env-%p>\n", obj);
			return;
		case THUNK_OBJECT:
			printf("<thunk-%p>\n", obj);
			return;
		case STACK_OBJECT:
			printf("<stack-%p>\n", obj);
			return;
	}
}
