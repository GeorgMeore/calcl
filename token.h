#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

typedef enum TokenType TokenType;
typedef struct Token Token;

enum TokenType {
	LPAREN_TOKEN,
	RPAREN_TOKEN,
	ASTERISK_TOKEN,
	PLUS_TOKEN,
	NUMBER_TOKEN,
	EOF_TOKEN,
};

struct Token {
	TokenType type;
	const char *string;
	int length;
};

#endif // TOKEN_INCLUDED
