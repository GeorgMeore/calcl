#include "lex.h"

#include <ctype.h>

#include "token.h"
#include "iter.h"


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

// [()+*^]
static Token take_singlet(CharIterator *iterator)
{
	TokenType type;
	switch (CharIterator_next(iterator)) {
		case '(':
			type = LPAREN_TOKEN;
			break;
		case ')':
			type = RPAREN_TOKEN;
			break;
		case '+':
			type = PLUS_TOKEN;
			break;
		case '*':
			type = ASTERISK_TOKEN;
			break;
		case '^':
			type = CARET_TOKEN;
	}
	Token separator = {type, CharIterator_cursor(iterator) - 1, 1};
	return separator;
}

static int issinglet(char c)
{
	switch (c) {
		case '(': case ')': case '+': case '*': case '^':
			return 1;
		default:
			return 0;
	}
}

static void take_spaces(CharIterator *iterator)
{
	for (;;) {
		switch (CharIterator_peek(iterator)) {
			case ' ': case '\t': case '\n':
				CharIterator_next(iterator);
			default:
				return;
		}
	}
}

// singlet | number | '\0'
Token take_token(CharIterator *iterator)
{
	take_spaces(iterator);
	char next = CharIterator_peek(iterator);
	if (isdigit(next)) {
		return take_number(iterator);
	}
	if (issinglet(next)) {
		return take_singlet(iterator);
	}
	if (next == '\0') {
		Token eof = {END_TOKEN, CharIterator_cursor(iterator), 0};
		return eof;
	}
	Token error = {ERROR_TOKEN, CharIterator_cursor(iterator), 1};
	return error;
}
