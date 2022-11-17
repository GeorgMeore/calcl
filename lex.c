#include "lex.h"

#include "token.h"
#include "iter.h"


// [0-9]+
static Token take_number(CharIterator *iterator)
{
	const char *start = CharIterator_cursor(iterator);
	for (;;) {
		switch (CharIterator_peek(iterator)) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				CharIterator_next(iterator);
				break;
			default:
				Token number = {NUMBER_TOKEN, start, CharIterator_cursor(iterator) - start};
				return number;
		}
	}
}

// [()+*]
static Token take_separator(CharIterator *iterator)
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

// [ \t\n]+
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

// spaces? (separator | number)* '\0'
Token take_token(CharIterator *iterator)
{
	take_spaces(iterator);
	switch (CharIterator_peek(iterator)) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return take_number(iterator);
		case '(': case ')': case '+': case '*': case '^':
			return take_separator(iterator);
		case '\0':
			Token eof = {END_TOKEN, CharIterator_cursor(iterator), 0};
			return eof;
		default:
			Token error = {ERROR_TOKEN, CharIterator_cursor(iterator), 1};
			return error;
	}
}
