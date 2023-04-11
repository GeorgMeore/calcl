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
		Node *ast = parse(&scanner);
		if (!ast) {
			continue;
		}
		if (debug) {
			print_expr(ast);
		}
		double result;
		if (eval(ast, &result)) {
			printf("%lf\n", result);
		}
		Node_drop(ast);
	}
	return 0;
}
