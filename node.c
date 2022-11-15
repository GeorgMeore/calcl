#include <stdlib.h>
#include "node.h"

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
