#include <stdlib.h>
#include <stdio.h>
#include "parse.h"
#include "scanner.h"

#define error(s) fprintf(stderr, "parsing error: %s\n", s)

static Node *parse_sum(Scanner *s);
static Node *parse_product(Scanner *s);
static Node *parse_term(Scanner *s);
static Node *parse_expt(Scanner *s);

// SUM ::= PRODUCT | PRODUCT '+' SUM
static Node *parse_sum(Scanner *scanner)
{
	Node *left = parse_product(scanner);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (next.type == PLUS_TOKEN) {
		Scanner_next(scanner);
		Node *right = parse_sum(scanner);
		if (!right) {
			Node_drop(left);
			return NULL;
		}
		return SumNode_new(left, right);
	}
	return left;
}

// PRODUCT ::= EXPT | EXPT '*' PRODUCT
static Node *parse_product(Scanner *scanner)
{
	Node *left = parse_expt(scanner);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (next.type == ASTERISK_TOKEN) {
		Scanner_next(scanner);
		Node *right = parse_product(scanner);
		if (!right) {
			Node_drop(left);
			return NULL;
		}
		return ProductNode_new(left, right);
	}
	return left;
}

// EXPT ::= TERM | TERM '^' EXPT
static Node *parse_expt(Scanner *scanner)
{
	Node *base = parse_term(scanner);
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

// TERM ::= '(' SUM ')' | 'NUMBER'
static Node *parse_term(Scanner *scanner)
{
	Token next = Scanner_next(scanner);
	if (next.type == LPAREN_TOKEN) {
		Node *expr = parse_sum(scanner);
		if (!expr) {
			return NULL;
		}
		next = Scanner_next(scanner);
		if (next.type != RPAREN_TOKEN) {
			error("expected )");
			Node_drop(expr);
			return NULL;
		}
		return expr;
	} else if (next.type == NUMBER_TOKEN) {
		return NumberNode_new(next.string, next.length);
	} else {
		error("expected '(' or a number");
		return NULL;
	}
}

// EXPRESSION ::= SUM 'END'
Node *parse(Scanner *scanner)
{
	Node *sum = parse_sum(scanner);
	if (!sum) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (next.type != END_TOKEN) {
		error("unexpected token after the sum");
		Node_drop(sum);
		return NULL;
	}
	return sum;
}
