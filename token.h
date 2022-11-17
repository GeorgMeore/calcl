#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

typedef enum {
	NUMBER_TOKEN,
	LPAREN_TOKEN,
	RPAREN_TOKEN,
	CARET_TOKEN,
	ASTERISK_TOKEN,
	PLUS_TOKEN,
	END_TOKEN,
	ERROR_TOKEN
} TokenType;

typedef struct {
	TokenType type;
	const char *string;
	int length;
} Token;

#endif // TOKEN_INCLUDED
