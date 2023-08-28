#include "debug.h"

#include <stdio.h>

#include "token.h"
#include "node.h"


#define do_print_token(name, token) \
	(printf(name "('%.*s')\n", token.length, token.string))

void print_token(Token token)
{
	switch (token.type) {
		case NUMBER_TOKEN:   do_print_token("NUMBER", token);   break;
		case ID_TOKEN:       do_print_token("ID", token);       break;
		case LPAREN_TOKEN:   do_print_token("LPAREN", token);   break;
		case RPAREN_TOKEN:   do_print_token("RPAREN", token);   break;
		case CARET_TOKEN:    do_print_token("CARET", token);    break;
		case ASTERISK_TOKEN: do_print_token("ASTERISK", token); break;
		case SLASH_TOKEN:    do_print_token("SLASH", token);    break;
		case PLUS_TOKEN:     do_print_token("PLUS", token);     break;
		case MINUS_TOKEN:    do_print_token("MINUS", token);    break;
		case GT_TOKEN:       do_print_token("GT", token);       break;
		case LT_TOKEN:       do_print_token("LT", token);       break;
		case EQ_TOKEN:       do_print_token("EQ", token);       break;
		case AND_TOKEN:      do_print_token("AND", token);      break;
		case OR_TOKEN:       do_print_token("OR", token);       break;
		case IF_TOKEN:       do_print_token("IF", token);       break;
		case THEN_TOKEN:     do_print_token("THEN", token);     break;
		case ELSE_TOKEN:     do_print_token("ELSE", token);     break;
		case FN_TOKEN:       do_print_token("FN", token);       break;
		case COLON_TOKEN:    do_print_token("COLON", token);    break;
		case LET_TOKEN:      do_print_token("LET", token);      break;
		case ERROR_TOKEN:    do_print_token("ERROR", token);    break;
		case END_TOKEN:      printf("END\n");
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
	indent(level); printf("%s(%c): {\n", type_string, expr->as.pair.op);
	print_tree(expr->as.pair.left, level + 1);
	print_tree(expr->as.pair.right, level + 1);
	indent(level); printf("}\n");
}

static void print_if(const Node *ifelse, int level)
{
	indent(level); printf("IF: {\n");
	indent(level + 1); printf("CONDITION:\n");
	print_tree(ifelse->as.ifelse.cond, level + 2);
	indent(level + 1); printf("THEN:\n");
	print_tree(ifelse->as.ifelse.true, level + 2);
	indent(level + 1); printf("ELSE:\n");
	print_tree(ifelse->as.ifelse.false, level + 2);
	indent(level); printf("}\n");
}

static void print_fn(const Node *fn, int level)
{
	indent(level); printf("FN: {\n");
	indent(level + 1); printf("PARAM:\n");
	print_tree(fn->as.fn.param, level + 2);
	indent(level + 1); printf("BODY:\n");
	print_tree(fn->as.fn.body, level + 2);
	indent(level); printf("}\n");
}

static void print_let(const Node *let, int level)
{
	indent(level); printf("LET: {\n");
	indent(level + 1); printf("NAME:\n");
	print_tree(let->as.let.name, level + 2);
	indent(level + 1); printf("VALUE:\n");
	print_tree(let->as.let.value, level + 2);
	indent(level); printf("}\n");
}

static void print_neg(const Node *neg, int level)
{
	indent(level); printf("NEG: {\n");
	print_tree(neg->as.neg, level + 1);
	indent(level); printf("}\n");
}

static void print_tree(const Node *expr, int level)
{
	switch (expr->type) {
		case NUMBER_NODE:
			indent(level); printf("NUMBER: %lf\n", expr->as.number);
			break;
		case ID_NODE:
			indent(level); printf("ID: %s\n", expr->as.id);
			break;
		case NEG_NODE:
			print_neg(expr, level);
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
			print_if(expr, level);
			break;
		case FN_NODE:
			print_fn(expr, level);
			break;
		case LET_NODE:
			print_let(expr, level);
			break;
	}
}

void print_expr(const Node *expr)
{
	print_tree(expr, 0);
}
