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

// OP_TOKEN <- any token from optable
static const Oplevel *is_op_token(Token token)
{
	for (int prec = 0; prec < MAXPREC; prec++) {
		for (int i = 0; i < optable[prec].count; i++) {
			if (optable[prec].types[i] == token.type) {
				return &optable[prec];
			}
		}
	}
	return NULL;
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
static Node *parse_opseq(Scanner *scanner, const Oplevel *op, Arena *a);
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
	return parse_opseq(scanner, 0, a);
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

// IF ::= 'IF OPSEQ 'THEN' EXPRESSION IF_TAIL
static Node *parse_if(Scanner *scanner, Arena *a)
{
	Scanner_next(scanner); // drop 'IF'
	Scanner_skip_nl(scanner);
	Node *cond = parse_opseq(scanner, 0, a);
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

// LASSOC ::= OP OPSEQ | LASSOC OP OPSEQ
static Node *parse_lassoc(Scanner *scanner, const Oplevel *op, Node *left, Arena *a)
{
	for (;;) {
		Token optok = Scanner_next(scanner);
		Scanner_skip_nl(scanner);
		Node *right = parse_opseq(scanner, op, a);
		if (!right) {
			return NULL;
		}
		left = OpNode_new(a, left, right, op->optype, optok.string[0]);
		Token next = Scanner_peek(scanner);
		const Oplevel *op2 = is_op_token(next);
		if (op2 < op) {
			return left;
		}
	}
}

// RASSOC ::= OP OPSEQ | OP OPSEQ RASSOC
static Node *parse_rassoc(Scanner *scanner, const Oplevel *op, Node *left, Arena *a)
{
	Token optok = Scanner_next(scanner);
	Scanner_skip_nl(scanner);
	Node *right = parse_opseq(scanner, op, a);
	if (!right) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	const Oplevel *op2 = is_op_token(next);
	if (op2 == op) {
		right = parse_rassoc(scanner, op, right, a);
		if (!right) {
			return NULL;
		}
	}
	return OpNode_new(a, left, right, op->optype, optok.string[0]);
}

// NASSOC ::= OP OPSEQ
static Node *parse_nassoc(Scanner *scanner, const Oplevel *op, Node *left, Arena *a)
{
	Token optok = Scanner_next(scanner);
	Scanner_skip_nl(scanner);
	Node *right = parse_opseq(scanner, op, a);
	if (!right) {
		return NULL;
	}
	Token next = Scanner_peek(scanner);
	const Oplevel *op2 = is_op_token(next);
	if (op2 == op) {
		tokerror("non-assosiative operator", next);
		return NULL;
	}
	return OpNode_new(a, left, right, op->optype, optok.string[0]);
}

// OPSEQ ::= APPLICATION | APPLICATION (LASSOC | RASSOC | NASSOC)
static Node *parse_opseq(Scanner *scanner, const Oplevel *op, Arena *a)
{
	Node *left = parse_application(scanner, a);
	if (!left) {
		return NULL;
	}
	for (;;) {
		Token next = Scanner_peek(scanner);
		const Oplevel *op2 = is_op_token(next);
		if (op2 <= op) {
			return left;
		}
		switch (op2->assoc) {
			case LeftAssoc:
				left = parse_lassoc(scanner, op2, left, a);
				break;
			case RightAssoc:
				left = parse_rassoc(scanner, op2, left, a);
				break;
			case NoneAssoc:
				left = parse_nassoc(scanner, op2, left, a);
				break;
		}
		if (!left) {
			return NULL;
		}
	}
	return NULL;
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
