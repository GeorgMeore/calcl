#include "eval.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "node.h"
#include "env.h"
#include "stack.h"
#include "gc.h"
#include "context.h"


#define EVAL_OK 1
#define EVAL_FAIL 0

#define error(message) \
	(fprintf(stderr, "evaluation error: " message "\n"))

#define errorf(fmt, args...) \
	(fprintf(stderr, "evaluation error: " fmt "\n", args))

Object *seval_dispatch(Node *expr, Context *ctx, Object *env);

static inline Object *seval_expect(Node *expr, Context *ctx, Object *env, ObjectType type)
{
	Object *obj = seval_dispatch(expr, ctx, env);
	if (!obj) {
		return NULL;
	}
	if (obj->type != type) {
		error("type mismatch");
		Stack_clear(Context_stack(ctx));
		return NULL;
	}
	return obj;
}

static Object *seval_neg(Node *expr, Context *ctx, Object *env)
{
	Object *value = seval_expect(expr, ctx, env, NUM_OBJECT);
	if (!value) {
		return NULL;
	}
	return GC_alloc_number(ctx->gc, value->as.num * -1);
}

static Object *seval_pair(Node *left, Node *right, Context *ctx, Object *env, int op)
{
	Context_stack_push(ctx, env);
	Object *leftv = seval_expect(left, ctx, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	Context_stack_pop(ctx);
	Context_stack_push(ctx, leftv);
	Object *rightv = seval_expect(right, ctx, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	Context_stack_pop(ctx);
	switch (op) {
		case '^':
			return GC_alloc_number(ctx->gc, pow(leftv->as.num, rightv->as.num));
		case '*':
			return GC_alloc_number(ctx->gc, leftv->as.num * rightv->as.num);
		case '/':
			return GC_alloc_number(ctx->gc, leftv->as.num / rightv->as.num);
		case '+':
			return GC_alloc_number(ctx->gc, leftv->as.num + rightv->as.num);
		case '-':
			return GC_alloc_number(ctx->gc, leftv->as.num - rightv->as.num);
		case '>':
			return GC_alloc_number(ctx->gc, leftv->as.num > rightv->as.num);
		case '<':
			return GC_alloc_number(ctx->gc, leftv->as.num < rightv->as.num);
		case '=':
			return GC_alloc_number(ctx->gc, leftv->as.num = rightv->as.num);
		default:
			errorf("unknown binary operation: '%c'", op);
			Stack_clear(Context_stack(ctx));
			return NULL;
	}
}

static Object *seval_or(Node *left, Node *right, Context *ctx, Object *env)
{
	Object *leftv = seval_expect(left, ctx, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (leftv->as.num) {
		return leftv;
	}
	Object *rightv = seval_expect(right, ctx, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return rightv;
}

static Object *seval_and(Node *left, Node *right, Context *ctx, Object *env)
{
	Object *leftv = seval_expect(left, ctx, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (!leftv->as.num) {
		return leftv;
	}
	Object *rightv = seval_expect(right, ctx, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return rightv;
}

static Object *seval_let(Node *name, Node *expr, Context *ctx, Object *env)
{
	Object *value = seval_dispatch(expr, ctx, env);
	if (!value) {
		return NULL;
	}
	Env_add(env->as.env, IdNode_value(name), value);
	return NULL;
}

static Object *seval_lookup(Node *id, Context *ctx, Object *env)
{
	Object *value = Env_get(env->as.env, IdNode_value(id));
	if (!value) {
		errorf("unbound variable: %s", IdNode_value(id));
		Stack_clear(Context_stack(ctx));
		return NULL;
	}
	return value;
}

static int seval_if(Context *ctx, Object **env, Node **expr)
{
	Object *condv = seval_expect(IfNode_cond(*expr), ctx, *env, NUM_OBJECT);
	if (!condv) {
		return EVAL_FAIL;
	}
	if (condv->as.num) {
		*expr = IfNode_true(*expr);
	} else {
		*expr = IfNode_false(*expr);
	}
	return EVAL_OK;
}

static int seval_application(Context *ctx, Object **env, Node **expr)
{
	Context_stack_push(ctx, *env);
	Object *fnv = seval_expect(PairNode_left(*expr), ctx, *env, FN_OBJECT);
	if (!fnv) {
		return EVAL_FAIL;
	}
	Context_stack_pop(ctx);
	Context_stack_push(ctx, *env);
	Context_stack_push(ctx, fnv);
	Object *argv = seval_dispatch(PairNode_right(*expr), ctx, *env);
	if (!argv) {
		return EVAL_FAIL;
	}
	Context_stack_pop(ctx);
	Context_stack_pop(ctx);
	*env = GC_alloc_env(ctx->gc, fnv->as.fn.env);
	Env_add((*env)->as.env, fnv->as.fn.arg, argv);
	*expr = fnv->as.fn.body; // FIXME: use after free
	return EVAL_OK;
}

Object *seval_dispatch(Node *expr, Context *ctx, Object *env)
{
	for (;;) {
		GC_collect(ctx->gc, env, ctx->stack);
		switch (expr->type) {
			case NUMBER_NODE:
				return GC_alloc_number(ctx->gc, NumNode_value(expr));
			case FN_NODE:
				return GC_alloc_fn(ctx->gc, env, Node_copy(FnNode_body(expr)), strdup(FnNode_param_value(expr)));
			case ID_NODE:
				return seval_lookup(expr, ctx, env);
			case NEG_NODE:
				return seval_neg(NegNode_value(expr), ctx, env);
			case EXPT_NODE:
				return seval_pair(PairNode_left(expr), PairNode_right(expr), ctx, env, '^');
			case PRODUCT_NODE:
			case SUM_NODE:
			case CMP_NODE:
				return seval_pair(PairNode_left(expr), PairNode_right(expr), ctx, env, PairNode_op(expr));
			case AND_NODE:
				return seval_and(PairNode_left(expr), PairNode_right(expr), ctx, env);
			case OR_NODE:
				return seval_or(PairNode_left(expr), PairNode_right(expr), ctx, env);
			case IF_NODE:
				if (seval_if(ctx, &env, &expr) != EVAL_OK) {
					return NULL;
				}
				break;
			case APPLICATION_NODE:
				if (seval_application(ctx, &env, &expr) != EVAL_OK) {
					return NULL;
				}
				break;
			case LET_NODE:
				return seval_let(LetNode_name(expr), LetNode_value(expr), ctx, env);
		}
	}
}

Object *seval(Node *expr, Context *ctx)
{
	return seval_dispatch(expr, ctx, ctx->root);
}

static Object *delay(Node *expr, GC *gc, Object *env)
{
	return GC_alloc_thunk(gc, env, Node_copy(expr));
}

static Object *leval_expect(Node *expr, GC *gc, Object *env, ObjectType type)
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

static Object *leval_lookup(Node *id, Object *env)
{
	Object *value = Env_get(env->as.env, id->as.id);
	if (!value) {
		error("unbound variable");
		return NULL;
	}
	return value;
}

static Object *leval_neg(Node *expr, GC *gc, Object *env)
{
	Object *value = leval_expect(expr, gc, env, NUM_OBJECT);
	if (!value) {
		return NULL;
	}
	value->as.num *= -1;
	return value;
}

static Object *leval_pair(Node *left, Node *right, GC *gc, Object *env, int op)
{
	Object *leftv = leval_expect(left, gc, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	Object *rightv = leval_expect(right, gc, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	switch (op) {
		case '^':
			return GC_alloc_number(gc, pow(leftv->as.num, rightv->as.num));
		case '*':
			return GC_alloc_number(gc, leftv->as.num * rightv->as.num);
		case '/':
			return GC_alloc_number(gc, leftv->as.num / rightv->as.num);
		case '+':
			return GC_alloc_number(gc, leftv->as.num + rightv->as.num);
		case '-':
			return GC_alloc_number(gc, leftv->as.num - rightv->as.num);
		case '>':
			return GC_alloc_number(gc, leftv->as.num > rightv->as.num);
		case '<':
			return GC_alloc_number(gc, leftv->as.num < rightv->as.num);
		case '=':
			return GC_alloc_number(gc, leftv->as.num = rightv->as.num);
		default:
			errorf("unknown binary operation: '%c'", op);
			return NULL;
	}
}

static Object *leval_or(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = leval_expect(left, gc, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (leftv->as.num) {
		return GC_alloc_number(gc, leftv->as.num);
	}
	Object *rightv = leval_expect(right, gc, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, rightv->as.num);
}

static Object *leval_and(Node *left, Node *right, GC *gc, Object *env)
{
	Object *leftv = leval_expect(left, gc, env, NUM_OBJECT);
	if (!leftv) {
		return NULL;
	}
	if (!leftv->as.num) {
		return GC_alloc_number(gc, leftv->as.num);
	}
	Object *rightv = leval_expect(right, gc, env, NUM_OBJECT);
	if (!rightv) {
		return NULL;
	}
	return GC_alloc_number(gc, rightv->as.num);
}

static Object *leval_if(Node *cond, Node *true, Node *false, GC *gc, Object *env)
{
	Object *condv = leval_expect(cond, gc, env, NUM_OBJECT);
	if (!condv) {
		return NULL;
	}
	if (condv->as.num) {
		return delay(true, gc, env);
	} else {
		return delay(false, gc, env);
	}
}

static Object *leval_application(Node *fn, Node *arg, GC *gc, Object *env)
{
	Object *fnv = leval_expect(fn, gc, env, FN_OBJECT);
	if (!fnv) {
		return NULL;
	}
	Object *extended = GC_alloc_env(gc, fnv->as.fn.env);
	Env_add(extended->as.env, fnv->as.fn.arg, delay(arg, gc, env));
	return delay(fnv->as.fn.body, gc, extended);
}

static Object *leval_let(Node *name, Node *expr, GC *gc, Object *env)
{
	Env_add(env->as.env, name->as.id, delay(expr, gc, env));
	return NULL;
}

static Object *leval_dispatch(Node *expr, GC *gc, Object *env)
{
	switch (expr->type) {
		case NUMBER_NODE:
			return GC_alloc_number(gc, NumNode_value(expr));
		case ID_NODE:
			return leval_lookup(expr, env);
		case NEG_NODE:
			return leval_neg(NegNode_value(expr), gc, env);
		case EXPT_NODE:
			return leval_pair(PairNode_left(expr), PairNode_right(expr), gc, env, '^');
		case PRODUCT_NODE:
		case SUM_NODE:
		case CMP_NODE:
			return leval_pair(PairNode_left(expr), PairNode_right(expr), gc, env, PairNode_op(expr));
		case AND_NODE:
			return leval_and(PairNode_left(expr), PairNode_right(expr), gc, env);
		case OR_NODE:
			return leval_or(PairNode_left(expr), PairNode_right(expr), gc, env);
		case IF_NODE:
			return leval_if(IfNode_cond(expr), IfNode_true(expr), IfNode_false(expr), gc, env);
		case FN_NODE:
			return GC_alloc_fn(gc, env, Node_copy(FnNode_body(expr)), strdup(FnNode_param_value(expr)));
		case APPLICATION_NODE:
			return leval_application(PairNode_left(expr), PairNode_right(expr), gc, env);
		case LET_NODE:
			return leval_let(LetNode_name(expr), LetNode_value(expr), gc, env);
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
	return force(leval_dispatch(expr, gc, env), gc);
}
