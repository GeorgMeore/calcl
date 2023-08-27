#include "parse.h"

#include <stdlib.h>
#include <stdio.h>

#include "scanner.h"
#include "node.h"
#include "token.h"


static void error(const char *message, Token last)
{
	fprintf(stderr, "parsing error: %s ", message);
	if (last.type == END_TOKEN) {
		fprintf(stderr, "(while parsing 'EOF')\n");
	} else {
		fprintf(stderr, "(while parsing '%.*s')\n", last.length, last.string);
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

// TERMTOKEN <- '(' | 'NUMBER' | 'ID'
static int is_term_token(Token token) {
	return (
		token.type == LPAREN_TOKEN ||
		token.type == NUMBER_TOKEN ||
		token.type == ID_TOKEN
	);
}

// CMPTOKEN <- '>' | '<' | '='
static int is_cmp_token(Token token) {
	return (
		token.type == GT_TOKEN ||
		token.type == LT_TOKEN ||
		token.type == EQ_TOKEN
	);
}

// SUMTOKEN <- '+' | '-'
static int is_sum_token(Token token) {
	return (token.type == PLUS_TOKEN || token.type == MINUS_TOKEN);
}

// PRODTOKEN <- '*' | '/'
static int is_prod_token(Token token) {
	return (token.type == ASTERISK_TOKEN || token.type == SLASH_TOKEN);
}

// VALID ::= (EXPRESSION | LET) 'END'
Node *parse(Scanner *scanner)
{
	Token next = Scanner_peek(scanner);
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
		error("unexpected token after the expression", next);
		Node_drop(expr);
		return NULL;
	}
	return expr;
}

// LET ::= 'LET' 'ID' '=' EXPRESSION
static Node *parse_let(Scanner *scanner)
{
	Scanner_next(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != ID_TOKEN) {
		error("expected identifier", next);
		return NULL;
	}
	Node *name = IdNode_new(next.string, next.length);
	next = Scanner_next(scanner);
	if (next.type != EQ_TOKEN) {
		error("expected '='", next);
		Node_drop(name);
		return NULL;
	}
	Node *value = parse_expression(scanner);
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

// FN ::= 'FN' 'ID' 'TO' EXPRESSION
static Node *parse_fn(Scanner *scanner)
{
	Scanner_next(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != ID_TOKEN) {
		error("expected identifier", next);
		return NULL;
	}
	Node *param = IdNode_new(next.string, next.length);
	next = Scanner_next(scanner);
	if (next.type != COLON_TOKEN) {
		error("expected ':'", next);
		Node_drop(param);
		return NULL;
	}
	Node *body = parse_expression(scanner);
	if (!body) {
		Node_drop(param);
		return NULL;
	}
	return FnNode_new(param, body);
}

// IF ::= 'IF' OR 'THEN' EXPRESSION 'ELSE' EXPRESSION
static Node *parse_if(Scanner *scanner)
{
	Scanner_next(scanner);
	Node *cond = parse_or(scanner);
	if (!cond) {
		return NULL;
	}
	Token next = Scanner_next(scanner);
	if (next.type != THEN_TOKEN) {
		error("expected 'then'", next);
		return NULL;
	}
	Node *true = parse_expression(scanner);
	if (!true) {
		Node_drop(cond);
		return NULL;
	}
	next = Scanner_next(scanner);
	if (next.type != ELSE_TOKEN) {
		error("expected 'else'", next);
		Node_drop(cond);
		Node_drop(true);
		return NULL;
	}
	Node *false = parse_expression(scanner);
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

// CMP ::= SUM {CMPTOKEN SUM}
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
			return NULL;
		}
		return CmpNode_new(left, right, next.string[0]);
	}
	return left;
}

// SUM ::= PRODUCT {SUMTOKEN PRODUCT}
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

// PRODUCT ::= EXPT {PRODTOKEN EXPT}
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

// TERM ::= '(' EXPRESSION ')' | 'NUMBER' | 'ID'
static Node *parse_term(Scanner *scanner)
{
	Token next = Scanner_next(scanner);
	if (next.type == LPAREN_TOKEN) {
		Node *expr = parse_expression(scanner);
		if (!expr) {
			return NULL;
		}
		next = Scanner_next(scanner);
		if (next.type != RPAREN_TOKEN) {
			error("expected ')'", next);
			Node_drop(expr);
			return NULL;
		}
		return expr;
	} else if (next.type == NUMBER_TOKEN) {
		return NumberNode_new(next.string, next.length);
	} else if (next.type == ID_TOKEN) {
		return IdNode_new(next.string, next.length);
	} else {
		error("expected '(' or a number", next);
		return NULL;
	}
}
