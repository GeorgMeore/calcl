#include "eval.h"

#include <math.h>

#include "node.h"


static int eval_sum(Node *expr, double *result)
{
	double left, right;
	if (!eval(expr->value.pair.left, &left)) {
		return 0;
	}
	if (!eval(expr->value.pair.right, &right)) {
		return 0;
	}
	*result = left + right;
	return 1;
}

static int eval_product(Node *expr, double *result)
{
	double left, right;
	if (!eval(expr->value.pair.left, &left)) {
		return 0;
	}
	if (!eval(expr->value.pair.right, &right)) {
		return 0;
	}
	*result = left * right;
	return 1;
}

static int eval_expt(Node *expr, double *result)
{
	double left, right;
	if (!eval(expr->value.pair.left, &left)) {
		return 0;
	}
	if (!eval(expr->value.pair.right, &right)) {
		return 0;
	}
	*result = pow(left, right);
	return 1;
}

int eval(Node *expr, double *result)
{
	switch (expr->type) {
		case NUMBER_NODE:
			*result = expr->value.number;
			return 1;
		case SUM_NODE:
			return eval_sum(expr, result);
		case PRODUCT_NODE:
			return eval_product(expr, result);
		case EXPT_NODE:
			return eval_expt(expr, result);
	}
	return 0;
}
