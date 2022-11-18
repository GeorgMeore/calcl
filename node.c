#include "node.h"

#include <stdlib.h>
#include <ctype.h>


void Node_drop(Node *node)
{
	switch (node->type) {
		case NUMBER_NODE:
			free(node);
			break;
		case SUM_NODE:
		case PRODUCT_NODE:
		case EXPT_NODE:
			Node_drop(node->value.pair.left);
			Node_drop(node->value.pair.right);
			free(node);
	}
}

static NumberValue number_value(const char *string, int length)
{
	double number = 0;
	int i;
	// whole part
	for (i = 0; i < length && isdigit(string[i]); i++) {
		number = (string[i] - '0') + number * 10;
	}
	if (string[i] == '.') {
		i++;
	}
	// fractional part
	for (double factor = 0.1; i < length; i++, factor/=10) {
		number += (string[i] - '0') * factor;
	}
	return number;
}

Node *NumberNode_new(const char *string, int length)
{
	Node *node = malloc(sizeof(*node));
	node->type = NUMBER_NODE;
	node->value.number = number_value(string, length);
	return node;
}

Node *SumNode_new(Node *left, Node *right)
{
	Node *node = malloc(sizeof(*node));
	node->type = SUM_NODE;
	node->value.pair.left = left;
	node->value.pair.right = right;
	return node;
}

Node *ProductNode_new(Node *left, Node *right)
{
	Node *node = malloc(sizeof(*node));
	node->type = PRODUCT_NODE;
	node->value.pair.left = left;
	node->value.pair.right = right;
	return node;
}

Node *ExptNode_new(Node *base, Node *exponent)
{
	Node *node = malloc(sizeof(*node));
	node->type = EXPT_NODE;
	node->value.pair.left = base;
	node->value.pair.right = exponent;
	return node;
}
