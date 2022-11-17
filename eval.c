#include "eval.h"

#include "node.h"


static int eval_sum(Node *expr, int *result)
{
	int left_value, right_value;
	if (!eval(expr->value.pair.left, &left_value)) {
		return 0;
	}
	if (!eval(expr->value.pair.right, &right_value)) {
		return 0;
	}
	*result = left_value + right_value;
	return 1;
}

static int eval_product(Node *expr, int *result)
{
	int left_value, right_value;
	if (!eval(expr->value.pair.left, &left_value)) {
		return 0;
	}
	if (!eval(expr->value.pair.right, &right_value)) {
		return 0;
	}
	*result = left_value * right_value;
	return 1;
}

static int eval_expt(Node *expr, int *result)
{
	int left_value, right_value;
	if (!eval(expr->value.pair.left, &left_value)) {
		return 0;
	}
	if (!eval(expr->value.pair.right, &right_value)) {
		return 0;
	}
	*result = 1;
	while (right_value > 0) {
		*result *= left_value;
		right_value -= 1;
	}
	return 1;
}

int eval(Node *expr, int *result)
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
