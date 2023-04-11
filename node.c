#include "node.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


void Node_drop(Node *node)
{
	switch (node->type) {
		case NUMBER_NODE:
			free(node);
			break;
		case ID_NODE:
			free(node->value.id);
			free(node);
			break;
		case APPLICATION_NODE:
		case EXPT_NODE:
		case PRODUCT_NODE:
		case SUM_NODE:
		case CMP_NODE:
		case AND_NODE:
		case OR_NODE:
			Node_drop(node->value.pair.left);
			Node_drop(node->value.pair.right);
			free(node);
			break;
		case IF_NODE:
			Node_drop(node->value.ifelse.cond);
			Node_drop(node->value.ifelse.true);
			Node_drop(node->value.ifelse.false);
			free(node);
			break;
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

static char *copy_string(const char *src, int length)
{
	char *copy = malloc(length + 1);
	strncpy(copy, src, length);
	copy[length] = '\0';
	return copy;
}

Node *IdNode_new(const char *string, int length)
{
	Node *node = malloc(sizeof(*node));
	node->type = ID_NODE;
	node->value.id = copy_string(string, length);
	return node;
}

static Node *pair_new(NodeType type, Node *left, Node *right)
{
	Node *node = malloc(sizeof(*node));
	node->type = type;
	node->value.pair.left = left;
	node->value.pair.right = right;
	return node;
}

Node *ApplicationNode_new(Node *left, Node *right)
{
	return pair_new(APPLICATION_NODE, left, right);
}

Node *SumNode_new(Node *left, Node *right)
{
	return pair_new(SUM_NODE, left, right);
}

Node *ProductNode_new(Node *left, Node *right)
{
	return pair_new(PRODUCT_NODE, left, right);
}

Node *ExptNode_new(Node *base, Node *exponent)
{
	return pair_new(EXPT_NODE, base, exponent);
}

Node *CmpNode_new(Node *left, Node *right)
{
	return pair_new(CMP_NODE, left, right);
}

Node *AndNode_new(Node *left, Node *right)
{
	return pair_new(AND_NODE, left, right);
}

Node *OrNode_new(Node *left, Node *right)
{
	return pair_new(OR_NODE, left, right);
}

Node *IfNode_new(Node *cond, Node *true, Node *false)
{
	Node *node = malloc(sizeof(*node));
	node->type = IF_NODE;
	node->value.ifelse.cond = cond;
	node->value.ifelse.true = true;
	node->value.ifelse.false = false;
	return node;
}
