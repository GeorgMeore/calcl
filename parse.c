#include "parse.h"

#include <stdio.h>
#include <ctype.h>

#include "scanner.h"
#include "node.h"
#include "token.h"
#include "error.h"
#include "arena.h"


// TODO: make the syntax not line-oriented

#define ERROR_PREFIX "parsing error"

static void tokerror(const char *message, Token last)
{
	if (last.type == END_TOKEN) {
		errorf("%s (while parsing 'END')", message);
	} else {
		errorf("%s (while parsing '%.*s')", message, last.length, last.string);
	}
}

#define LEFT_ASSOC  0
#define RIGHT_ASSOC 1
#define NONE_ASSOC  2

typedef struct {
	int       assoc;
	NodeType  optype;
	int       count;
	TokenType types[TOKEN_COUNT];
} Oplevel;

// Binary operators in the order of increasing precedence
static const Oplevel optable[] = {
	{LEFT_ASSOC,  OR_NODE,      1, {OR_TOKEN}},
	{LEFT_ASSOC,  AND_NODE,     1, {AND_TOKEN}},
	{NONE_ASSOC,  CMP_NODE,     3, {GT_TOKEN, LT_TOKEN, EQ_TOKEN}},
	{LEFT_ASSOC,  SUM_NODE,     2, {PLUS_TOKEN, MINUS_TOKEN}},
	{LEFT_ASSOC,  PRODUCT_NODE, 3, {ASTERISK_TOKEN, SLASH_TOKEN, PERCENT_TOKEN}},
	{RIGHT_ASSOC, EXPT_NODE,    1, {CARET_TOKEN}},
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
static int is_term_token(Token token) {
	return (
		token.type == LPAREN_TOKEN ||
		token.type == NUMBER_TOKEN ||
		token.type == ID_TOKEN
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
	if (next.type == END_TOKEN) {
		return NULL;
	}
	Node *expr = NULL;
	if (next.type == LET_TOKEN) {
		expr = parse_let(scanner, a);
	} else {
		expr = parse_expression(scanner, a);
	}
	if (!expr) {
		Scanner_seek(scanner, END_TOKEN);
		return NULL;
	}
	next = Scanner_next(scanner);
	if (next.type != END_TOKEN) {
		tokerror("unexpected token after the expression", next);
		Scanner_seek(scanner, END_TOKEN);
		return NULL;
	}
	return expr;
}

// LET_VALUE ::= '=' EXPRESSION | 'ID' LET_VALUE
static Node *parse_let_value(Scanner *scanner, Arena *a)
{
	Token next = Scanner_next(scanner);
	if (next.type == EQ_TOKEN) {
		return parse_expression(scanner, a);
	} else if (next.type == ID_TOKEN) {
		Node *param = IdNode_new(a, next.string, next.length);
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
	Scanner_next(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != ID_TOKEN) {
		tokerror("expected identifier", next);
		return NULL;
	}
	Node *name = IdNode_new(a, next.string, next.length);
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
	if (next.type == IF_TOKEN) {
		return parse_if(scanner, a);
	}
	if (next.type == FN_TOKEN) {
		return parse_fn(scanner, a);
	}
	return parse_op(scanner, 0, a);
}

// FN_BODY ::= ':' EXPRESSION | 'ID' FN_BODY
static Node *parse_fn_body(Scanner *scanner, Arena *a)
{
	Token next = Scanner_next(scanner);
	if (next.type == COLON_TOKEN) {
		return parse_expression(scanner, a);
	} else if (next.type == ID_TOKEN) {
		Node *param = IdNode_new(a, next.string, next.length);
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
	Scanner_next(scanner);
	Token next = Scanner_next(scanner);
	if (next.type != ID_TOKEN) {
		tokerror("expected identifier", next);
		return NULL;
	}
	Node *param = IdNode_new(a, next.string, next.length);
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
	if (next.type == ELSE_TOKEN) {
		Scanner_next(scanner);
		return parse_expression(scanner, a);
	} else if (next.type == IF_TOKEN) {
		return parse_if(scanner, a);
	} else {
		tokerror("expected 'if' or 'else'", next);
		return NULL;
	}
}

// IF ::= 'IF OP 'THEN' EXPRESSION IF_TAIL
static Node *parse_if(Scanner *scanner, Arena *a)
{
	Scanner_next(scanner);
	Node *cond = parse_op(scanner, 0, a);
	if (!cond) {
		return NULL;
	}
	Token next = Scanner_next(scanner);
	if (next.type != THEN_TOKEN) {
		tokerror("expected 'then'", next);
		return NULL;
	}
	Node *true = parse_expression(scanner, a);
	if (!true) {
		return NULL;
	}
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
		case LEFT_ASSOC:
			return parse_lassoc(scanner, op, prec, a);
		case RIGHT_ASSOC:
			return parse_rassoc(scanner, op, prec, a);
		case NONE_ASSOC:
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
	if (next.type == MINUS_TOKEN) {
		Node *term = parse_term(scanner, a);
		if (!term) {
			return NULL;
		}
		return OpNode_new(a, term, NumberNode_new(a, -1), PRODUCT_NODE, '*');
	}
	if (next.type == LPAREN_TOKEN) {
		Node *expr = parse_expression(scanner, a);
		if (!expr) {
			return NULL;
		}
		next = Scanner_next(scanner);
		if (next.type != RPAREN_TOKEN) {
			tokerror("expected ')'", next);
			return NULL;
		}
		return expr;
	} else if (next.type == NUMBER_TOKEN) {
		return parse_number(next, a);
	} else if (next.type == ID_TOKEN) {
		return IdNode_new(a, next.string, next.length);
	} else {
		tokerror("expected '(', '-' or a number", next);
		return NULL;
	}
}
