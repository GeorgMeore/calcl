#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lex.h"
#include "iter.h"

#define error(fmt, args...) fprintf(stderr, "lexical error: "fmt"\n", args)

// [0-9]+
Token take_number(CharIterator *iterator)
{
	const char *start = CharIterator_cursor(iterator);
	for (;;) {
		switch (CharIterator_peek(iterator)) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				CharIterator_next(iterator);
				break;
			default:
				Token number = {
					NUMBER_TOKEN,
					start,
					CharIterator_cursor(iterator) - start
				};
				return number;
		}
	}
}

// [()+*]
Token take_separator(CharIterator *iterator)
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
	}
	Token separator = {
		type,
		CharIterator_cursor(iterator) - 1,
		1
	};
	return separator;
}

// [ \t\n]+
void take_spaces(CharIterator *iterator)
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

// (spaces | separator | number)* '\0'
Token *tokenize(const char *input)
{
	// There can't be more tokens than characters
	Token *tokens = malloc(sizeof(*tokens) * (strlen(input) + 1));
	CharIterator iterator = input;
	int lexed = 0;
	while (CharIterator_peek(&iterator)) {
		Token new;
		switch (CharIterator_peek(&iterator)) {
			case ' ': case '\t': case '\n':
				take_spaces(&iterator);
				continue;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				new = take_number(&iterator);
				break;
			case '(': case ')': case '+': case '*':
				new = take_separator(&iterator);
				break;
			default:
				error("unexpected character: %c", CharIterator_peek(&iterator));
				return NULL;
		}
		tokens[lexed] = new;
		lexed += 1;
	}
	Token eof = {EOF_TOKEN, CharIterator_cursor(&iterator), 0};
	tokens[lexed] = eof;
	return tokens;
}
