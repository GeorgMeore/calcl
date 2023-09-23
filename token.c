#include "token.h"

#include <stdio.h>


#define tokprintf(name, token) \
	(printf("<%s|'%.*s'>", name, token.length, token.string))

void Token_print(Token token)
{
	switch (token.type) {
		case NUMBER_TOKEN:   tokprintf("number", token);   break;
		case ID_TOKEN:       tokprintf("id", token);       break;
		case LPAREN_TOKEN:   tokprintf("lparen", token);   break;
		case RPAREN_TOKEN:   tokprintf("rparen", token);   break;
		case CARET_TOKEN:    tokprintf("caret", token);    break;
		case ASTERISK_TOKEN: tokprintf("asterisk", token); break;
		case SLASH_TOKEN:    tokprintf("slash", token);    break;
		case PERCENT_TOKEN:  tokprintf("percent", token);  break;
		case PLUS_TOKEN:     tokprintf("plus", token);     break;
		case MINUS_TOKEN:    tokprintf("minus", token);    break;
		case GT_TOKEN:       tokprintf("gt", token);       break;
		case LT_TOKEN:       tokprintf("lt", token);       break;
		case EQ_TOKEN:       tokprintf("eq", token);       break;
		case AND_TOKEN:      tokprintf("and", token);      break;
		case OR_TOKEN:       tokprintf("or", token);       break;
		case IF_TOKEN:       tokprintf("if", token);       break;
		case THEN_TOKEN:     tokprintf("then", token);     break;
		case ELSE_TOKEN:     tokprintf("else", token);     break;
		case FN_TOKEN:       tokprintf("fn", token);       break;
		case COLON_TOKEN:    tokprintf("colon", token);    break;
		case LET_TOKEN:      tokprintf("let", token);      break;
		case ERROR_TOKEN:    tokprintf("error", token);    break;
		case END_TOKEN:      tokprintf("end", token);      break;
	}
}
