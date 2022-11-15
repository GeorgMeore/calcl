#include <stdio.h>
#include "token.h"
#include "node.h"

void print_tokens(const Token *tokens)
{
	printf("[");
	while (tokens->type != EOF_TOKEN) {
		printf("%.*s, ", tokens->length, tokens->string);
		tokens++;
	}
	printf("EOF]\n");
}

static void print_expression_rec(const Node *expr, int putnl)
{
	switch (expr->type) {
		case NUMBER_NODE:
			printf("%d", expr->value.number);
			break;
		case SUM_NODE:
			printf("(+ ");
			print_expression_rec(expr->value.left, 0);
			printf(" ");
			print_expression_rec(expr->value.right, 0);
			printf(")");
			break;
		case PRODUCT_NODE:
			printf("(*");
			print_expression_rec(expr->value.left, 0);
			printf(" ");
			print_expression_rec(expr->value.right, 0);
			printf(")");
	}
	if (putnl) {
		printf("\n");
	}
}

void print_expression(const Node *expr)
{
	print_expression_rec(expr, 1);
}
