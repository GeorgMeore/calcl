#include "node.h"

#include <stdlib.h>


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

Node *NumberNode_new(const char *string, int length)
{
	Node *node = malloc(sizeof(*node));
	node->type = NUMBER_NODE;
	int number = 0;
	for (int i = 0; i < length; i++) {
		int digit = (string[i] - '0');
		number = digit + number * 10;
	}
	node->value.number = number;
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
