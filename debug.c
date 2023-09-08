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
		case PERCENT_TOKEN:  do_print_token("PERCENT", token);  break;
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
	if (PairNode_op(expr)) {
		indent(level); printf("%s(%c): {\n", type_string, PairNode_op(expr));
	} else {
		indent(level); printf("%s: {\n", type_string);
	}
	print_tree(PairNode_left(expr), level + 1);
	print_tree(PairNode_right(expr), level + 1);
	indent(level); printf("}\n");
}

static void print_if(const Node *expr, int level)
{
	indent(level); printf("IF: {\n");
	indent(level + 1); printf("CONDITION:\n");
	print_tree(IfNode_cond(expr), level + 2);
	indent(level + 1); printf("THEN:\n");
	print_tree(IfNode_true(expr), level + 2);
	indent(level + 1); printf("ELSE:\n");
	print_tree(IfNode_false(expr), level + 2);
	indent(level); printf("}\n");
}

static void print_fn(const Node *expr, int level)
{
	indent(level); printf("FN: {\n");
	indent(level + 1); printf("PARAM:\n");
	print_tree(FnNode_param(expr), level + 2);
	indent(level + 1); printf("BODY:\n");
	print_tree(FnNode_body(expr), level + 2);
	indent(level); printf("}\n");
}

static void print_let(const Node *expr, int level)
{
	indent(level); printf("LET: {\n");
	indent(level + 1); printf("NAME:\n");
	print_tree(LetNode_name(expr), level + 2);
	indent(level + 1); printf("VALUE:\n");
	print_tree(LetNode_value(expr), level + 2);
	indent(level); printf("}\n");
}

static void print_neg(const Node *expr, int level)
{
	indent(level); printf("NEG: {\n");
	print_tree(NegNode_value(expr), level + 1);
	indent(level); printf("}\n");
}

static void print_number(const Node *expr, int level)
{
	indent(level); printf("NUMBER: %lf\n", NumNode_value(expr));
}

static void print_id(const Node *expr, int level)
{
	indent(level); printf("ID: %s\n", IdNode_value(expr));
}

static void print_tree(const Node *expr, int level)
{
	switch (expr->type) {
		case NUMBER_NODE:      print_number(expr, level);                 break;
		case ID_NODE:          print_id(expr, level);                     break;
		case NEG_NODE:         print_neg(expr, level);                    break;
		case APPLICATION_NODE: print_pair(expr, "APPLICATION", level);    break;
		case EXPT_NODE:        print_pair(expr, "EXPONENTIATION", level); break;
		case PRODUCT_NODE:     print_pair(expr, "PRODUCT", level);        break;
		case SUM_NODE:         print_pair(expr, "SUM", level);            break;
		case CMP_NODE:         print_pair(expr, "COMPARISON", level);     break;
		case AND_NODE:         print_pair(expr, "AND", level);            break;
		case OR_NODE:          print_pair(expr, "OR", level);             break;
		case IF_NODE:          print_if(expr, level);                     break;
		case FN_NODE:          print_fn(expr, level);                     break;
		case LET_NODE:         print_let(expr, level);                    break;
	}
}

void print_expr(const Node *expr)
{
	print_tree(expr, 0);
}
