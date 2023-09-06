#include "lex.h"

#include <ctype.h>
#include <string.h>

#include "token.h"
#include "iter.h"


#define kweq(s, keyword, length) \
	(length == sizeof(keyword) - 1 && !strncmp(s, keyword, length))


// keyword <- 'if' | 'then' | 'else' | 'or' | 'and' | 'fn' | 'to'
// id <- alpha+
static Token take_keyword_or_id(CharIterator *iterator)
{
	const char *start = CharIterator_cursor(iterator);
	while (isalnum(CharIterator_peek(iterator))) {
		CharIterator_next(iterator);
	}
	long unsigned length = CharIterator_cursor(iterator) - start;
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
static Token take_number(CharIterator *iterator)
{
	const char *start = CharIterator_cursor(iterator);
	while (isdigit(CharIterator_peek(iterator))) {
		CharIterator_next(iterator);
	}
	if (CharIterator_peek(iterator) == '.') {
		CharIterator_next(iterator);
	}
	while (isdigit(CharIterator_peek(iterator))) {
		CharIterator_next(iterator);
	}
	return (Token){NUMBER_TOKEN, start, CharIterator_cursor(iterator) - start};
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
		case '^':  return CARET_TOKEN;
		case '>':  return GT_TOKEN;
		case '<':  return LT_TOKEN;
		case '=':  return EQ_TOKEN;
		case ':':  return COLON_TOKEN;
		case '\0': return END_TOKEN;
		default:   return ERROR_TOKEN;
	}
}

// token <- number | id | keyword | '(' | ')' | '+' | '*' | '/' | '^' | '>' | '<' | '=' | ':' | '\0'
Token take_token(CharIterator *iterator)
{
	// skip leading spaces
	while (isspace(CharIterator_peek(iterator))) {
		CharIterator_next(iterator);
	}
	char next = CharIterator_peek(iterator);
	if (next == '#') {
		return (Token){END_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (isdigit(next)) {
		return take_number(iterator);
	}
	if (isalpha(next)) {
		return take_keyword_or_id(iterator);
	}
	Token token = {singlet_token_type(next), CharIterator_cursor(iterator), 1};
	CharIterator_next(iterator);
	return token;
}
