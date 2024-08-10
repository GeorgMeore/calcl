#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED

#include "annotations.h"
#include "arena.h"

typedef struct Type Type;

typedef enum {
	GEN_TYPE,
	VAR_TYPE,
	NUM_TYPE,
	FN_TYPE,
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

// basic types (only number for now)
extern Type num_type;

// complex types
Type *VarType_new(Arena *a);
void VarType_reset(void);
Type *FnType_new(Arena *a, Type *from, Type *to);
Type *GenType_new(Arena *a, Type *inner);

Type *Type_copy(const Type *type);
void Type_drop(passed Type *type);
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
void TypeEnv_drop(passed TypeEnv *env);

#endif // TYPES_INCLUDED
