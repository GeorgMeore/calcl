#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED

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
Type *VarType_new();
void VarType_reset();
Type *FnType_new(Type *from, Type *to);
Type *GenType_new(Type *inner);
Type *NumType_get();
Type *Type_copy(const Type *type);
void Type_drop(Type *type);
int  Type_eq(const Type *t1, const Type *t2);

typedef struct TypeEnv TypeEnv;

struct TypeEnv {
	char    *name;
	Type    *type;
	TypeEnv *prev;
};

#define TYPEENV_EMPTY (TypeEnv *)0

TypeEnv *TypeEnv_push(const char *name, const Type *type, TypeEnv *prev);
TypeEnv *TypeEnv_pop(TypeEnv *env);
Type    *TypeEnv_lookup(const TypeEnv *env, const char *name);
void    TypeEnv_drop(TypeEnv *env);

#endif // TYPES_INCLUDED
