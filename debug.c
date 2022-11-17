#include <stdio.h>
#include "debug.h"

void print_token(Token token)
{
	switch (token.type) {
		case NUMBER_TOKEN:
			printf("NUMBER('%.*s')\n", token.length, token.string);
			break;
		case LPAREN_TOKEN:
			printf("LPAREN('%.*s')\n", token.length, token.string);
			break;
		case RPAREN_TOKEN:
			printf("RPAREN('%.*s')\n", token.length, token.string);
			break;
		case CARET_TOKEN:
			printf("CARROT('%.*s')\n", token.length, token.string);
			break;
		case ASTERISK_TOKEN:
			printf("ASTERISK('%.*s')\n", token.length, token.string);
			break;
		case PLUS_TOKEN:
			printf("PLUS('%.*s')\n", token.length, token.string);
			break;
		case ERROR_TOKEN:
			printf("ERROR('%.*s')\n", token.length, token.string);
			break;
		case END_TOKEN:
			printf("END\n");
			return;
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
			print_tree(expr->value.pair.left, level + 1);
			print_tree(expr->value.pair.right, level + 1);
			indent(level); printf("}\n");
			break;
		case PRODUCT_NODE:
			indent(level); printf("PRODUCT: {\n");
			print_tree(expr->value.pair.left, level + 1);
			print_tree(expr->value.pair.right, level + 1);
			indent(level); printf("}\n");
			break;
		case EXPT_NODE:
			indent(level); printf("EXPT: {\n");
			print_tree(expr->value.pair.left, level + 1);
			print_tree(expr->value.pair.right, level + 1);
			indent(level); printf("}\n");
	}
}

void print_expr(const Node *expr)
{
	print_tree(expr, 0);
}
