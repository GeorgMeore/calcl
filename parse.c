#include <stdlib.h>
#include <stdio.h>
#include "parse.h"
#include "scanner.h"

#define error(s) fprintf(stderr, "parsing error: %s\n", s)

static Node *parse_sum(Scanner *s);
static Node *parse_product(Scanner *s);
static Node *parse_term(Scanner *s);
static Node *parse_number(Scanner *s);

// sum ::= product | product '+' sum
static Node *parse_sum(Scanner *s)
{
	Node *left = parse_product(s);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(s);
	if (next.type == PLUS_TOKEN) {
		Scanner_next(s);
		Node *right = parse_sum(s);
		if (!right) {
			return NULL;
		}
		return SumNode_new(left, right);
	}
	return left;
}

// product ::= term | term '*' product
static Node *parse_product(Scanner *s)
{
	Node *left = parse_term(s);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(s);
	if (next.type == ASTERISK_TOKEN) {
		Scanner_next(s);
		Node *right = parse_product(s);
		if (!right) {
			return NULL;
		}
		return ProductNode_new(left, right);
	}
	return left;
}

// term ::= '(' sum ')' | number
static Node *parse_term(Scanner *scanner)
{
	Token next = Scanner_peek(scanner);
	if (next.type == LPAREN_TOKEN) {
		Scanner_next(scanner);
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
		return parse_number(scanner);
	} else {
		error("expected '(' or a number");
		return NULL;
	}
}

// 'NUMBER'
static Node *parse_number(Scanner *s)
{
	Token t = Scanner_next(s);
	int number = 0;
	for (int i = 0; i < t.length; i++) {
		int digit = t.string[i] - '0';
		number = digit + number * 10;
	}
	return NumberNode_new(number);
}

// expression ::= sum 'EOF'
Node *parse(const Token *tokens)
{
	Scanner scanner = tokens;
	Node *expr = parse_sum(&scanner);
	if (!expr) {
		return NULL;
	}
	Token next = Scanner_peek(&scanner);
	if (next.type != EOF_TOKEN) {
		error("expected EOF");
		return NULL;
	}
	return expr;
}
