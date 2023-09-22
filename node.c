#include "node.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


void Node_drop(passed Node *node)
{
	switch (node->type) {
		case NUMBER_NODE:
			free(node);
			break;
		case ID_NODE:
			free(node->as.id);
			free(node);
			break;
		case NEG_NODE:
			Node_drop(node->as.neg);
			free(node);
			break;
		case APPLICATION_NODE:
		case EXPT_NODE:
		case PRODUCT_NODE:
		case SUM_NODE:
		case CMP_NODE:
		case AND_NODE:
		case OR_NODE:
			Node_drop(PairNode_left(node));
			Node_drop(PairNode_right(node));
			free(node);
			break;
		case IF_NODE:
			Node_drop(IfNode_cond(node));
			Node_drop(IfNode_true(node));
			Node_drop(IfNode_false(node));
			free(node);
			break;
		case FN_NODE:
			Node_drop(FnNode_param(node));
			Node_drop(FnNode_body(node));
			free(node);
			break;
		case LET_NODE:
			Node_drop(LetNode_name(node));
			Node_drop(LetNode_value(node));
			free(node);
			break;
	}
}

void Node_drop_one(passed Node *node)
{
	free(node);
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
	node->as.number = number_value(string, length);
	return node;
}

static Node *NumberNode_copy(const Node *src)
{
	Node *node = malloc(sizeof(*node));
	node->type = NUMBER_NODE;
	node->as.number = src->as.number;
	return node;
}

Node *IdNode_new(const char *string, int length)
{
	Node *node = malloc(sizeof(*node));
	node->type = ID_NODE;
	node->as.id = strndup(string, length);
	return node;
}

static Node *IdNode_copy(const Node *src)
{
	Node *node = malloc(sizeof(*node));
	node->type = ID_NODE;
	node->as.id = strdup(src->as.id);
	return node;
}

Node *NegNode_new(passed Node *value)
{
	Node *node = malloc(sizeof(*node));
	node->type = NEG_NODE;
	node->as.neg = value;
	return node;
}

Node *NegNode_copy(const Node *src)
{
	Node *node = malloc(sizeof(*node));
	node->type = NEG_NODE;
	node->as.neg = Node_copy(src->as.neg);
	return node;
}

static Node *pair_new(NodeType type, passed Node *left, passed Node *right, int op)
{
	Node *node = malloc(sizeof(*node));
	node->type = type;
	node->as.pair.left = left;
	node->as.pair.right = right;
	node->as.pair.op = op;
	return node;
}

static Node *pair_copy(const Node *src)
{
	Node *node = malloc(sizeof(*node));
	node->type = src->type;
	node->as.pair.left = Node_copy(src->as.pair.left);
	node->as.pair.right = Node_copy(src->as.pair.right);
	node->as.pair.op = src->as.pair.op;
	return node;
}

Node *ApplicationNode_new(passed Node *left, passed Node *right)
{
	return pair_new(APPLICATION_NODE, left, right, 0);
}

Node *SumNode_new(passed Node *left, passed Node *right, int op)
{
	return pair_new(SUM_NODE, left, right, op);
}

Node *ProductNode_new(passed Node *left, passed Node *right, int op)
{
	return pair_new(PRODUCT_NODE, left, right, op);
}

Node *ExptNode_new(passed Node *base, passed Node *exponent)
{
	return pair_new(EXPT_NODE, base, exponent, '^');
}

Node *CmpNode_new(passed Node *left, passed Node *right, int op)
{
	return pair_new(CMP_NODE, left, right, op);
}

Node *AndNode_new(passed Node *left, passed Node *right)
{
	return pair_new(AND_NODE, left, right, 0);
}

Node *OrNode_new(passed Node *left, passed Node *right)
{
	return pair_new(OR_NODE, left, right, 0);
}

Node *IfNode_new(passed Node *cond, passed Node *true, passed Node *false)
{
	Node *node = malloc(sizeof(*node));
	node->type = IF_NODE;
	node->as.ifelse.cond = cond;
	node->as.ifelse.true = true;
	node->as.ifelse.false = false;
	return node;
}

static Node *IfNode_copy(const Node *src)
{
	return IfNode_new(
		Node_copy(IfNode_cond(src)),
		Node_copy(IfNode_true(src)),
		Node_copy(IfNode_false(src))
	);
}

Node *FnNode_new(passed Node *param, passed Node *body)
{
	Node *node = malloc(sizeof(*node));
	node->type = FN_NODE;
	node->as.fn.param = param;
	node->as.fn.body = body;
	return node;
}

static Node *FnNode_copy(const Node *src)
{
	return FnNode_new(
		Node_copy(src->as.fn.param),
		Node_copy(src->as.fn.body)
	);
}

Node *LetNode_new(passed Node *name, passed Node *value)
{
	Node *node = malloc(sizeof(*node));
	node->type = LET_NODE;
	node->as.let.name = name;
	node->as.let.value = value;
	return node;
}

static Node *LetNode_copy(const Node *src)
{
	return LetNode_new(
		Node_copy(LetNode_name(src)),
		Node_copy(LetNode_value(src))
	);
}

Node *Node_copy(const Node *node)
{
	switch (node->type) {
		case NUMBER_NODE:
			return NumberNode_copy(node);
		case ID_NODE:
			return IdNode_copy(node);
		case NEG_NODE:
			return NegNode_copy(node);
		case APPLICATION_NODE:
		case EXPT_NODE:
		case PRODUCT_NODE:
		case SUM_NODE:
		case CMP_NODE:
		case AND_NODE:
		case OR_NODE:
			return pair_copy(node);
		case IF_NODE:
			return IfNode_copy(node);
		case FN_NODE:
			return FnNode_copy(node);
		case LET_NODE:
			return LetNode_copy(node);
	}
	return NULL;
}
