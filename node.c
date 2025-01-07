#include "node.h"

#include <stdio.h>
#include <string.h>

#include "arena.h"


static Node *Node_alloc(Arena *a, NodeType type)
{
	Node *node = Arena_alloc(a, sizeof(*node));
	node->type = type;
	return node;
}

Node *NumberNode_new(Arena *a, double number)
{
	Node *node = Node_alloc(a, NumberNode);
	node->as.number = number;
	return node;
}

Node *IdNode_new(Arena *a, const char *string, int length)
{
	Node *node = Node_alloc(a, IdNode);
	char *id = Arena_alloc(a, length+1);
	strncpy(id, string, length);
	id[length] = '\0';
	node->as.id = id;
	return node;
}

static Node *PairNode_new(Arena *a, NodeType type, Node *left, Node *right, int op)
{
	Node *node = Node_alloc(a, type);
	node->as.pair.left = left;
	node->as.pair.right = right;
	node->as.pair.op = op;
	return node;
}

Node *ApplicationNode_new(Arena *a, Node *left, Node *right)
{
	return PairNode_new(a, ApplNode, left, right, ' ');
}

Node *OpNode_new(Arena *a, Node *left, Node *right, NodeType type, int op)
{
	return PairNode_new(a, type, left, right, op);
}

Node *IfNode_new(Arena *a, Node *cond, Node *true, Node *false)
{
	Node *node = Node_alloc(a, IfNode);
	node->as.ifelse.cond = cond;
	node->as.ifelse.true = true;
	node->as.ifelse.false = false;
	return node;
}

Node *FnNode_new(Arena *a, Node *param, Node *body)
{
	Node *node = Node_alloc(a, FnNode);
	node->as.fn.param = param;
	node->as.fn.body = body;
	return node;
}

Node *LetNode_new(Arena *a, Node *name, Node *value)
{
	Node *node = Node_alloc(a, LetNode);
	node->as.let.name = name;
	node->as.let.value = value;
	return node;
}

static void Node_print_parenthesised(const Node *expr)
{
	putchar('(');
	Node_print(expr);
	putchar(')');
}

void Node_print(const Node *expr)
{
	switch (expr->type) {
		case NumberNode:
			printf("%lf", NumNode_value(expr));
			break;
		case IdNode:
			printf("%s", IdNode_value(expr));
			break;
		case ApplNode:
		case ExptNode:
		case ProdNode:
		case SumNode:
		case CmpNode:
		case AndNode:
		case OrNode:
			Node_print_parenthesised(PairNode_left(expr));
			putchar(PairNode_op(expr));
			Node_print_parenthesised(PairNode_right(expr));
			break;
		case IfNode:
			printf("if ");
			Node_print_parenthesised(IfNode_cond(expr));
			printf(" then ");
			Node_print_parenthesised(IfNode_true(expr));
			printf(" else ");
			Node_print_parenthesised(IfNode_false(expr));
			break;
		case FnNode:
			printf("fn ");
			Node_print(FnNode_param(expr));
			printf(": ");
			Node_print(FnNode_body(expr));
			break;
		case LetNode:
			printf("let ");
			Node_print(LetNode_name(expr));
			printf("= ");
			Node_print(LetNode_value(expr));
			break;
	}
}

void Node_println(const Node *node)
{
	Node_print(node);
	putchar('\n');
}
