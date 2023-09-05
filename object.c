#include "object.h"

#include <stdio.h>


void Object_println(Object *obj)
{
	switch (obj->type) {
		case NUM_OBJECT:
			printf("%lf\n", obj->as.num);
			return;
		case FN_OBJECT:
			printf("<fn %s>\n", obj->as.fn.arg);
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
