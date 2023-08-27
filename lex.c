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
	while (isalpha(CharIterator_peek(iterator))) {
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

// token <- number | id | keyword | '(' | ')' | '+' | '*' | '^' | '>' | ':' '\0'
Token take_token(CharIterator *iterator)
{
	// skip leading spaces
	while (isspace(CharIterator_peek(iterator))) {
		CharIterator_next(iterator);
	}
	if (isdigit(CharIterator_peek(iterator))) {
		return take_number(iterator);
	}
	if (isalpha(CharIterator_peek(iterator))) {
		return take_keyword_or_id(iterator);
	}
	char next = CharIterator_next(iterator);
	if (next == '(') {
		return (Token){LPAREN_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == ')') {
		return (Token){RPAREN_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == '+') {
		return (Token){PLUS_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == '*') {
		return (Token){ASTERISK_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == '^') {
		return (Token){CARET_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == '>') {
		return (Token){GT_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == ':') {
		return (Token){COLON_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == '=') {
		return (Token){EQ_TOKEN, CharIterator_cursor(iterator), 1};
	}
	if (next == '\0') {
		return (Token){END_TOKEN, CharIterator_cursor(iterator), 0};
	}
	return (Token){ERROR_TOKEN, CharIterator_cursor(iterator), 1};
}
