#ifndef OBJECT_INCLUDED
#define OBJECT_INCLUDED

#include <stddef.h>

typedef enum {
	FnObject,
	CompfnObject,
	EnvObject,
	NumObject,
	ThunkObject,
	CompthunkObject,
	StackObject,
} ObjectType;

typedef struct Object Object;

struct Object {
	ObjectType  type;
	Object      *next;
	int         mark;
};

#define ValToObj(val) (&(val)->handle)
#define ObjToVal(objptr, type) ((type *)((char *)(objptr) - offsetof(type, handle)))
#define ObjValOff(type) (-(int)offsetof(type, handle))
#define ObjFldOff(type, field) ((int)offsetof(type, field) - (int)offsetof(type, handle))

void Object_print(const Object *obj);
void Object_println(const Object *obj);

#endif // OBJECT_INCLUDED
