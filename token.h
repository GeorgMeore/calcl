#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

typedef enum {
	NumberToken,
	IdToken,
	LparenToken,
	RparenToken,
	CaretToken,
	AsteriskToken,
	SlashToken,
	PercentToken,
	PlusToken,
	MinusToken,
	GtToken,
	LtToken,
	EqToken,
	AndToken,
	OrToken,
	IfToken,
	ThenToken,
	ElseToken,
	FnToken,
	ColonToken,
	LetToken,
	ErrorToken,
	EndToken,
} TokenType;

#define TOKEN_COUNT (EndToken + 1)

typedef struct {
	TokenType  type;
	const char *string;
	int        length;
} Token;

void Token_print(Token token);

#endif // TOKEN_INCLUDED
