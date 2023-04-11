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
		case ID_TOKEN:
			printf("ID('%.*s')\n", token.length, token.string);
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

static void print_tree(const Node *expr, int level);

static void print_pair(const Node *expr, const char *type_string, int level)
{
	indent(level); printf("%s: {\n", type_string);
	print_tree(expr->value.pair.left, level + 1);
	print_tree(expr->value.pair.right, level + 1);
	indent(level); printf("}\n");
}

static void print_tree(const Node *expr, int level)
{
	switch (expr->type) {
		case NUMBER_NODE:
			indent(level); printf("NUMBER: %lf\n", expr->value.number);
			break;
		case ID_NODE:
			indent(level); printf("ID: %s\n", expr->value.id);
			break;
		case APPLICATION_NODE:
			print_pair(expr, "APPLICATION", level);
			break;
		case EXPT_NODE:
			print_pair(expr, "EXPONENTIATION", level);
			break;
		case PRODUCT_NODE:
			print_pair(expr, "PRODUCT", level);
			break;
		case SUM_NODE:
			print_pair(expr, "SUM", level);
			break;
		case CMP_NODE:
			print_pair(expr, "COMPARISON", level);
			break;
		case AND_NODE:
			print_pair(expr, "AND", level);
			break;
		case OR_NODE:
			print_pair(expr, "OR", level);
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
