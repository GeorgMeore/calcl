#include "lex.h"

#include <ctype.h>
#include <string.h>

#include "token.h"
#include "iter.h"


#define kweq(s, keyword, length) \
	(length == sizeof(keyword) - 1 && !strncmp(s, keyword, length))


// keyword <- 'if' | 'then' | 'else' | 'or' | 'and' | 'fn' | 'to'
// id <- alpha+
static Token take_keyword_or_id(Iter *iterator)
{
	const char *start = Iter_cursor(iterator);
	while (isalnum(Iter_peek(iterator))) {
		Iter_next(iterator);
	}
	long unsigned length = Iter_cursor(iterator) - start;
	if (kweq(start, "if", length)) {
		return (Token){IF_TOKEN, start, length};
	}
	if (kweq(start, "then", length)) {
		return (Token){THEN_TOKEN, start, length};
	}
	if (kweq(start, "else", length)) {
		return (Token){ELSE_TOKEN, start, length};
	}
	if (kweq(start, "or", length)) {
		return (Token){OR_TOKEN, start, length};
	}
	if (kweq(start, "and", length)) {
		return (Token){AND_TOKEN, start, length};
	}
	if (kweq(start, "fn", length)) {
		return (Token){FN_TOKEN, start, length};
	}
	if (kweq(start, "let", length)) {
		return (Token){LET_TOKEN, start, length};
	}
	return (Token){ID_TOKEN, start, length};
}

// number <- digit+ ('.' digit*)?
static Token take_number(Iter *iterator)
{
	const char *start = Iter_cursor(iterator);
	while (isdigit(Iter_peek(iterator))) {
		Iter_next(iterator);
	}
	if (Iter_peek(iterator) == '.') {
		Iter_next(iterator);
	}
	while (isdigit(Iter_peek(iterator))) {
		Iter_next(iterator);
	}
	return (Token){NUMBER_TOKEN, start, Iter_cursor(iterator) - start};
}

TokenType singlet_token_type(char c)
{
	switch (c) {
		case '(':  return LPAREN_TOKEN;
		case ')':  return RPAREN_TOKEN;
		case '+':  return PLUS_TOKEN;
		case '-':  return MINUS_TOKEN;
		case '*':  return ASTERISK_TOKEN;
		case '/':  return SLASH_TOKEN;
		case '%':  return PERCENT_TOKEN;
		case '^':  return CARET_TOKEN;
		case '>':  return GT_TOKEN;
		case '<':  return LT_TOKEN;
		case '=':  return EQ_TOKEN;
		case ':':  return COLON_TOKEN;
		case '!':  return BANG_TOKEN;
		default:   return ERROR_TOKEN;
	}
}

// token <- number | id | keyword | '(' | ')' | '+' | '*' | '/' | '^' | '>' | '<' | '=' | ':' | '\0'
Token take_token(Iter *iterator)
{
	while (Iter_peek(iterator) == ' ' || Iter_peek(iterator) == '\t') {
		Iter_next(iterator);
	}
	if (Iter_peek(iterator) == '#') {
		while (Iter_peek(iterator) != '\n' && Iter_peek(iterator) != '\0') {
			Iter_next(iterator);
		}
	}
	char next = Iter_peek(iterator);
	if (next == '\0' || next == '\n') {
		return (Token){END_TOKEN, Iter_cursor(iterator), 0};
	} else if (isdigit(next)) {
		return take_number(iterator);
	} else if (isalpha(next)) {
		return take_keyword_or_id(iterator);
	} else {
		Token token = {singlet_token_type(next), Iter_cursor(iterator), 1};
		Iter_next(iterator);
		if (token.type == ERROR_TOKEN) {
			while (Iter_peek(iterator) != '\n' && Iter_peek(iterator) != '\0') {
				Iter_next(iterator);
			}
		}
		return token;
	}
}
