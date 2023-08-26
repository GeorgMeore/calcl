#include "eval.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "node.h"
#include "env.h"
#include "gc.h"


static void error(const char *message)
{
	fprintf(stderr, "evaluation error: %s", message);
}

static inline Object *eval_expect(Node *expr, GC *gc, Object *env, ObjectType type)
{
	Object *obj = eval(expr, gc, env);
	if (!obj) {
		return NULL;
	}
	if (obj->type != type) {
		error("Type mismatch");
		return NULL;
	}
	return obj;
}

static Object *eval_sum(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, leftv->as.num + rightv->as.num);
}

static Object *eval_product(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, leftv->as.num * rightv->as.num);
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

static Object *eval_cmp(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = eval_expect(left, gc, env, NUM_OBJECT);
	Object *rightv = eval_expect(right, gc, env, NUM_OBJECT);
	if (!leftv || !rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, leftv->as.num > rightv->as.num);
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
		return eval(true, gc, env);
	} else {
		return eval(false, gc, env);
	}
}

static Object *eval_application(Node *fn, Node *arg, GC *gc, Object *env)
{
	Object *fnv = eval_expect(fn, gc, env, FN_OBJECT);
	if (!fn) {
		return NULL;
	}
	Object *argv = eval(arg, gc, env);
	if (!arg) {
		return NULL;
	}
	Object *extended = GC_alloc_env(gc, fnv->as.fn.env);
	Env_add(extended->as.env, fnv->as.fn.arg, argv);
	return eval(fnv->as.fn.body, gc, extended);
}

Object *eval(Node *expr, GC *gc, Object *env)
{
	switch (expr->type) {
		case NUMBER_NODE:
			return GC_alloc_number(gc, expr->as.number);
		case ID_NODE:
			return Env_get(env->as.env, expr->as.id);
		case SUM_NODE:
			return eval_sum(expr->as.pair.left, expr->as.pair.right, gc, env);
		case PRODUCT_NODE:
			return eval_product(expr->as.pair.left, expr->as.pair.right, gc, env);
		case EXPT_NODE:
			return eval_expt(expr->as.pair.left, expr->as.pair.right, gc, env);
		case CMP_NODE:
			return eval_cmp(expr->as.pair.left, expr->as.pair.right, gc, env);
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
	}
	return NULL;
}
