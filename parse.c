#include <stdlib.h>
#include <stdio.h>
#include "parse.h"
#include "scanner.h"

#define error(s) fprintf(stderr, "parsing error: %s\n", s)

static Node *parse_sum(Scanner *s);
static Node *parse_product(Scanner *s);
static Node *parse_term(Scanner *s);

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
			return NULL;
		}
		return SumNode_new(left, right);
	}
	return left;
}

// PRODUCT ::= TERM | TERM '*' PRODUCT
static Node *parse_product(Scanner *scanner)
{
	Node *left = parse_term(scanner);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (next.type == ASTERISK_TOKEN) {
		Scanner_next(scanner);
		Node *right = parse_product(scanner);
		if (!right) {
			return NULL;
		}
		return ProductNode_new(left, right);
	}
	return left;
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
	Node *expr = parse_sum(scanner);
	if (!expr) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (next.type != END_TOKEN) {
		error("unexpected token after the sum");
		return NULL;
	}
	return expr;
}
