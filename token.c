#include "token.h"

#include <stdio.h>


#define tokprintf(name, token) \
	(printf("<%s|'%.*s'>", name, token.length, token.string))

void Token_print(Token token)
{
	switch (token.type) {
		case NumberToken:   tokprintf("number", token);   break;
		case IdToken:       tokprintf("id", token);       break;
		case LparenToken:   tokprintf("lparen", token);   break;
		case RparenToken:   tokprintf("rparen", token);   break;
		case CaretToken:    tokprintf("caret", token);    break;
		case AsteriskToken: tokprintf("asterisk", token); break;
		case SlashToken:    tokprintf("slash", token);    break;
		case PercentToken:  tokprintf("percent", token);  break;
		case PlusToken:     tokprintf("plus", token);     break;
		case MinusToken:    tokprintf("minus", token);    break;
		case GtToken:       tokprintf("gt", token);       break;
		case LtToken:       tokprintf("lt", token);       break;
		case EqToken:       tokprintf("eq", token);       break;
		case AndToken:      tokprintf("and", token);      break;
		case OrToken:       tokprintf("or", token);       break;
		case IfToken:       tokprintf("if", token);       break;
		case ThenToken:     tokprintf("then", token);     break;
		case ElseToken:     tokprintf("else", token);     break;
		case FnToken:       tokprintf("fn", token);       break;
		case ColonToken:    tokprintf("colon", token);    break;
		case LetToken:      tokprintf("let", token);      break;
		case ErrorToken:    tokprintf("error", token);    break;
		case EndToken:      tokprintf("end", token);      break;
	}
}
