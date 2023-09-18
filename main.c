#include <stdio.h>
#include <unistd.h>

#include "opts.h"
#include "input.h"
#include "iter.h"
#include "scanner.h"
#include "parse.h"
#include "node.h"
#include "infer.h"
#include "types.h"
#include "eval.h"
#include "debug.h"


int main(int argc, char **argv)
{
	int optind = setopts(argc, argv);
	if (optind < 0) {
		fprintf(stderr, "usage: %s [-dlt]\n", argv[0]);
		return 1;
	}
	int tty = isatty(0);
	Context ctx = Context_make();
	for (;;) {
		if (tty) {
			fprintf(stderr, "> ");
		}
		const char *input = get_line();
		if (!input) {
			break;
		}
		Scanner scanner = Scanner_make(&input);
		Node *ast = parse(&scanner);
		if (!ast) {
			continue;
		}
		Type *type = NULL;
		if (typed) {
			type = infer(ast, &ctx);
			if (!type) {
				Node_drop(ast);
				continue;
			}
		}
		if (debug) {
			Node_print(ast);
		}
		Object *result = eval(ast, &ctx);
		if (result) {
			if (typed) {
				Object_print(result);
				printf(" :: ");
				Type_println(type);
				Type_drop(type);
			} else {
				Object_println(result);
			}
		}
	}
	Context_destroy(ctx);
	return 0;
}
