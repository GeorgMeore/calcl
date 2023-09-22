#include "parse.h"

#include <stdlib.h>
#include <stdio.h>

#include "scanner.h"
#include "node.h"
#include "token.h"
#include "error.h"


#define ERROR_PREFIX "parsing error"

static void tokerror(const char *message, Token last)
{
	if (last.type == END_TOKEN) {
		errorf("%s (while parsing 'END')", message);
	} else {
		errorf("%s (while parsing '%.*s')", message, last.length, last.string);
	}
}

static Node *parse_let(Scanner *scanner);
static Node *parse_expression(Scanner *scanner);
static Node *parse_if(Scanner *s);
static Node *parse_fn(Scanner *s);
static Node *parse_or(Scanner *s);
static Node *parse_and(Scanner *s);
static Node *parse_cmp(Scanner *s);
static Node *parse_sum(Scanner *s);
static Node *parse_product(Scanner *s);
static Node *parse_expt(Scanner *s);
static Node *parse_application(Scanner *s);
static Node *parse_term(Scanner *s);

// TERM_TOKEN <- '(' | 'NUMBER' | 'ID'
static int is_term_token(Token token) {
	return (
		token.type == LPAREN_TOKEN ||
		token.type == NUMBER_TOKEN ||
		token.type == ID_TOKEN
	);
}

// CMP_TOKEN <- '>' | '<' | '='
static int is_cmp_token(Token token) {
	return (
		token.type == GT_TOKEN ||
		token.type == LT_TOKEN ||
		token.type == EQ_TOKEN
	);
}

// SUM_TOKEN <- '+' | '-'
static int is_sum_token(Token token) {
	return (token.type == PLUS_TOKEN || token.type == MINUS_TOKEN);
}

// PROD_TOKEN <- '*' | '/' | '%'
static int is_prod_token(Token token) {
	return (
		token.type == ASTERISK_TOKEN ||
		token.type == SLASH_TOKEN ||
		token.type == PERCENT_TOKEN
	);
}

// VALID ::= (EXPRESSION | LET)? 'END'
Node *parse(Scanner *scanner)
{
	Token next = Scanner_peek(scanner);
	if (next.type == END_TOKEN) {
		return NULL;
	}
	Node *expr = NULL;
	if (next.type == LET_TOKEN) {
		expr = parse_let(scanner);
	} else {
		expr = parse_expression(scanner);
	}
	if (!expr) {
		return NULL;
	}
	next = Scanner_peek(scanner);
	if (next.type != END_TOKEN) {
		tokerror("unexpected token after the expression", next);
		Node_drop(expr);
		return NULL;
	}
	return expr;
}

// LET_VALUE ::= '=' EXPRESSION | 'ID' LET_VALUE
static Node *parse_let_value(Scanner *scanner)
{
	Token next = Scanner_next(scanner);
	if (next.type == EQ_TOKEN) {
		return parse_expression(scanner);
	} else if (next.type == ID_TOKEN) {
		Node *param = IdNode_new(next.string, next.length);
		Node *body = parse_let_value(scanner);
		if (!body) {
			Node_drop(param);
			return NULL;
		}
		return FnNode_new(param, body);
	} else {
		tokerror("expected '=' or and identifier", next);
		return NULL;
	}
}

// LET ::= 'LET' 'ID' LET_VALUE
static Node *parse_let(Scanner *scanner)
{
	Scanner_next(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != ID_TOKEN) {
		tokerror("expected identifier", next);
		return NULL;
	}
	Node *name = IdNode_new(next.string, next.length);
	Node *value = parse_let_value(scanner);
	if (!value) {
		Node_drop(name);
		return NULL;
	}
	return LetNode_new(name, value);
}

// EXPRESSION ::= IF | FN | OR
static Node *parse_expression(Scanner *scanner)
{
	Token next = Scanner_peek(scanner);
	if (next.type == IF_TOKEN) {
		return parse_if(scanner);
	}
	if (next.type == FN_TOKEN) {
		return parse_fn(scanner);
	}
	return parse_or(scanner);
}

// FN_BODY ::= ':' EXPRESSION | 'ID' FN_BODY
static Node *parse_fn_body(Scanner *scanner)
{
	Token next = Scanner_next(scanner);
	if (next.type == COLON_TOKEN) {
		return parse_expression(scanner);
	} else if (next.type == ID_TOKEN) {
		Node *param = IdNode_new(next.string, next.length);
		Node *body = parse_fn_body(scanner);
		if (!body) {
			Node_drop(param);
			return NULL;
		}
		return FnNode_new(param, body);
	} else {
		tokerror("expected ':' or and identifier", next);
		return NULL;
	}
}

// FN ::= 'FN' FN_BODY
static Node *parse_fn(Scanner *scanner)
{
	Scanner_next(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != ID_TOKEN) {
		tokerror("expected identifier", next);
		return NULL;
	}
	Node *param = IdNode_new(next.string, next.length);
	Node *body = parse_fn_body(scanner);
	if (!body) {
		Node_drop(param);
		return NULL;
	}
	return FnNode_new(param, body);
}

// IF_TAIL ::= 'ELSE' EXPRESSION | IF
static Node *parse_if_tail(Scanner *scanner)
{
	Token next = Scanner_peek(scanner);
	if (next.type == ELSE_TOKEN) {
		Scanner_next(scanner);
		return parse_expression(scanner);
	} else if (next.type == IF_TOKEN) {
		return parse_if(scanner);
	} else {
		tokerror("expected 'if' or 'else'", next);
		return NULL;
	}
}

// IF ::= 'IF OR 'THEN' EXPRESSION IF_TAIL
static Node *parse_if(Scanner *scanner)
{
	Scanner_next(scanner);
	Node *cond = parse_or(scanner);
	if (!cond) {
		return NULL;
	}
	Token next = Scanner_next(scanner);
	if (next.type != THEN_TOKEN) {
		tokerror("expected 'then'", next);
		Node_drop(cond);
		return NULL;
	}
	Node *true = parse_expression(scanner);
	if (!true) {
		Node_drop(cond);
		return NULL;
	}
	Node *false = parse_if_tail(scanner);
	if (!false) {
		Node_drop(cond);
		Node_drop(true);
		return NULL;
	}
	return IfNode_new(cond, true, false);
}

// OR ::= AND {'OR' AND}
static Node *parse_or(Scanner *scanner)
{
	Node *left = parse_and(scanner);
	if (!left) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		if (next.type != OR_TOKEN) {
			return left;
		}
		Scanner_next(scanner);
		Node *right = parse_and(scanner);
		if (!right) {
			Node_drop(left);
			return NULL;
		}
		left = OrNode_new(left, right);
	}
}

// AND ::= CMP {'AND' CMP}
static Node *parse_and(Scanner *scanner)
{
	Node *left = parse_cmp(scanner);
	if (!left) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		if (next.type != AND_TOKEN) {
			return left;
		}
		Scanner_next(scanner);
		Node *right = parse_cmp(scanner);
		if (!right) {
			Node_drop(left);
			return NULL;
		}
		left = AndNode_new(left, right);
	}
}

// CMP ::= SUM (CMP_TOKEN SUM)?
static Node *parse_cmp(Scanner *scanner)
{
	Node *left = parse_sum(scanner);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (is_cmp_token(next)) {
		Scanner_next(scanner);
		Node *right = parse_sum(scanner);
		if (!right) {
			Node_drop(left);
			return NULL;
		}
		return CmpNode_new(left, right, next.string[0]);
	}
	return left;
}

// SUM ::= PRODUCT {SUM_TOKEN PRODUCT}
static Node *parse_sum(Scanner *scanner)
{
	Node *left = parse_product(scanner);
	if (!left) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		if (!is_sum_token(next)) {
			return left;
		}
		Scanner_next(scanner);
		Node *right = parse_product(scanner);
		if (!right) {
			Node_drop(left);
			return NULL;
		}
		left = SumNode_new(left, right, next.string[0]);
	}
}

// PRODUCT ::= EXPT {PROD_TOKEN EXPT}
static Node *parse_product(Scanner *scanner)
{
	Node *left = parse_expt(scanner);
	if (!left) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		if (!is_prod_token(next)) {
			return left;
		}
		Scanner_next(scanner);
		Node *right = parse_expt(scanner);
		if (!right) {
			Node_drop(left);
			return NULL;
		}
		left = ProductNode_new(left, right, next.string[0]);
	}
}

// EXPT ::= APPLICATION | APPLICATION '^' EXPT
static Node *parse_expt(Scanner *scanner)
{
	Node *base = parse_application(scanner);
	if (!base) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (next.type == CARET_TOKEN) {
		Scanner_next(scanner);
		Node *exponent = parse_expt(scanner);
		if (!exponent) {
			Node_drop(base);
			return NULL;
		}
		return ExptNode_new(base, exponent);
	}
	return base;
}

// APPLICATION ::= TERM {TERM}
static Node *parse_application(Scanner *scanner)
{
	Node *operator = parse_term(scanner);
	if (!operator) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		if (!is_term_token(next)) {
			return operator;
		}
		Node *operand = parse_term(scanner);
		if (!operand) {
			Node_drop(operator);
			return NULL;
		}
		operator = ApplicationNode_new(operator, operand);
	}
}

// TERM ::= '(' EXPRESSION ')' | 'NUMBER' | 'ID' | '-' TERM
static Node *parse_term(Scanner *scanner)
{
	Token next = Scanner_next(scanner);
	if (next.type == MINUS_TOKEN) {
		Node *term = parse_term(scanner);
		if (!term) {
			return NULL;
		}
		return NegNode_new(term);
	}
	if (next.type == LPAREN_TOKEN) {
		Node *expr = parse_expression(scanner);
		if (!expr) {
			return NULL;
		}
		next = Scanner_next(scanner);
		if (next.type != RPAREN_TOKEN) {
			tokerror("expected ')'", next);
			Node_drop(expr);
			return NULL;
		}
		return expr;
	} else if (next.type == NUMBER_TOKEN) {
		return NumberNode_new(next.string, next.length);
	} else if (next.type == ID_TOKEN) {
		return IdNode_new(next.string, next.length);
	} else {
		tokerror("expected '(', '-' or a number", next);
		return NULL;
	}
}
