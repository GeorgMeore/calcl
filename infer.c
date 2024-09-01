#include "infer.h"

#include <string.h>
#include <stdlib.h>

#include "node.h"
#include "types.h"
#include "context.h"
#include "error.h"
#include "arena.h"


// TODO: better error messages

#define ERROR_PREFIX "inference error"

typedef struct Subst Subst;

struct Subst {
	int   var;
	Type  *type;
	Subst *prev;
};

// TODO: remove that garbage and make a normal struct for that
#define SUBST_EMPTY (Subst *)1

static Subst *Subst_extend(int var, Type *type, Subst *prev, Arena *a)
{
	Subst *new = Arena_alloc(a, sizeof(*new));
	new->var = var;
	new->type = type;
	new->prev = prev;
	return new;
}

static Type *Subst_lookup(const Subst *subs, int var)
{
	while (subs != SUBST_EMPTY) {
		if (subs->var == var) {
			return subs->type;
		}
		subs = subs->prev;
	}
	return NULL;
}

static int occurs(Type *var, Type *type, const Subst *subs)
{
	if (type->kind == VAR_TYPE) {
		if (VarType_value(type) == VarType_value(var)) {
			return 1;
		}
		Type *val = Subst_lookup(subs, VarType_value(type));
		return val && occurs(var, val, subs);
	}
	if (type->kind == FN_TYPE) {
		return (
			occurs(var, FnType_from(type), subs) ||
			occurs(var, FnType_to(type), subs)
		);
	}
	return 0;
}

static Subst *unify(Type *t1, Type *t2, Subst *subs, Arena *a);

static Subst *unify_var(Type *t1, Type *t2, Subst *subs, Arena *a)
{
	Type *v1 = Subst_lookup(subs, VarType_value(t1));
	if (v1) {
		return unify(v1, t2, subs, a);
	}
	if (t2->kind == VAR_TYPE) {
		if (VarType_value(t1) == VarType_value(t2)) {
			return subs;
		}
		Type *v2 = Subst_lookup(subs, VarType_value(t2));
		if (v2) {
			return unify(t1, v2, subs, a);
		}
	}
	if (occurs(t1, t2, subs)) {
		error("recursive type");
		return NULL;
	}
	return Subst_extend(VarType_value(t1), t2, subs, a);
}

static Subst *unify_fn(Type *f1, Type *f2, Subst *subs, Arena *a)
{
	subs = unify(FnType_from(f1), FnType_from(f2), subs, a);
	if (!subs) {
		return NULL;
	}
	return unify(FnType_to(f1), FnType_to(f2), subs, a);
}

static Subst *unify(Type *t1, Type *t2, Subst *subs, Arena *a)
{
	if (t1->kind == VAR_TYPE) {
		return unify_var(t1, t2, subs, a);
	} else if (t2->kind == VAR_TYPE) {
		return unify_var(t2, t1, subs, a);
	} else if (t1->kind == t2->kind) {
		if (t1->kind == FN_TYPE) {
			return unify_fn(t1, t2, subs, a);
		} else {
			return subs;
		}
	}
	error("ununifiable types");
	return NULL;
}

static Type *substitute(Type *mono, const Subst *subs, int recursive, Arena *a)
{
	switch (mono->kind) {
		case VAR_TYPE:
			Type *val = Subst_lookup(subs, VarType_value(mono));
			if (!val) {
				return mono;
			}
			if (recursive) {
				return substitute(val, subs, recursive, a);
			}
			return val;
		case NUM_TYPE:
			return NumType_get();
		case FN_TYPE:
			Type *new_from = substitute(FnType_from(mono), subs, recursive, a);
			Type *new_to = substitute(FnType_to(mono), subs, recursive, a);
			return FnType_new(a, new_from, new_to);
		case GEN_TYPE:
			error("unexpected polytype");
	}
	return NULL;
}

static Subst *refresh(Type *mono, Subst *subs, Arena *a)
{
	switch (mono->kind) {
		case VAR_TYPE:
			if (Subst_lookup(subs, VarType_value(mono))) {
				return subs;
			}
			return Subst_extend(VarType_value(mono), VarType_new(a), subs, a);
		case FN_TYPE:
			return refresh(FnType_to(mono), refresh(FnType_from(mono), subs, a), a);
		case NUM_TYPE:
			return subs;
		case GEN_TYPE:
			error("unexpected polytype");
	}
	return NULL;
}

static Type *instantiate(Type *type, Arena *a)
{
	if (type->kind != GEN_TYPE) {
		return type;
	}
	Subst *subs = refresh(GenType_inner(type), SUBST_EMPTY, a);
	Type *mono = substitute(GenType_inner(type), subs, 0, a);
	return mono;
}

static Type *generalize(Type *mono, Arena *a)
{
	VarType_reset();
	Subst *subs = refresh(mono, SUBST_EMPTY, a);
	Type *gen = GenType_new(a, substitute(mono, subs, 0, a));
	return gen;
}

static Subst *M(const Node *expr, TypeEnv *env, Subst *subs, Type *target, Arena *a);

static Subst *M_id(const Node *id, TypeEnv *env, Subst *subs, Type *target, Arena *a)
{
	Type *id_type = TypeEnv_lookup(env, IdNode_value(id));
	if (!id_type) {
		errorf("unbound variable: %s", IdNode_value(id));
		return NULL;
	}
	id_type = instantiate(id_type, a);
	subs = unify(target, id_type, subs, a);
	return subs;
}

static Subst *M_if(const Node *ifelse, TypeEnv *env, Subst *subs, Type *target, Arena *a)
{
	subs = M(IfNode_cond(ifelse), env, subs, NumType_get(), a);
	if (!subs) {
		return NULL;
	}
	subs = M(IfNode_true(ifelse), env, subs, target, a);
	if (!subs) {
		return NULL;
	}
	return M(IfNode_false(ifelse), env, subs, target, a);
}

static Subst *M_fn(const Node *fn, TypeEnv *env, Subst *subs, Type *target, Arena *a)
{
	Type *arg_type = VarType_new(a);
	Type *body_type = VarType_new(a);
	Type *fn_type = FnType_new(a, arg_type, body_type);
	subs = unify(target, fn_type, subs, a);
	if (!subs) {
		return NULL;
	}
	TypeEnv extended = {FnNode_param_value(fn), arg_type, env};
	subs = M(FnNode_body(fn), &extended, subs, body_type, a);
	return subs;
}

static Subst *M_pair(const Node *pair, TypeEnv *env, Subst *subs, Type *target, Arena *a)
{
	subs = unify(target, NumType_get(), subs, a);
	if (!subs) {
		return NULL;
	}
	subs = M(PairNode_left(pair), env, subs, target, a);
	if (!subs) {
		return NULL;
	}
	return M(PairNode_right(pair), env, subs, target, a);
}

static Subst *M_application(const Node *appl, TypeEnv *env, Subst *subs, Type *target, Arena *a)
{
	Type *operand_type = VarType_new(a);
	Type *operator_type = FnType_new(a, operand_type, target);
	subs = M(PairNode_left(appl), env, subs, operator_type, a);
	if (!subs) {
		return NULL;
	}
	subs = M(PairNode_right(appl), env, subs, operand_type, a);
	return subs;
}

static Subst *M_let(const Node *let, TypeEnv *env, Subst *subs, Type *target, Arena *a)
{
	TypeEnv extended = {LetNode_name_value(let), target, env};
	subs = M(LetNode_value(let), &extended, subs, target, a);
	return subs;
}

static Subst *M(const Node *expr, TypeEnv *env, Subst *subs, Type *target, Arena *a)
{
	switch (expr->type) {
		case NUMBER_NODE:
			return unify(target, NumType_get(), subs, a);
		case ID_NODE:
			return M_id(expr, env, subs, target, a);
		case IF_NODE:
			return M_if(expr, env, subs, target, a);
		case FN_NODE:
			return M_fn(expr, env, subs, target, a);
		case SUM_NODE:
		case PRODUCT_NODE:
		case EXPT_NODE:
		case CMP_NODE:
		case AND_NODE:
		case OR_NODE:
			return M_pair(expr, env, subs, target, a);
		case APPLICATION_NODE:
			return M_application(expr, env, subs, target, a);
		case LET_NODE:
			return M_let(expr, env, subs, target, a);
	}
	return NULL;
}

Type *infer(const Node *expr, TypeEnv **tenv, Arena *a)
{
	Type *target = VarType_new(a);
	Subst *subs = M(expr, *tenv, SUBST_EMPTY, target, a);
	if (!subs) {
		return NULL;
	}
	Type *mono = substitute(target, subs, 1, a);
	Type *poly = generalize(mono, a);
	if (expr->type == LET_NODE) {
		const char *name = LetNode_name_value(expr);
		Type *old = TypeEnv_lookup(*tenv, name);
		if (!old) {
			TypeEnv_push(tenv, name, poly);
		} else if (!Type_eq(old, poly)) {
			error("symbol type cannot change");
			return NULL;
		}
	}
	return poly;
}
