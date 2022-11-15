#include <stdio.h>
#include "token.h"
#include "node.h"

void print_tokens(const Token *tokens)
{
	while (tokens++) {
		switch (tokens->type) {
			case LPAREN_TOKEN:
				printf("LPAREN('%.*s')\n", tokens->length, tokens->string);
				break;
			case RPAREN_TOKEN:
				printf("RPAREN('%.*s')\n", tokens->length, tokens->string);
				break;
			case ASTERISK_TOKEN:
				printf("ASTERISK('%.*s')\n", tokens->length, tokens->string);
				break;
			case PLUS_TOKEN:
				printf("PLUS('%.*s')\n", tokens->length, tokens->string);
				break;
			case NUMBER_TOKEN:
				printf("NUMBER('%.*s')\n", tokens->length, tokens->string);
				break;
			case EOF_TOKEN:
				printf("EOF\n");
				return;
		}
	}
}

static void indent(int level)
{
	while (level--) {
		putchar('\t');
	}
}

static void print_tree(const Node *expr, int level)
{
	switch (expr->type) {
		case NUMBER_NODE:
			indent(level); printf("NUMBER: %d\n", expr->value.number);
			break;
		case SUM_NODE:
			indent(level); printf("SUM: {\n");
			print_tree(expr->value.left, level + 1);
			print_tree(expr->value.right, level + 1);
			indent(level); printf("}\n");
			break;
		case PRODUCT_NODE:
			indent(level); printf("PRODUCT: {\n");
			print_tree(expr->value.left, level + 1);
			print_tree(expr->value.right, level + 1);
			indent(level); printf("}");
	}
}

void print_expr(const Node *expr)
{
	print_tree(expr, 0);
}
