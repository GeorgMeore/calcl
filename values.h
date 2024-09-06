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
	void   *text;
	Object handle;
} CompFn;

#define CompFnObj_env(objptr) (ObjToVal(objptr, CompFn)->env)
#define CompFnObj_text(objptr) (ObjToVal(objptr, CompFn)->text)

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
	Object *env;
	void   *text;
	Object *value;
	Object handle;
} CompThunk;

#define CompThunkObj_env(objptr) (ObjToVal(objptr, CompThunk)->env)
#define CompThunkObj_text(objptr) (ObjToVal(objptr, CompThunk)->text)
#define CompThunkObj_value(objptr) (ObjToVal(objptr, CompThunk)->value)

typedef struct {
	double num;
	Object handle;
} Num;

#define NumObj_num(objptr) (ObjToVal(objptr, Num)->num)

#endif // VALUES_INCLUDED
