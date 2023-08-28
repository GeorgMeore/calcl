#include "eval.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "node.h"
#include "env.h"
#include "gc.h"


#define error(message) \
	(fprintf(stderr, "evaluation error: " message "\n"))

#define errorf(fmt, args...) \
	(fprintf(stderr, "evaluation error: " fmt "\n", args))

static inline Object *eval_expect(Node *expr, GC *gc, Object *env, ObjectType type)
{
	Object *obj = seval(expr, gc, env);
	if (!obj) {
		return NULL;
	}
	if (obj->type != type) {
		error("type mismatch");
		return NULL;
	}
	return obj;
}

static Object *eval_neg(Node *expr, GC *gc, Object *env)
{
	Object *value = eval_expect(expr, gc, env, NUM_OBJECT);
	if (!value) {
		return NULL;
	}
	value->as.num *= -1;
	return value;
}

static Object *eval_expt(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, pow(leftv->as.num, rightv->as.num));
}

static Object *eval_product(Node *left, Node *right, int op, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	if (op == '*') {
		return GC_alloc_number(gc, leftv->as.num * rightv->as.num);
	}
	return GC_alloc_number(gc, leftv->as.num / rightv->as.num);
}

static Object *eval_sum(Node *left, Node *right, int op, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	if (op == '+') {
		return GC_alloc_number(gc, leftv->as.num + rightv->as.num);
	}
	return GC_alloc_number(gc, leftv->as.num - rightv->as.num);
}

static Object *eval_cmp(Node *left, Node *right, int op, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	if (op == '>') {
		return GC_alloc_number(gc, leftv->as.num > rightv->as.num);
	} else if (op == '<') {
		return GC_alloc_number(gc, leftv->as.num < rightv->as.num);
	}
	return GC_alloc_number(gc, leftv->as.num == rightv->as.num);
}

static Object *eval_or(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (leftv->as.num) {
		return GC_alloc_number(gc, leftv->as.num);
	}
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, rightv->as.num);
}

static Object *eval_and(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (!leftv->as.num) {
		return GC_alloc_number(gc, leftv->as.num);
	}
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, rightv->as.num);
}

static Object *eval_if(Node *cond, Node *true, Node *false, GC *gc, Object *env)
{
	Object *condv = eval_expect(cond, gc, env, NUM_OBJECT);
	if (!condv) {
		return NULL;
	}
	if (condv->as.num) {
		return seval(true, gc, env);
	} else {
		return seval(false, gc, env);
	}
}

static Object *eval_application(Node *fn, Node *arg, GC *gc, Object *env)
{
	Object *fnv = eval_expect(fn, gc, env, FN_OBJECT);
	if (!fnv) {
		return NULL;
	}
	Object *argv = seval(arg, gc, env);
	if (!argv) {
		return NULL;
	}
	Object *extended = GC_alloc_env(gc, fnv->as.fn.env);
	Env_add(extended->as.env, fnv->as.fn.arg, argv);
	return seval(fnv->as.fn.body, gc, extended);
}

static Object *eval_let(Node *name, Node *expr, GC *gc, Object *env)
{
	Object *value = seval(expr, gc, env);
	if (!value) {
		return NULL;
	}
	Env_add(env->as.env, name->as.id, value);
	return NULL;
}

static Object *eval_lookup(Node *id, Object *env)
{
	Object *value = Env_get(env->as.env, id->as.id);
	if (!value) {
		errorf("unbound variable: %s", id->as.id);
		return NULL;
	}
	return value;
}

Object *seval(Node *expr, GC *gc, Object *env)
{
	switch (expr->type) {
		case NUMBER_NODE:
			return GC_alloc_number(gc, expr->as.number);
		case ID_NODE:
			return eval_lookup(expr, env);
		case NEG_NODE:
			return eval_neg(expr->as.neg, gc, env);
		case EXPT_NODE:
			return eval_expt(expr->as.pair.left, expr->as.pair.right, gc, env);
		case PRODUCT_NODE:
			return eval_product(expr->as.pair.left, expr->as.pair.right, expr->as.pair.op, gc, env);
		case SUM_NODE:
			return eval_sum(expr->as.pair.left, expr->as.pair.right, expr->as.pair.op, gc, env);
		case CMP_NODE:
			return eval_cmp(expr->as.pair.left, expr->as.pair.right, expr->as.pair.op, gc, env);
		case AND_NODE:
			return eval_and(expr->as.pair.left, expr->as.pair.right, gc, env);
		case OR_NODE:
			return eval_or(expr->as.pair.left, expr->as.pair.right, gc, env);
		case IF_NODE:
			return eval_if(expr->as.ifelse.cond, expr->as.ifelse.true, expr->as.ifelse.false, gc, env);
		case FN_NODE:
			return GC_alloc_fn(gc, env, Node_copy(expr->as.fn.body), strdup(expr->as.fn.param->as.id));
		case APPLICATION_NODE:
			return eval_application(expr->as.pair.left, expr->as.pair.right, gc, env);
		case LET_NODE:
			return eval_let(expr->as.let.name, expr->as.let.value, gc, env);
	}
	return NULL;
}

static Object *delay(Node *expr, GC *gc, Object *env)
{
	return GC_alloc_thunk(gc, env, Node_copy(expr));
}

static Object *force_expect(Node *expr, GC *gc, Object *env, ObjectType type)
{
	Object *obj = leval(expr, gc, env);
	if (!obj) {
		return NULL;
	}
	if (obj->type != type) {
		error("type mismatch");
		return NULL;
	}
	return obj;
}

static Object *force_lookup(Node *id, Object *env)
{
	Object *value = Env_get(env->as.env, id->as.id);
	if (!value) {
		error("unbound variable");
		return NULL;
	}
	return value;
}

static Object *force_neg(Node *expr, GC *gc, Object *env)
{
	Object *value = force_expect(expr, gc, env, NUM_OBJECT);
	if (!value) {
		return NULL;
	}
	value->as.num *= -1;
	return value;
}

static Object *force_expt(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = force_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = force_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, pow(leftv->as.num, rightv->as.num));
}

static Object *force_product(Node *left, Node *right, int op, GC *gc, Object *env)
{
	Object *leftv = force_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = force_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	if (op == '*') {
		return GC_alloc_number(gc, leftv->as.num * rightv->as.num);
	}
	return GC_alloc_number(gc, leftv->as.num / rightv->as.num);
}

static Object *force_sum(Node *left, Node *right, int op, GC *gc, Object *env)
{
	Object *leftv = force_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = force_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	if (op == '+') {
		return GC_alloc_number(gc, leftv->as.num + rightv->as.num);
	}
	return GC_alloc_number(gc, leftv->as.num - rightv->as.num);
}

static Object *force_cmp(Node *left, Node *right, int op, GC *gc, Object *env)
{
	Object *leftv = force_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = force_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	if (op == '>') {
		return GC_alloc_number(gc, leftv->as.num > rightv->as.num);
	} else if (op == '<') {
		return GC_alloc_number(gc, leftv->as.num < rightv->as.num);
	}
	return GC_alloc_number(gc, leftv->as.num == rightv->as.num);
}

static Object *force_or(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = force_expect(left, gc, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (leftv->as.num) {
		return GC_alloc_number(gc, leftv->as.num);
	}
	Object *rightv = force_expect(right, gc, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, rightv->as.num);
}

static Object *force_and(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = force_expect(left, gc, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (!leftv->as.num) {
		return GC_alloc_number(gc, leftv->as.num);
	}
	Object *rightv = force_expect(right, gc, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, rightv->as.num);
}

static Object *force_if(Node *cond, Node *true, Node *false, GC *gc, Object *env)
{
	Object *condv = force_expect(cond, gc, env, NUM_OBJECT);
	if (!condv) {
		return NULL;
	}
	if (condv->as.num) {
		return delay(true, gc, env);
	} else {
		return delay(false, gc, env);
	}
}

static Object *force_application(Node *fn, Node *arg, GC *gc, Object *env)
{
	Object *fnv = force_expect(fn, gc, env, FN_OBJECT);
	if (!fnv) {
		return NULL;
	}
	Object *extended = GC_alloc_env(gc, fnv->as.fn.env);
	Env_add(extended->as.env, fnv->as.fn.arg, delay(arg, gc, env));
	return delay(fnv->as.fn.body, gc, extended);
}

static Object *force_let(Node *name, Node *expr, GC *gc, Object *env)
{
	Env_add(env->as.env, name->as.id, delay(expr, gc, env));
	return NULL;
}

static Object *force_dispatch(Node *expr, GC *gc, Object *env)
{
	switch (expr->type) {
	case NUMBER_NODE:
		return GC_alloc_number(gc, expr->as.number);
	case ID_NODE:
		return force_lookup(expr, env);
	case NEG_NODE:
		return force_neg(expr->as.neg, gc, env);
	case EXPT_NODE:
		return force_expt(expr->as.pair.left, expr->as.pair.right, gc, env);
	case PRODUCT_NODE:
		return force_product(expr->as.pair.left, expr->as.pair.right, expr->as.pair.op, gc, env);
	case SUM_NODE:
		return force_sum(expr->as.pair.left, expr->as.pair.right, expr->as.pair.op, gc, env);
	case CMP_NODE:
		return force_cmp(expr->as.pair.left, expr->as.pair.right, expr->as.pair.op, gc, env);
	case AND_NODE:
		return force_and(expr->as.pair.left, expr->as.pair.right, gc, env);
	case OR_NODE:
		return force_or(expr->as.pair.left, expr->as.pair.right, gc, env);
	case IF_NODE:
		return force_if(expr->as.ifelse.cond, expr->as.ifelse.true, expr->as.ifelse.false, gc, env);
	case FN_NODE:
		return GC_alloc_fn(gc, env, Node_copy(expr->as.fn.body), strdup(expr->as.fn.param->as.id));
	case APPLICATION_NODE:
		return force_application(expr->as.pair.left, expr->as.pair.right, gc, env);
	case LET_NODE:
		return force_let(expr->as.let.name, expr->as.let.value, gc, env);
	}
	return NULL;
}

static Object *force(Object *obj, GC *gc)
{
	if (!obj || obj->type != THUNK_OBJECT) {
		return obj;
	}
	if (obj->as.thunk.value) {
		return obj->as.thunk.value;
	}
	Object *value = leval(obj->as.thunk.body, gc, obj->as.thunk.env);
	if (!value) {
		return NULL;
	}
	Node_drop(obj->as.thunk.body);
	obj->as.thunk.body = NULL;
	obj->as.thunk.env = NULL;
	obj->as.thunk.value = value;
	return value;
}

Object *leval(Node *expr, GC *gc, Object *env)
{
	return force(force_dispatch(expr, gc, env), gc);
}
