#include "eval.h"

#include <math.h>
#include <string.h>

#include "opts.h"
#include "node.h"
#include "env.h"
#include "stack.h"
#include "gc.h"
#include "context.h"
#include "annotations.h"
#include "error.h"


#define ERROR_PREFIX "evaluation error"

#define EVAL_OK   1
#define EVAL_FAIL 0

static Object *eval_dispatch(passed Node *expr, Context *ctx, Object *env);
static Object *actual_value(passed Node *expr, Context *ctx, Object *env);

static Object *delay(passed Node *expr, Context *ctx, Object *env)
{
	return GC_alloc_thunk(ctx->gc, env, expr);
}

static inline Object *eval_expect(passed Node *expr, Context *ctx, Object *env, ObjectType type)
{
	Object *obj = actual_value(expr, ctx, env);
	if (!obj) {
		return NULL;
	}
	if (obj->type != type) {
		error("type mismatch");
		return NULL;
	}
	return obj;
}

static Object *eval_number(passed Node *expr, Context *ctx)
{
	Object *num = GC_alloc_number(ctx->gc, NumNode_value(expr));
	Node_drop(expr);
	return num;
}

static Object *eval_fn(passed Node *expr, Context *ctx, Object *env)
{
	Object *fn = GC_alloc_fn(ctx->gc, env, FnNode_body(expr), strdup(FnNode_param_value(expr)));
	Node_drop(FnNode_param(expr));
	Node_drop_one(expr);
	return fn;
}

static Object *eval_lookup(passed Node *id, Object *env)
{
	Object *value = Env_get(EnvObj_env(env), IdNode_value(id));
	if (!value) {
		errorf("unbound variable: %s", IdNode_value(id));
		Node_drop(id);
		return NULL;
	}
	Node_drop(id);
	return value;
}

static Object *eval_neg(passed Node *neg, Context *ctx, Object *env)
{
	Object *value = eval_expect(NegNode_value(neg), ctx, env, NUM_OBJECT);
	Node_drop_one(neg);
	if (!value) {
		return NULL;
	}
	return GC_alloc_number(ctx->gc, NumObj_num(value) * -1);
}

static Object *eval_pair(passed Node *pair, Context *ctx, Object *env)
{
	int op = PairNode_op(pair);
	Context_stack_push(ctx, env);
	Object *leftv = eval_expect(PairNode_left(pair), ctx, env, NUM_OBJECT);
	if (!leftv) {
		Node_drop(PairNode_right(pair));
		Node_drop_one(pair);
		return NULL;
	}
	Context_stack_pop(ctx);
	Context_stack_push(ctx, leftv);
	Object *rightv = eval_expect(PairNode_right(pair), ctx, env, NUM_OBJECT);
	Node_drop_one(pair);
	if (!rightv) {
		return NULL;
	}
	Context_stack_pop(ctx);
	switch (op) {
		case '^':
			return GC_alloc_number(ctx->gc, pow(NumObj_num(leftv), NumObj_num(rightv)));
		case '*':
			return GC_alloc_number(ctx->gc, NumObj_num(leftv) * NumObj_num(rightv));
		case '/':
			return GC_alloc_number(ctx->gc, NumObj_num(leftv) / NumObj_num(rightv));
		case '%':
			return GC_alloc_number(ctx->gc, fmod(NumObj_num(leftv), NumObj_num(rightv)));
		case '+':
			return GC_alloc_number(ctx->gc, NumObj_num(leftv) + NumObj_num(rightv));
		case '-':
			return GC_alloc_number(ctx->gc, NumObj_num(leftv) - NumObj_num(rightv));
		case '>':
			return GC_alloc_number(ctx->gc, NumObj_num(leftv) > NumObj_num(rightv));
		case '<':
			return GC_alloc_number(ctx->gc, NumObj_num(leftv) < NumObj_num(rightv));
		case '=':
			return GC_alloc_number(ctx->gc, NumObj_num(leftv) == NumObj_num(rightv));
		default:
			errorf("unknown binary operation: '%c'", op);
			return NULL;
	}
}

static Object *eval_or(passed Node *or, Context *ctx, Object *env)
{
	Object *leftv = eval_expect(PairNode_left(or), ctx, env, NUM_OBJECT);
	if (!leftv) {
		Node_drop(PairNode_right(or));
		Node_drop_one(or);
		return NULL;
	}
	if (NumObj_num(leftv)) {
		Node_drop(PairNode_right(or));
		Node_drop_one(or);
		return leftv;
	}
	Object *rightv = eval_expect(PairNode_right(or), ctx, env, NUM_OBJECT);
	Node_drop_one(or);
	if (!rightv) {
		return NULL;
	}
	return rightv;
}

static Object *eval_and(passed Node *and, Context *ctx, Object *env)
{
	Object *leftv = eval_expect(PairNode_left(and), ctx, env, NUM_OBJECT);
	if (!leftv) {
		Node_drop(PairNode_right(and));
		Node_drop_one(and);
		return NULL;
	}
	if (!NumObj_num(leftv)) {
		Node_drop(PairNode_right(and));
		Node_drop_one(and);
		return leftv;
	}
	Object *rightv = eval_expect(PairNode_right(and), ctx, env, NUM_OBJECT);
	Node_drop_one(and);
	if (!rightv) {
		return NULL;
	}
	return rightv;
}

static Object *eval_let(passed Node *let, Context *ctx, Object *env)
{
	Object *value = eval_dispatch(LetNode_value(let), ctx, env);
	if (!value) {
		Node_drop(LetNode_name(let));
		Node_drop_one(let);
		return NULL;
	}
	Env_add(EnvObj_env(env), LetNode_name_value(let), value);
	Node_drop(LetNode_name(let));
	Node_drop_one(let);
	return value;
}

static int eval_if(Context *ctx, Object **env, Node **expr)
{
	Node *ifn = *expr;
	Object *condv = eval_expect(IfNode_cond(ifn), ctx, *env, NUM_OBJECT);
	if (!condv) {
		Node_drop(IfNode_true(ifn));
		Node_drop(IfNode_false(ifn));
		Node_drop_one(ifn);
		return EVAL_FAIL;
	}
	if (NumObj_num(condv)) {
		*expr = IfNode_true(*expr);
		Node_drop(IfNode_false(ifn));
		Node_drop_one(ifn);
	} else {
		*expr = IfNode_false(*expr);
		Node_drop(IfNode_true(ifn));
		Node_drop_one(ifn);
	}
	return EVAL_OK;
}

static int eval_application(Context *ctx, Object **env, Node **expr)
{
	Node *appl = *expr;
	Context_stack_push(ctx, *env);
	Object *fnv = eval_expect(PairNode_left(appl), ctx, *env, FN_OBJECT);
	if (!fnv) {
		Node_drop(PairNode_right(appl));
		Node_drop_one(appl);
		return EVAL_FAIL;
	}
	Object *argv = NULL;
	if (lazy) {
		argv = delay(PairNode_right(appl), ctx, *env);
		Node_drop_one(appl);
	} else {
		Context_stack_push(ctx, fnv);
		argv = eval_dispatch(PairNode_right(appl), ctx, *env);
		Node_drop_one(appl);
		if (!argv) {
			return EVAL_FAIL;
		}
		Context_stack_pop(ctx);
	}
	Context_stack_pop(ctx);
	*env = GC_alloc_env(ctx->gc, FnObj_env(fnv));
	Env_add(EnvObj_env(*env), FnObj_arg(fnv), argv);
	*expr = Node_copy(FnObj_body(fnv));
	return EVAL_OK;
}

static Object *eval_dispatch(passed Node *expr, Context *ctx, Object *env)
{
	for (;;) {
		GC_collect(ctx->gc, env, ctx->stack);
		switch (expr->type) {
			case NUMBER_NODE:
				return eval_number(expr, ctx);
			case FN_NODE:
				return eval_fn(expr, ctx, env);
			case ID_NODE:
				return eval_lookup(expr, env);
			case NEG_NODE:
				return eval_neg(expr, ctx, env);
			case EXPT_NODE:
			case PRODUCT_NODE:
			case SUM_NODE:
			case CMP_NODE:
				return eval_pair(expr, ctx, env);
			case AND_NODE:
				return eval_and(expr, ctx, env);
			case OR_NODE:
				return eval_or(expr, ctx, env);
			case IF_NODE:
				if (eval_if(ctx, &env, &expr) != EVAL_OK) {
					return NULL;
				}
				break;
			case APPLICATION_NODE:
				if (eval_application(ctx, &env, &expr) != EVAL_OK) {
					return NULL;
				}
				break;
			case LET_NODE:
				return eval_let(expr, ctx, env);
		}
	}
}

static Object *force(Object *obj, Context *ctx)
{
	if (!obj || obj->type != THUNK_OBJECT) {
		return obj;
	}
	if (ThunkObj_value(obj)) {
		return ThunkObj_value(obj);
	}
	Context_stack_push(ctx, obj);
	Object *value = actual_value(ThunkObj_body(obj), ctx, ThunkObj_env(obj));
	ThunkObj_set_value(obj, value);
	if (!value) {
		return NULL;
	}
	Context_stack_pop(ctx);
	return value;
}

static Object *actual_value(passed Node *expr, Context *ctx, Object *env)
{
	return force(eval_dispatch(expr, ctx, env), ctx);
}

Object *eval(passed Node *expr, Context *ctx)
{
	Stack_clear(Context_stack(ctx));
	if (lazy) {
		return actual_value(expr, ctx, ctx->root);
	} else {
		return eval_dispatch(expr, ctx, ctx->root);
	}
}
