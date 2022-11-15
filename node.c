#include <stdlib.h>
#include "node.h"

Node *NumberNode_new(int value)
{
	Node *node = malloc(sizeof(*node));
	node->type = NUMBER_NODE;
	node->value.number = value;
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
