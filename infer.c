#include "infer.h"

#include <string.h>
#include <stdlib.h>

#include "node.h"
#include "types.h"
#include "context.h"
#include "annotations.h"
#include "error.h"


// TODO: there is a looot of tricky copying, an arena allocator should be used here
// TODO: better error messages

#define ERROR_PREFIX "inference error"

typedef struct Subst Subst;

struct Subst {
	int   var;
	Type  *type;
	Subst *prev;
};

#define SUBST_EMPTY (Subst *)1

static Subst *Subst_extend(int var, const Type *type, passed Subst *prev)
{
	Subst *new = malloc(sizeof(*new));
	new->var = var;
	new->type = Type_copy(type);
	new->prev = prev;
	return new;
}

static Subst *Subst_extend_no_copy(int var, passed Type *type, passed Subst *prev)
{
	Subst *new = malloc(sizeof(*new));
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

static void Subst_drop(passed Subst *subs)
{
	while (subs != SUBST_EMPTY) {
		Subst *prev = subs->prev;
		Type_drop(subs->type);
		free(subs);
		subs = prev;
	}
}

static int occurs(const Type *var, const Type *type, const Subst *subs)
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

static Subst *unify(const Type *t1, const Type *t2, Subst *subs);

static Subst *unify_var(const Type *t1, const Type *t2, Subst *subs)
{
	Type *v1 = Subst_lookup(subs, VarType_value(t1));
	if (v1) {
		return unify(v1, t2, subs);
	}
	if (t2->kind == VAR_TYPE) {
		if (VarType_value(t1) == VarType_value(t2)) {
			return subs;
		}
		Type *v2 = Subst_lookup(subs, VarType_value(t2));
		if (v2) {
			return unify(t1, v2, subs);
		}
	}
	if (occurs(t1, t2, subs)) {
		error("recursive type");
		Subst_drop(subs);
		return NULL;
	}
	return Subst_extend(VarType_value(t1), t2, subs);
}

static Subst *unify_fn(const Type *f1, const Type *f2, Subst *subs)
{
	subs = unify(FnType_from(f1), FnType_from(f2), subs);
	if (!subs) {
		return NULL;
	}
	return unify(FnType_to(f1), FnType_to(f2), subs);
}

static Subst *unify(const Type *t1, const Type *t2, Subst *subs)
{
	if (t1->kind == VAR_TYPE) {
		return unify_var(t1, t2, subs);
	} else if (t2->kind == VAR_TYPE) {
		return unify_var(t2, t1, subs);
	} else if (t1->kind == t2->kind) {
		if (t1->kind == FN_TYPE) {
			return unify_fn(t1, t2, subs);
		} else {
			return subs;
		}
	}
	error("ununifiable types");
	Subst_drop(subs);
	return NULL;
}

#define SUB_ONELEVEL  0
#define SUB_RECURSIVE 1

static Type *substitute(const Type *mono, const Subst *subs, int recursive)
{
	switch (mono->kind) {
		case VAR_TYPE:
			Type *val = Subst_lookup(subs, VarType_value(mono));
			if (!val) {
				return Type_copy(mono);
			}
			if (recursive == SUB_RECURSIVE) {
				return substitute(val, subs, recursive);
			}
			return Type_copy(val);
		case NUM_TYPE:
			return NumType_get();
		case FN_TYPE:
			Type *new_from = substitute(FnType_from(mono), subs, recursive);
			Type *new_to = substitute(FnType_to(mono), subs, recursive);
			return FnType_new(new_from, new_to);
		case GEN_TYPE:
			error("unexpected polytype");
	}
	return NULL;
}

static Subst *refresh(const Type *mono, Subst *subs)
{
	switch (mono->kind) {
		case VAR_TYPE:
			if (Subst_lookup(subs, VarType_value(mono))) {
				return subs;
			}
			return Subst_extend_no_copy(VarType_value(mono), VarType_new(), subs);
		case FN_TYPE:
			return refresh(FnType_to(mono), refresh(FnType_from(mono), subs));
		case NUM_TYPE:
			return subs;
		case GEN_TYPE:
			error("unexpected polytype");
	}
	return NULL;
}

static Type *instantiate(const Type *type)
{
	if (type->kind != GEN_TYPE) {
		return Type_copy(type);
	}
	Subst *subs = refresh(GenType_inner(type), SUBST_EMPTY);
	Type *mono = substitute(GenType_inner(type), subs, SUB_ONELEVEL);
	Subst_drop(subs);
	return mono;
}

static Type *generalize(const Type *mono)
{
	VarType_reset();
	Subst *subs = refresh(mono, SUBST_EMPTY);
	Type *gen = GenType_new(substitute(mono, subs, SUB_ONELEVEL));
	Subst_drop(subs);
	return gen;
}

Subst *M(const Node *expr, TypeEnv *env, passed Subst *subs, const Type *target);

Subst *M_id(const Node *id, TypeEnv *env, passed Subst *subs, const Type *target)
{
	Type *id_type = TypeEnv_lookup(env, IdNode_value(id));
	if (!id_type) {
		errorf("unbound variable: %s", IdNode_value(id));
		Subst_drop(subs);
		return NULL;
	}
	id_type = instantiate(id_type);
	subs = unify(target, id_type, subs);
	Type_drop(id_type);
	return subs;
}

Subst *M_neg(const Node *neg, TypeEnv *env, passed Subst *subs, const Type *target)
{
	subs = unify(target, NumType_get(), subs);
	if (!subs) {
		return NULL;
	}
	return M(NegNode_value(neg), env, subs, target);
}

Subst *M_if(const Node *ifelse, TypeEnv *env, passed Subst *subs, const Type *target)
{
	subs = M(IfNode_cond(ifelse), env, subs, NumType_get());
	if (!subs) {
		return NULL;
	}
	subs = M(IfNode_true(ifelse), env, subs, target);
	if (!subs) {
		return NULL;
	}
	return M(IfNode_false(ifelse), env, subs, target);
}

Subst *M_fn(const Node *fn, TypeEnv *env, passed Subst *subs, const Type *target)
{
	Type *arg_type = VarType_new();
	Type *body_type = VarType_new();
	Type *fn_type = FnType_new(arg_type, body_type);
	subs = unify(target, fn_type, subs);
	if (!subs) {
		Type_drop(fn_type);
		return NULL;
	}
	TypeEnv *extended = TypeEnv_push(FnNode_param_value(fn), arg_type, env);
	subs = M(FnNode_body(fn), extended, subs, body_type);
	TypeEnv_pop(extended);
	Type_drop(fn_type);
	return subs;
}

Subst *M_pair(const Node *pair, TypeEnv *env, passed Subst *subs, const Type *target)
{
	subs = unify(target, NumType_get(), subs);
	if (!subs) {
		return NULL;
	}
	subs = M(PairNode_left(pair), env, subs, target);
	if (!subs) {
		return NULL;
	}
	return M(PairNode_right(pair), env, subs, target);
}

Subst *M_application(const Node *appl, TypeEnv *env, passed Subst *subs, const Type *target)
{
	Type *operand_type = VarType_new();
	Type *operator_type = FnType_new(operand_type, Type_copy(target));
	subs = M(PairNode_left(appl), env, subs, operator_type);
	if (!subs) {
		Type_drop(operator_type);
		return NULL;
	}
	subs = M(PairNode_right(appl), env, subs, operand_type);
	Type_drop(operator_type);
	return subs;
}

Subst *M_let(const Node *let, TypeEnv *env, passed Subst *subs, const Type *target)
{
	TypeEnv *extended = TypeEnv_push(LetNode_name_value(let), target, env);
	subs = M(LetNode_value(let), extended, subs, target);
	TypeEnv_pop(extended);
	return subs;
}

Subst *M(const Node *expr, TypeEnv *env, passed Subst *subs, const Type *target)
{
	switch (expr->type) {
		case NUMBER_NODE:
			return unify(target, NumType_get(), subs);
		case ID_NODE:
			return M_id(expr, env, subs, target);
		case NEG_NODE:
			return M_neg(expr, env, subs, target);
		case IF_NODE:
			return M_if(expr, env, subs, target);
		case FN_NODE:
			return M_fn(expr, env, subs, target);
		case SUM_NODE:
		case PRODUCT_NODE:
		case EXPT_NODE:
		case CMP_NODE:
		case AND_NODE:
		case OR_NODE:
			return M_pair(expr, env, subs, target);
		case APPLICATION_NODE:
			return M_application(expr, env, subs, target);
		case LET_NODE:
			return M_let(expr, env, subs, target);
	}
	return NULL;
}

Type *infer(const Node *expr, Context *ctx)
{
	Type *target = VarType_new();
	Subst *subs = M(expr, ctx->tenv, SUBST_EMPTY, target);
	if (!subs) {
		Type_drop(target);
		return NULL;
	}
	Type *mono = substitute(target, subs, SUB_RECURSIVE);
	Type_drop(target);
	Subst_drop(subs);
	Type *poly = generalize(mono);
	Type_drop(mono);
	if (expr->type == LET_NODE && poly) {
		const char *name = LetNode_name_value(expr);
		Type *old = TypeEnv_lookup(ctx->tenv, name);
		if (!old) {
			ctx->tenv = TypeEnv_push(name, poly, ctx->tenv);
		} else if (!Type_eq(old, poly)) {
			error("symbol type cannot change");
			Type_drop(poly);
			return NULL;
		}
	}
	return poly;
}
