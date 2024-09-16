#include "types.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "arena.h"


static Type *Type_alloc(Arena *a, TypeKind kind)
{
	Type *type = a ? Arena_alloc(a, sizeof(*type)) : malloc(sizeof(*type));
	type->kind = kind;
	return type;
}

static long long unsigned id = 0;

Type *VarType_new(Arena *a)
{
	Type *var = Type_alloc(a, VarType);
	var->as.var = id++;
	return var;
}

void VarType_reset()
{
	id = 0;
}

static Type *VarType_new_from_value(Arena *a, int value)
{
	Type *var = Type_alloc(a, VarType);
	var->as.var = value;
	return var;
}

Type *FnType_new(Arena *a, Type *from, Type *to)
{
	Type *fn = Type_alloc(a, FnType);
	fn->as.fn.from = from;
	fn->as.fn.to = to;
	return fn;
}

Type *GenType_new(Arena *a, Type *inner)
{
	Type *gen = Type_alloc(a, GenType);
	gen->as.gen = inner;
	return gen;
}

Type *NumType_get(void)
{
	static Type num = {.kind = NumType};
	return &num;
}

void Type_print(const Type *type)
{
	switch (type->kind) {
		case VarType:
			printf("v%d", VarType_value(type));
			break;
		case NumType:
			printf("num");
			break;
		case GenType:
			Type_print(GenType_inner(type));
			break;
		case FnType:
			if (FnType_from(type)->kind == FnType) {
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

void Type_drop(Type *type)
{
	switch (type->kind) {
		case VarType:
			free(type);
			break;
		case FnType:
			Type_drop(FnType_from(type));
			Type_drop(FnType_to(type));
			free(type);
			break;
		case GenType:
			Type_drop(GenType_inner(type));
			free(type);
			break;
		case NumType:
			return;
	}
}

Type *Type_copy(const Type *type)
{
	switch (type->kind) {
		case VarType:
			return VarType_new_from_value(NULL, VarType_value(type));
		case NumType:
			return NumType_get();
		case FnType:
			return FnType_new(NULL, Type_copy(FnType_from(type)), Type_copy(FnType_to(type)));
		case GenType:
			return GenType_new(NULL, Type_copy(GenType_inner(type)));
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
	if (t1->kind == GenType) {
		return Type_eq(GenType_inner(t1), GenType_inner(t2));
	}
	if (t1->kind == FnType) {
		return (
			Type_eq(FnType_from(t1), FnType_from(t2)) &&
			Type_eq(FnType_to(t1), FnType_to(t2))
		);
	}
	if (t1->kind == VarType) {
		return VarType_value(t1) == VarType_value(t2);
	}
	return 1;
}

void TypeEnv_push(TypeEnv **env, const char *name, const Type *type)
{
	TypeEnv *new = malloc(sizeof(*new));
	new->name = strdup(name);
	new->type = Type_copy(type);
	new->prev = *env;
	*env = new;
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

void TypeEnv_drop(TypeEnv *env)
{
	while (env != TYPEENV_EMPTY) {
		TypeEnv *prev = env->prev;
		Type_drop(env->type);
		free(env->name);
		free(env);
		env = prev;
	}
}
