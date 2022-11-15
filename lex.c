#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lex.h"
#include "iter.h"

#define error(fmt, args...) fprintf(stderr, "lexical error: "fmt"\n", args)

// [0-9]+
Token take_number(CharIterator *i)
{
	const char *start = CharIterator_cursor(i);
	for (;;) {
		switch (CharIterator_peek(i)) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				CharIterator_next(i);
				break;
			default:
				Token number = {NUMBER_TOKEN, start, CharIterator_cursor(i) - start};
				return number;
		}
	}
}

// [()+*]
Token take_separator(CharIterator *i)
{
	TokenType type;
	switch (CharIterator_next(i)) {
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
	}
	Token separator = {type, CharIterator_cursor(i) - 1, 1};
	return separator;
}

// [ \t\n]+
void take_spaces(CharIterator *i)
{
	for (;;) {
		switch (CharIterator_peek(i)) {
			case ' ': case '\t': case '\n':
				CharIterator_next(i);
			default:
				return;
		}
	}
}

// (spaces | separator | number)* '\0'
Token *tokenize(const char *input)
{
	// There can't be more tokens than characters
	Token *tokens = malloc(sizeof(*tokens) * (strlen(input) + 1));
	CharIterator i = input;
	int lexed = 0;
	while (CharIterator_peek(&i)) {
		Token new;
		switch (CharIterator_peek(&i)) {
			case ' ': case '\t': case '\n':
				take_spaces(&i);
				continue;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				new = take_number(&i);
				break;
			case '(': case ')': case '+': case '*':
				new = take_separator(&i);
				break;
			default:
				error("unexpected character: %c", CharIterator_peek(&i));
				return NULL;
		}
		tokens[lexed] = new;
		lexed += 1;
	}
	Token eof = {EOF_TOKEN, CharIterator_cursor(&i), 0};
	tokens[lexed] = eof;
	return tokens;
}
