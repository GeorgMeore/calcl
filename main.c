#include <stdio.h>
#include "input.h"
#include "lex.h"
#include "parse.h"
#include "eval.h"

int main()
{
	for (;;) {
		char *input = get_line();
		if (!input) {
			break;
		}
		CharIterator iter = input;
		Scanner scanner = Scanner_make(&iter);
		Node *expr = parse(&scanner);
		if (!expr) {
			continue;
		}
		int result;
		evaluate(expr, &result);
		printf("%d\n", result);
		Node_drop(expr);
	}
	return 0;
}
