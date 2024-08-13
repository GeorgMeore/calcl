#ifndef VALUES_INCLUDED
#define VALUES_INCLUDED

#include "object.h"
#include "node.h"

typedef struct {
	Object *env;
	Node   *body;
	char   *arg;
	Object handle;
} Fn;

#define FnObj_env(objptr) (ObjToVal(objptr, Fn)->env)
#define FnObj_body(objptr) (ObjToVal(objptr, Fn)->body)
#define FnObj_arg(objptr) (ObjToVal(objptr, Fn)->arg)

typedef struct {
	Object *env;
	Node   *body;
	Object *value;
	Object handle;
} Thunk;

#define ThunkObj_env(objptr) (ObjToVal(objptr, Thunk)->env)
#define ThunkObj_body(objptr) (ObjToVal(objptr, Thunk)->body)
#define ThunkObj_value(objptr) (ObjToVal(objptr, Thunk)->value)
#define ThunkObj_set_value(objptr, value) ({\
	Node_drop(ObjToVal(objptr, Thunk)->body);\
	ObjToVal(objptr, Thunk)->body = NULL;\
	ObjToVal(objptr, Thunk)->env = NULL;\
	ObjToVal(objptr, Thunk)->value = (value);\
})

typedef struct {
	double num;
	Object handle;
} Num;

#define NumObj_num(objptr) (ObjToVal(objptr, Num)->num)

#endif // VALUES_INCLUDED
