#include <stdio.h>

#include "opts.h"
#include "input.h"
#include "iter.h"
#include "lex.h"
#include "scanner.h"
#include "parse.h"
#include "node.h"
#include "eval.h"
#include "debug.h"


int main(int argc, char **argv)
{
	int optind = setopts(argc, argv);
	if (optind < 0) {
		fprintf(stderr, "usage: %s [-d]", argv[0]);
		return 1;
	}
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
		if (debug) {
			print_expr(expr);
		}
		int result;
		if (eval(expr, &result)) {
			printf("%d\n", result);
		}
		Node_drop(expr);
	}
	return 0;
}
