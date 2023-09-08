#include "debug.h"

#include <stdio.h>

#include "token.h"
#include "node.h"


#define tokprintf(name, token) \
	(printf("<%s|'%.*s'>", name, token.length, token.string))

void Token_print(Token token)
{
	switch (token.type) {
		case NUMBER_TOKEN:   tokprintf("number", token);   break;
		case ID_TOKEN:       tokprintf("id", token);       break;
		case LPAREN_TOKEN:   tokprintf("lparen", token);   break;
		case RPAREN_TOKEN:   tokprintf("rparen", token);   break;
		case CARET_TOKEN:    tokprintf("caret", token);    break;
		case ASTERISK_TOKEN: tokprintf("asterisk", token); break;
		case SLASH_TOKEN:    tokprintf("slash", token);    break;
		case PERCENT_TOKEN:  tokprintf("percent", token);  break;
		case PLUS_TOKEN:     tokprintf("plus", token);     break;
		case MINUS_TOKEN:    tokprintf("minus", token);    break;
		case GT_TOKEN:       tokprintf("gt", token);       break;
		case LT_TOKEN:       tokprintf("lt", token);       break;
		case EQ_TOKEN:       tokprintf("eq", token);       break;
		case AND_TOKEN:      tokprintf("and", token);      break;
		case OR_TOKEN:       tokprintf("or", token);       break;
		case IF_TOKEN:       tokprintf("if", token);       break;
		case THEN_TOKEN:     tokprintf("then", token);     break;
		case ELSE_TOKEN:     tokprintf("else", token);     break;
		case FN_TOKEN:       tokprintf("fn", token);       break;
		case COLON_TOKEN:    tokprintf("colon", token);    break;
		case LET_TOKEN:      tokprintf("let", token);      break;
		case ERROR_TOKEN:    tokprintf("error", token);    break;
		case END_TOKEN:      tokprintf("end", token);      break;
	}
}

static void indent(int level)
{
	while (level > 0) {
		printf("  ");
		level -= 1;
	}
}

#define iprintln(level, string)\
	(indent(level), puts(string))

#define iprintfln(level, fmt, args...)\
	(indent(level), printf(fmt "\n", args))

static void Node_iprint(const Node *expr, int level);

static void PairNode_iprint(const Node *expr, const char *type_string, int level)
{
	if (PairNode_op(expr)) {
		iprintfln(level, "%s(%c): {", type_string, PairNode_op(expr));
	} else {
		iprintfln(level, "%s: {", type_string);
	}
	Node_iprint(PairNode_left(expr), level + 1);
	Node_iprint(PairNode_right(expr), level + 1);
	iprintln(level, "}");
}

static void IfNode_iprint(const Node *expr, int level)
{
	iprintln(level, "IF: {");
	iprintln(level + 1, "CONDITION:");
	Node_iprint(IfNode_cond(expr), level + 2);
	iprintln(level + 1, "THEN:");
	Node_iprint(IfNode_true(expr), level + 2);
	iprintln(level + 1, "ELSE:");
	Node_iprint(IfNode_false(expr), level + 2);
	iprintln(level, "}");
}

static void FnNode_iprint(const Node *expr, int level)
{
	iprintln(level, "FN: {");
	iprintln(level + 1, "PARAM:");
	Node_iprint(FnNode_param(expr), level + 2);
	iprintln(level + 1, "BODY:");
	Node_iprint(FnNode_body(expr), level + 2);
	iprintln(level, "}");
}

static void LetNode_iprint(const Node *expr, int level)
{
	iprintln(level, "LET: {");
	iprintln(level + 1, "NAME:");
	Node_iprint(LetNode_name(expr), level + 2);
	iprintln(level + 1, "VALUE:");
	Node_iprint(LetNode_value(expr), level + 2);
	iprintln(level, "}");
}

static void NegNode_iprint(const Node *expr, int level)
{
	iprintln(level, "NEG: {");
	Node_iprint(NegNode_value(expr), level + 1);
	iprintln(level, "}");
}

static void Node_iprint(const Node *expr, int level)
{
	switch (expr->type) {
		case NUMBER_NODE:
			iprintfln(level, "NUMBER: %lf", NumNode_value(expr));
			break;
		case ID_NODE:
			iprintfln(level, "ID: %s", IdNode_value(expr));
			break;
		case NEG_NODE:         NegNode_iprint(expr, level);                 break;
		case APPLICATION_NODE: PairNode_iprint(expr, "APPLICATION", level); break;
		case EXPT_NODE:        PairNode_iprint(expr, "EXPT", level);        break;
		case PRODUCT_NODE:     PairNode_iprint(expr, "PRODUCT", level);     break;
		case SUM_NODE:         PairNode_iprint(expr, "SUM", level);         break;
		case CMP_NODE:         PairNode_iprint(expr, "CMP", level);         break;
		case AND_NODE:         PairNode_iprint(expr, "AND", level);         break;
		case OR_NODE:          PairNode_iprint(expr, "OR", level);          break;
		case IF_NODE:          IfNode_iprint(expr, level);                  break;
		case FN_NODE:          FnNode_iprint(expr, level);                  break;
		case LET_NODE:         LetNode_iprint(expr, level);                 break;
	}
}

void Node_print(const Node *expr)
{
	Node_iprint(expr, 0);
}
