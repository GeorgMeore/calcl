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
		return (Token){IfToken, start, length};
	}
	if (kweq(start, "then", length)) {
		return (Token){ThenToken, start, length};
	}
	if (kweq(start, "else", length)) {
		return (Token){ElseToken, start, length};
	}
	if (kweq(start, "or", length)) {
		return (Token){OrToken, start, length};
	}
	if (kweq(start, "and", length)) {
		return (Token){AndToken, start, length};
	}
	if (kweq(start, "fn", length)) {
		return (Token){FnToken, start, length};
	}
	if (kweq(start, "let", length)) {
		return (Token){LetToken, start, length};
	}
	return (Token){IdToken, start, length};
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
	return (Token){NumberToken, start, Iter_cursor(iterator) - start};
}

TokenType singlet_token_type(char c)
{
	switch (c) {
		case '(':  return LparenToken;
		case ')':  return RparenToken;
		case '+':  return PlusToken;
		case '-':  return MinusToken;
		case '*':  return AsteriskToken;
		case '/':  return SlashToken;
		case '%':  return PercentToken;
		case '^':  return CaretToken;
		case '>':  return GtToken;
		case '<':  return LtToken;
		case '=':  return EqToken;
		case ':':  return ColonToken;
		default:   return ErrorToken;
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
		Token token = (Token){EndToken, Iter_cursor(iterator), 1};
		Iter_next(iterator);
		return token;
	} else if (isdigit(next)) {
		return take_number(iterator);
	} else if (isalpha(next)) {
		return take_keyword_or_id(iterator);
	} else {
		Token token = {singlet_token_type(next), Iter_cursor(iterator), 1};
		Iter_next(iterator);
		if (token.type == ErrorToken) {
			while (Iter_peek(iterator) != '\n' && Iter_peek(iterator) != '\0') {
				Iter_next(iterator);
			}
		}
		return token;
	}
}
