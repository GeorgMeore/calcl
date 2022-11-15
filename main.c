#include <stdio.h>
#include "lex.h"
#include "parse.h"
#include "eval.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}
	Token *tokens = tokenize(argv[1]);
	if (!tokens) {
		return 1;
	}
	Node *expr = parse(tokens);
	if (!expr) {
		return 1;
	}
	int result;
	evaluate(expr, &result);
	printf("%d\n", result);
	return 0;
}
