#include "types.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static Type *Type_alloc(TypeKind kind)
{
	Type *type = malloc(sizeof(*type));
	type->kind = kind;
	return type;
}

static long long unsigned id = 0;

Type *VarType_new()
{
	Type *var = Type_alloc(VAR_TYPE);
	var->as.var = id++;
	return var;
}

void VarType_reset()
{
	id = 0;
}

static Type *VarType_new_from_value(int value)
{
	Type *var = Type_alloc(VAR_TYPE);
	var->as.var = value;
	return var;
}

Type *FnType_new(passed Type *from, passed Type *to)
{
	Type *fn = Type_alloc(FN_TYPE);
	fn->as.fn.from = from;
	fn->as.fn.to = to;
	return fn;
}

Type *GenType_new(passed Type *inner)
{
	Type *gen = Type_alloc(GEN_TYPE);
	gen->as.gen = inner;
	return gen;
}

Type *NumType_get()
{
	static Type num = {.kind = NUM_TYPE};
	return &num;
}

void Type_print(const Type *type)
{
	switch (type->kind) {
		case VAR_TYPE:
			printf("v%d", VarType_value(type));
			break;
		case NUM_TYPE:
			printf("num");
			break;
		case GEN_TYPE:
			Type_print(GenType_inner(type));
			break;
		case FN_TYPE:
			if (FnType_from(type)->kind == FN_TYPE) {
				putchar('(');
				Type_print(FnType_from(type));
				putchar(')');
			} else {
				Type_print(FnType_from(type));
			}
			printf(" -> ");
			Type_print(FnType_to(type));
			break;
	}
}

void Type_drop(passed Type *type)
{
	switch (type->kind) {
		case VAR_TYPE:
			free(type);
			break;
		case FN_TYPE:
			Type_drop(FnType_from(type));
			Type_drop(FnType_to(type));
			free(type);
			break;
		case GEN_TYPE:
			Type_drop(GenType_inner(type));
			free(type);
			break;
		case NUM_TYPE:
			return;
	}
}

Type *Type_copy(const Type *type)
{
	switch (type->kind) {
		case VAR_TYPE:
			return VarType_new_from_value(VarType_value(type));
		case NUM_TYPE:
			return NumType_get();
		case FN_TYPE:
			return FnType_new(Type_copy(FnType_from(type)), Type_copy(FnType_to(type)));
		case GEN_TYPE:
			return GenType_new(Type_copy(GenType_inner(type)));
	}
	return NULL;
}

void Type_println(const Type *type)
{
	Type_print(type);
	putchar('\n');
}

int Type_eq(const Type *t1, const Type *t2)
{
	if (t1->kind != t2->kind) {
		return 0;
	}
	if (t1->kind == GEN_TYPE) {
		return Type_eq(GenType_inner(t1), GenType_inner(t2));
	}
	if (t1->kind == FN_TYPE) {
		return (
			Type_eq(FnType_from(t1), FnType_from(t2)) &&
			Type_eq(FnType_to(t1), FnType_to(t2))
		);
	}
	if (t1->kind == VAR_TYPE) {
		return VarType_value(t1) == VarType_value(t2);
	}
	return 1;
}

TypeEnv *TypeEnv_push(const char *name, const Type *type, passed TypeEnv *prev)
{
	TypeEnv *new = malloc(sizeof(*new));
	new->name = strdup(name);
	new->type = Type_copy(type);
	new->prev = prev;
	return new;
}

Type *TypeEnv_lookup(const TypeEnv *env, const char *name)
{
	while (env != TYPEENV_EMPTY) {
		if (!strcmp(env->name, name)) {
			return env->type;
		}
		env = env->prev;
	}
	return NULL;
}

TypeEnv *TypeEnv_pop(passed TypeEnv *env)
{
	TypeEnv *prev = env->prev;
	free(env->name);
	Type_drop(env->type);
	free(env);
	return prev;
}

void TypeEnv_drop(passed TypeEnv *env)
{
	while (env != TYPEENV_EMPTY) {
		env = TypeEnv_pop(env);
	}
}
