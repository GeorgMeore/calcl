#include <stdio.h>
#include <unistd.h>

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
		fprintf(stderr, "usage: %s [-dl]\n", argv[0]);
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
		if (debug) {
			print_expr(ast);
		}
		Object *result = NULL;
		if (lazy) {
			GC_collect(ctx.gc, ctx.root, NULL);
			result = leval(ast, ctx.gc, ctx.root);
			Node_drop(ast);
		} else {
			result = seval(ast, &ctx);
		}
		if (result) {
			Object_println(result);
		}
	}
	Context_destroy(ctx);
	return 0;
}
