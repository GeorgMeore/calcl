#include "eval.h"

#include <math.h>
#include <string.h>

#include "opts.h"
#include "node.h"
#include "gc.h"
#include "values.h"
#include "env.h"
#include "stack.h"
#include "context.h"
#include "error.h"


#define ERROR_PREFIX "evaluation error"

static Object *eval_dispatch(const Node *expr, Context *ctx, Object *env);
static Object *actual_value(const Node *expr, Context *ctx, Object *env);

static inline Object *eval_expect(const Node *expr, Context *ctx, Object *env, ObjectType type)
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

static Object *eval_lookup(const Node *expr, Object *env)
{
	Object *value = Env_get(EnvObj_env(env), IdNode_value(expr));
	if (!value) {
		errorf("unbound variable: %s", IdNode_value(expr));
		return NULL;
	}
	return value;
}

static Object *eval_pair(const Node *expr, Context *ctx, Object *env)
{
	int op = PairNode_op(expr);
	Context_stack_push(ctx, env);
	Object *leftv = eval_expect(PairNode_left(expr), ctx, env, NumObject);
	Context_stack_pop(ctx);
	if (!leftv) {
		return NULL;
	}
	Context_stack_push(ctx, leftv);
	Object *rightv = eval_expect(PairNode_right(expr), ctx, env, NumObject);
	Context_stack_pop(ctx);
	if (!rightv) {
		return NULL;
	}
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

static Object *eval_or(const Node *expr, Context *ctx, Object *env)
{
	Context_stack_push(ctx, env);
	Object *leftv = eval_expect(PairNode_left(expr), ctx, env, NumObject);
	Context_stack_pop(ctx);
	if (!leftv) {
		return NULL;
	}
	if (NumObj_num(leftv)) {
		return leftv;
	}
	Object *rightv = eval_expect(PairNode_right(expr), ctx, env, NumObject);
	if (!rightv) {
		return NULL;
	}
	return rightv;
}

static Object *eval_and(const Node *expr, Context *ctx, Object *env)
{
	Context_stack_push(ctx, env);
	Object *leftv = eval_expect(PairNode_left(expr), ctx, env, NumObject);
	Context_stack_pop(ctx);
	if (!leftv) {
		return NULL;
	}
	if (!NumObj_num(leftv)) {
		return leftv;
	}
	Object *rightv = eval_expect(PairNode_right(expr), ctx, env, NumObject);
	if (!rightv) {
		return NULL;
	}
	return rightv;
}

static Object *eval_let(const Node *expr, Context *ctx, Object *env)
{
	Object *value = eval_dispatch(LetNode_value(expr), ctx, env);
	if (!value) {
		return NULL;
	}
	Env_add(EnvObj_env(env), LetNode_name_value(expr), value);
	return NULL;
}

static Node *eval_if(Context *ctx, Object **env, const Node *expr)
{
	Context_stack_push(ctx, *env);
	Object *condv = eval_expect(IfNode_cond(expr), ctx, *env, NumObject);
	Context_stack_pop(ctx);
	if (!condv) {
		return NULL;
	}
	if (NumObj_num(condv)) {
		return IfNode_true(expr);
	} else {
		return IfNode_false(expr);
	}
}

static const Node *eval_application(Context *ctx, Object **env, const Node *expr)
{
	Context_stack_push(ctx, *env);
	Object *fnv = eval_expect(PairNode_left(expr), ctx, *env, FnObject);
	Context_stack_pop(ctx);
	if (!fnv) {
		return NULL;
	}
	Object *argv = NULL;
	if (lazy) {
		argv = GC_alloc_thunk(ctx->gc, *env, PairNode_right(expr));
	} else {
		Context_stack_push(ctx, fnv);
		argv = eval_dispatch(PairNode_right(expr), ctx, *env);
		Context_stack_pop(ctx);
		if (!argv) {
			return NULL;
		}
	}
	*env = GC_alloc_env(ctx->gc, FnObj_env(fnv));
	Env_add(EnvObj_env(*env), FnObj_arg(fnv), argv);
	return FnObj_body(fnv);
}

static Object *eval_dispatch(const Node *expr, Context *ctx, Object *env)
{
	for (;;) {
		GC_collect(ctx->gc, env, ctx->stack);
		switch (expr->type) {
			case NumberNode:
				return GC_alloc_number(ctx->gc, NumNode_value(expr));
			case FnNode:
				return GC_alloc_fn(ctx->gc, env, FnNode_body(expr), FnNode_param_value(expr));
			case IdNode:
				return eval_lookup(expr, env);
			case ExptNode:
			case ProdNode:
			case SumNode:
			case CmpNode:
				return eval_pair(expr, ctx, env);
			case AndNode:
				return eval_and(expr, ctx, env);
			case OrNode:
				return eval_or(expr, ctx, env);
			case IfNode:
				expr = eval_if(ctx, &env, expr);
				break;
			case ApplNode:
				expr = eval_application(ctx, &env, expr);
				break;
			case LetNode:
				return eval_let(expr, ctx, env);
		}
		if (!expr) {
			return NULL;
		}
	}
}

static Object *actual_value(const Node *expr, Context *ctx, Object *env)
{
	Object *result = eval_dispatch(expr, ctx, env);
	if (!result || result->type != ThunkObject) {
		return result;
	}
	if (ThunkObj_value(result)) {
		return ThunkObj_value(result);
	}
	Context_stack_push(ctx, result);
	Object *value = actual_value(ThunkObj_body(result), ctx, ThunkObj_env(result));
	Context_stack_pop(ctx);
	ThunkObj_set_value(result, value);
	return value;
}

Object *eval(const Node *expr, Context *ctx)
{
	Stack_clear(Context_stack(ctx));
	if (lazy) {
		return actual_value(expr, ctx, ctx->root);
	} else {
		return eval_dispatch(expr, ctx, ctx->root);
	}
}
