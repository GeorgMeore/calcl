#include "eval.h"

static int sum(Node *expr, int *result)
{
	int left_value, right_value;
	if (!evaluate(expr->value.pair.left, &left_value)) {
		return 0;
	}
	if (!evaluate(expr->value.pair.right, &right_value)) {
		return 0;
	}
	*result = left_value + right_value;
	return 1;
}

static int product(Node *expr, int *result)
{
	int left_value, right_value;
	if (!evaluate(expr->value.pair.left, &left_value)) {
		return 0;
	}
	if (!evaluate(expr->value.pair.right, &right_value)) {
		return 0;
	}
	*result = left_value * right_value;
	return 1;
}

int evaluate(Node *expr, int *result)
{
	switch (expr->type) {
		case NUMBER_NODE:
			*result = expr->value.number;
			return 1;
		case SUM_NODE:
			return sum(expr, result);
		case PRODUCT_NODE:
			return product(expr, result);
		default:
			return 0;
	}
}
