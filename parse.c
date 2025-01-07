#include "parse.h"

#include <stdio.h>
#include <ctype.h>

#include "scanner.h"
#include "node.h"
#include "token.h"
#include "error.h"
#include "arena.h"


#define ERROR_PREFIX "parsing error"

static void tokerror(const char *message, Token last)
{
	if (last.type == EndToken) {
		errorf("%s (while parsing 'END')", message);
	} else {
		errorf("%s (while parsing '%.*s')", message, last.length, last.string);
	}
}

typedef enum {
	LeftAssoc,
	RightAssoc,
	NoneAssoc
} Assoc;

typedef struct {
	Assoc     assoc;
	NodeType  optype;
	int       count;
	TokenType types[TOKEN_COUNT];
} Oplevel;

// Binary operators in the order of increasing precedence
static const Oplevel optable[] = {
	{LeftAssoc,  OrNode,      1, {OrToken}},
	{LeftAssoc,  AndNode,     1, {AndToken}},
	{NoneAssoc,  CmpNode,     3, {GtToken, LtToken, EqToken}},
	{LeftAssoc,  SumNode,     2, {PlusToken, MinusToken}},
	{LeftAssoc,  ProdNode,    3, {AsteriskToken, SlashToken, PercentToken}},
	{RightAssoc, ExptNode,    1, {CaretToken}},
};

#define MAXPREC (int)(sizeof(optable)/sizeof(optable[0]))

// OP_TOKEN <- any token with type from op->types
static int is_op_token(const Oplevel *op, Token token)
{
	for (int i = 0; i < op->count; i++) {
		if (op->types[i] == token.type) {
			return 1;
		}
	}
	return 0;
}

// TERM_TOKEN <- '(' | 'NUMBER' | 'ID'
static int is_term_token(Token token)
{
	return (
		token.type == LparenToken ||
		token.type == NumberToken ||
		token.type == IdToken
	);
}

static Node *parse_let(Scanner *scanner, Arena *a);
static Node *parse_expression(Scanner *scanner, Arena *a);
static Node *parse_if(Scanner *s, Arena *a);
static Node *parse_fn(Scanner *s, Arena *a);
static Node *parse_op(Scanner *scanner, int prec, Arena *a);
static Node *parse_application(Scanner *s, Arena *a);
static Node *parse_term(Scanner *s, Arena *a);

// VALID ::= (EXPRESSION | LET)? 'END'
Node *parse(Scanner *scanner, Arena *a)
{
	Scanner_start(scanner);
	Token next = Scanner_peek(scanner);
	if (next.type == EndToken) {
		return NULL;
	}
	Node *expr = NULL;
	if (next.type == LetToken) {
		expr = parse_let(scanner, a);
	} else {
		expr = parse_expression(scanner, a);
	}
	if (!expr) {
		Scanner_seek_end(scanner);
		return NULL;
	}
	next = Scanner_peek(scanner);
	if (next.type != EndToken) {
		tokerror("unexpected token after the expression", next);
		Scanner_seek_end(scanner);
		return NULL;
	}
	return expr;
}

// LET_VALUE ::= '=' EXPRESSION | 'ID' LET_VALUE
static Node *parse_let_value(Scanner *scanner, Arena *a)
{
	Token next = Scanner_next(scanner);
	if (next.type == EqToken) {
		Scanner_skip_nl(scanner);
		return parse_expression(scanner, a);
	} else if (next.type == IdToken) {
		Node *param = IdNode_new(a, next.string, next.length);
		Scanner_skip_nl(scanner);
		Node *body = parse_let_value(scanner, a);
		if (!body) {
			return NULL;
		}
		return FnNode_new(a, param, body);
	} else {
		tokerror("expected '=' or an identifier", next);
		return NULL;
	}
}

// LET ::= 'LET' 'ID' LET_VALUE
static Node *parse_let(Scanner *scanner, Arena *a)
{
	Scanner_next(scanner); // drop 'LET'
	Scanner_skip_nl(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != IdToken) {
		tokerror("expected identifier", next);
		return NULL;
	}
	Node *name = IdNode_new(a, next.string, next.length);
	Scanner_skip_nl(scanner);
	Node *value = parse_let_value(scanner, a);
	if (!value) {
		return NULL;
	}
	return LetNode_new(a, name, value);
}

// EXPRESSION ::= IF | FN | OP
static Node *parse_expression(Scanner *scanner, Arena *a)
{
	Token next = Scanner_peek(scanner);
	if (next.type == IfToken) {
		return parse_if(scanner, a);
	}
	if (next.type == FnToken) {
		return parse_fn(scanner, a);
	}
	return parse_op(scanner, 0, a);
}

// FN_BODY ::= ':' EXPRESSION | 'ID' FN_BODY
static Node *parse_fn_body(Scanner *scanner, Arena *a)
{
	Token next = Scanner_next(scanner);
	if (next.type == ColonToken) {
		Scanner_skip_nl(scanner);
		return parse_expression(scanner, a);
	} else if (next.type == IdToken) {
		Node *param = IdNode_new(a, next.string, next.length);
		Scanner_skip_nl(scanner);
		Node *body = parse_fn_body(scanner, a);
		if (!body) {
			return NULL;
		}
		return FnNode_new(a, param, body);
	} else {
		tokerror("expected ':' or and identifier", next);
		return NULL;
	}
}

// FN ::= 'FN' FN_BODY
static Node *parse_fn(Scanner *scanner, Arena *a)
{
	Scanner_next(scanner); // drop 'FN'
	Scanner_skip_nl(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != IdToken) {
		tokerror("expected identifier", next);
		return NULL;
	}
	Node *param = IdNode_new(a, next.string, next.length);
	Scanner_skip_nl(scanner);
	Node *body = parse_fn_body(scanner, a);
	if (!body) {
		return NULL;
	}
	return FnNode_new(a, param, body);
}

// IF_TAIL ::= 'ELSE' EXPRESSION | IF
static Node *parse_if_tail(Scanner *scanner, Arena *a)
{
	Token next = Scanner_peek(scanner);
	if (next.type == ElseToken) {
		Scanner_next(scanner);
		Scanner_skip_nl(scanner);
		return parse_expression(scanner, a);
	} else if (next.type == IfToken) {
		return parse_if(scanner, a);
	} else {
		tokerror("expected 'if' or 'else'", next);
		return NULL;
	}
}

// IF ::= 'IF OP 'THEN' EXPRESSION IF_TAIL
static Node *parse_if(Scanner *scanner, Arena *a)
{
	Scanner_next(scanner); // drop 'IF'
	Scanner_skip_nl(scanner);
	Node *cond = parse_op(scanner, 0, a);
	if (!cond) {
		return NULL;
	}
	Scanner_skip_nl(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != ThenToken) {
		tokerror("expected 'then'", next);
		return NULL;
	}
	Scanner_skip_nl(scanner);
	Node *true = parse_expression(scanner, a);
	if (!true) {
		return NULL;
	}
	Scanner_skip_nl(scanner);
	Node *false = parse_if_tail(scanner, a);
	if (!false) {
		return NULL;
	}
	return IfNode_new(a, cond, true, false);
}

// LASSOC ::= OP | LASSOC OP_TOKEN OP
static Node *parse_lassoc(Scanner *scanner, const Oplevel *op, int prec, Arena *a)
{
	Node *left = parse_op(scanner, prec + 1, a);
	if (!left) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		if (!is_op_token(op, next)) {
			return left;
		}
		Scanner_next(scanner);
		Scanner_skip_nl(scanner);
		Node *right = parse_op(scanner, prec + 1, a);
		if (!right) {
			return NULL;
		}
		left = OpNode_new(a, left, right, op->optype, next.string[0]);
	}
}

// RASSOC ::= OP | OP OP_TOKEN RASSOC
static Node *parse_rassoc(Scanner *scanner, const Oplevel *op, int prec, Arena *a)
{
	Node *left = parse_op(scanner, prec + 1, a);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (is_op_token(op, next)) {
		Scanner_next(scanner);
		Scanner_skip_nl(scanner);
		Node *right = parse_op(scanner, prec + 1, a);
		if (!right) {
			return NULL;
		}
		return OpNode_new(a, left, right, op->optype, next.string[0]);
	}
	return left;
}

// NASSOC ::= OP | OP OP_TOKEN OP
static Node *parse_nassoc(Scanner *scanner, const Oplevel *op, int prec, Arena *a)
{
	Node *left = parse_op(scanner, prec + 1, a);
	if (!left) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	if (is_op_token(op, next)) {
		Scanner_next(scanner);
		Scanner_skip_nl(scanner);
		Node *right = parse_op(scanner, prec + 1, a);
		if (!right) {
			return NULL;
		}
		return OpNode_new(a, left, right, op->optype, next.string[0]);
	}
	return left;
}

// OP ::= APPLICATION | RASSOC | LASSOC | NASSOC
static Node *parse_op(Scanner *scanner, int prec, Arena *a)
{
	if (prec >= MAXPREC) {
		return parse_application(scanner, a);
	}
	const Oplevel *op = &optable[prec];
	switch (op->assoc) {
		case LeftAssoc:
			return parse_lassoc(scanner, op, prec, a);
		case RightAssoc:
			return parse_rassoc(scanner, op, prec, a);
		case NoneAssoc:
			return parse_nassoc(scanner, op, prec, a);
		default: // unreachable
			return NULL;
	}
}

// APPLICATION ::= TERM | APPLICATION TERM
static Node *parse_application(Scanner *scanner, Arena *a)
{
	Node *operator = parse_term(scanner, a);
	if (!operator) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		if (!is_term_token(next)) {
			return operator;
		}
		Node *operand = parse_term(scanner, a);
		if (!operand) {
			return NULL;
		}
		operator = ApplicationNode_new(a, operator, operand);
	}
}

// NUMBER ::= DIGIT+ ('.' DIGIT*)?
static Node *parse_number(Token tok, Arena *a)
{
	double number = 0;
	int i;
	for (i = 0; i < tok.length && isdigit(tok.string[i]); i++) {
		number = (tok.string[i] - '0') + number * 10;
	}
	if (tok.string[i] == '.') {
		i++;
	}
	for (double factor = 0.1; i < tok.length; i++, factor/=10) {
		number += (tok.string[i] - '0') * factor;
	}
	return NumberNode_new(a, number);
}

// TERM ::= '(' EXPRESSION ')' | 'NUMBER' | 'ID' | '-' TERM
static Node *parse_term(Scanner *scanner, Arena *a)
{
	Token next = Scanner_next(scanner);
	if (next.type == MinusToken) {
		Node *term = parse_term(scanner, a);
		if (!term) {
			return NULL;
		}
		return OpNode_new(a, term, NumberNode_new(a, -1), ProdNode, '*');
	}
	if (next.type == LparenToken) {
		Scanner_skip_nl(scanner);
		Node *expr = parse_expression(scanner, a);
		if (!expr) {
			return NULL;
		}
		Scanner_skip_nl(scanner);
		next = Scanner_next(scanner);
		if (next.type != RparenToken) {
			tokerror("expected ')'", next);
			return NULL;
		}
		return expr;
	} else if (next.type == NumberToken) {
		return parse_number(next, a);
	} else if (next.type == IdToken) {
		return IdNode_new(a, next.string, next.length);
	} else {
		tokerror("expected '(', '-' or a number", next);
		return NULL;
	}
}
