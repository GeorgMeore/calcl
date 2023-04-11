#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

typedef enum {
	NUMBER_TOKEN,
	ID_TOKEN,
	LPAREN_TOKEN,
	RPAREN_TOKEN,
	CARET_TOKEN,
	ASTERISK_TOKEN,
	PLUS_TOKEN,
	GT_TOKEN,
	AND_TOKEN,
	OR_TOKEN,
	IF_TOKEN,
	THEN_TOKEN,
	ELSE_TOKEN,
	ERROR_TOKEN,
	END_TOKEN
} TokenType;

typedef struct {
	TokenType type;
	const char *string;
	int length;
} Token;

#endif // TOKEN_INCLUDED
