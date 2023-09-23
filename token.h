#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

typedef enum {
	NUMBER_TOKEN,
	ID_TOKEN,
	LPAREN_TOKEN,
	RPAREN_TOKEN,
	CARET_TOKEN,
	ASTERISK_TOKEN,
	SLASH_TOKEN,
	PERCENT_TOKEN,
	PLUS_TOKEN,
	MINUS_TOKEN,
	GT_TOKEN,
	LT_TOKEN,
	EQ_TOKEN,
	AND_TOKEN,
	OR_TOKEN,
	IF_TOKEN,
	THEN_TOKEN,
	ELSE_TOKEN,
	FN_TOKEN,
	COLON_TOKEN,
	LET_TOKEN,
	ERROR_TOKEN,
	END_TOKEN
} TokenType;

typedef struct {
	TokenType  type;
	const char *string;
	int        length;
} Token;

void Token_print(Token token);

#endif // TOKEN_INCLUDED
