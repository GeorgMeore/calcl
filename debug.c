#include "debug.h"

#include <stdio.h>

#include "token.h"
#include "node.h"


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
			printf("CARET('%.*s')\n", token.length, token.string);
			break;
		case ASTERISK_TOKEN:
			printf("ASTERISK('%.*s')\n", token.length, token.string);
			break;
		case PLUS_TOKEN:
			printf("PLUS('%.*s')\n", token.length, token.string);
			break;
		case GT_TOKEN:
			printf("GT('%.*s')\n", token.length, token.string);
			break;
		case AND_TOKEN:
			printf("AND('%.*s')\n", token.length, token.string);
			break;
		case OR_TOKEN:
			printf("OR('%.*s')\n", token.length, token.string);
			break;
		case IF_TOKEN:
			printf("IF('%.*s')\n", token.length, token.string);
			break;
		case THEN_TOKEN:
			printf("THEN('%.*s')\n", token.length, token.string);
			break;
		case ELSE_TOKEN:
			printf("ELSE('%.*s')\n", token.length, token.string);
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
			indent(level); printf("NUMBER: %lf\n", expr->value.number);
			break;
		case EXPT_NODE:
			indent(level); printf("EXPONENTIATION: {\n");
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
		case SUM_NODE:
			indent(level); printf("SUM: {\n");
			print_tree(expr->value.pair.left, level + 1);
			print_tree(expr->value.pair.right, level + 1);
			indent(level); printf("}\n");
			break;
		case CMP_NODE:
			indent(level); printf("COMPARISON: {\n");
			print_tree(expr->value.pair.left, level + 1);
			print_tree(expr->value.pair.right, level + 1);
			indent(level); printf("}\n");
			break;
		case AND_NODE:
			indent(level); printf("AND: {\n");
			print_tree(expr->value.pair.left, level + 1);
			print_tree(expr->value.pair.right, level + 1);
			indent(level); printf("}\n");
			break;
		case OR_NODE:
			indent(level); printf("OR: {\n");
			print_tree(expr->value.pair.left, level + 1);
			print_tree(expr->value.pair.right, level + 1);
			indent(level); printf("}\n");
			break;
		case IF_NODE:
			indent(level); printf("IF: {\n");
			indent(level + 1); printf("CONDITION:\n");
			print_tree(expr->value.ifelse.cond, level + 2);
			indent(level + 1); printf("THEN:\n");
			print_tree(expr->value.ifelse.true, level + 2);
			indent(level + 1); printf("ELSE:\n");
			print_tree(expr->value.ifelse.false, level + 2);
			indent(level); printf("}\n");
			break;
	}
}

void print_expr(const Node *expr)
{
	print_tree(expr, 0);
}
