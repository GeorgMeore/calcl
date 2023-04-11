#include "lex.h"

#include <ctype.h>
#include <string.h>

#include "token.h"
#include "iter.h"


// keyword <- 'if' | 'then' | 'else' | 'or' | 'and'
// id <- alpha+
static Token take_keyword_or_id(CharIterator *iterator)
{
	const char *start = CharIterator_cursor(iterator);
	while (isalpha(CharIterator_peek(iterator))) {
		CharIterator_next(iterator);
	}
	long unsigned length = CharIterator_cursor(iterator) - start;
	if (!strncmp(start, "if", length)) {
		Token ift = {IF_TOKEN, start, length};
		return ift;
	}
	if (!strncmp(start, "then", length)) {
		Token then = {THEN_TOKEN, start, length};
		return then;
	}
	if (!strncmp(start, "else", length)) {
		Token elset = {ELSE_TOKEN, start, length};
		return elset;
	}
	if (!strncmp(start, "or", length)) {
		Token or = {OR_TOKEN, start, length};
		return or;
	}
	if (!strncmp(start, "and", length)) {
		Token and = {AND_TOKEN, start, length};
		return and;
	} else {
		Token id = {ID_TOKEN, start, length};
		return id;
	}
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
	Token number = {NUMBER_TOKEN, start, CharIterator_cursor(iterator) - start};
	return number;
}

// token <- number | id | keyword | '(' | ')' | '+' | '*' | '^' | '>' | '\0'
Token take_token(CharIterator *iterator)
{
	// skip leading spaces
	while (isspace(CharIterator_peek(iterator))) {
		CharIterator_next(iterator);
	}
	char next = CharIterator_peek(iterator);
	if (isdigit(next)) {
		return take_number(iterator);
	}
	if (isalpha(next)) {
		return take_keyword_or_id(iterator);
	}
	if (next == '(') {
		Token lparen = {LPAREN_TOKEN, CharIterator_cursor(iterator), 1};
		CharIterator_next(iterator);
		return lparen;
	}
	if (next == ')') {
		Token rparen = {RPAREN_TOKEN, CharIterator_cursor(iterator), 1};
		CharIterator_next(iterator);
		return rparen;
	}
	if (next == '+') {
		Token plus = {PLUS_TOKEN, CharIterator_cursor(iterator), 1};
		CharIterator_next(iterator);
		return plus;
	}
	if (next == '*') {
		Token asterisk = {ASTERISK_TOKEN, CharIterator_cursor(iterator), 1};
		CharIterator_next(iterator);
		return asterisk;
	}
	if (next == '^') {
		Token caret = {CARET_TOKEN, CharIterator_cursor(iterator), 1};
		CharIterator_next(iterator);
		return caret;
	}
	if (next == '>') {
		Token gt = {GT_TOKEN, CharIterator_cursor(iterator), 1};
		CharIterator_next(iterator);
		return gt;
	}
	if (next == '\0') {
		Token eof = {END_TOKEN, CharIterator_cursor(iterator), 0};
		CharIterator_next(iterator);
		return eof;
	}
	Token error = {ERROR_TOKEN, CharIterator_cursor(iterator), 1};
	return error;
}
