#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED

#include "arena.h"

typedef struct Type Type;

// TODO: polymorphic inductive types
typedef enum {
	GenType,
	VarType,
	NumType,
	FnType,
} TypeKind;

typedef struct {
	Type *from;
	Type *to;
} FnTypeValue;

#define FnType_from(typeptr) ((typeptr)->as.fn.from)
#define FnType_to(typeptr) ((typeptr)->as.fn.to)

typedef union {
	int         var;
	FnTypeValue fn;
	Type        *gen;
} TypeValue;

#define VarType_value(typeptr) ((typeptr)->as.var)
#define GenType_inner(typeptr) ((typeptr)->as.gen)

struct Type {
	TypeKind  kind;
	TypeValue as;
};

void Type_print(const Type *type);
void Type_println(const Type *type);

Type *NumType_get(void);
Type *VarType_new(Arena *a);
void VarType_reset(void);
Type *FnType_new(Arena *a, Type *from, Type *to);
Type *GenType_new(Arena *a, Type *inner);

Type *Type_copy(const Type *type); // NOTE: the return value is malloc'ed, must be Type_drop'ed
void Type_drop(Type *type);
int  Type_eq(const Type *t1, const Type *t2);

typedef struct TypeEnv TypeEnv;

struct TypeEnv {
	char    *name;
	Type    *type;
	TypeEnv *prev;
};

#define TYPEENV_EMPTY (TypeEnv *)0

void TypeEnv_push(TypeEnv **env, const char *name, const Type *type);
Type *TypeEnv_lookup(const TypeEnv *env, const char *name);
void TypeEnv_drop(TypeEnv *env);

#endif // TYPES_INCLUDED
