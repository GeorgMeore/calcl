#include <stdio.h>

#include "opts.h"
#include "input.h"
#include "iter.h"
#include "lex.h"
#include "scanner.h"
#include "parse.h"
#include "node.h"
#include "eval.h"
#include "env.h"
#include "gc.h"
#include "debug.h"


int main(int argc, char **argv)
{
	int optind = setopts(argc, argv);
	if (optind < 0) {
		fprintf(stderr, "usage: %s [-d]", argv[0]);
		return 1;
	}
	GC *gc = GC_new();
	Object *root = GC_alloc_env(gc);
	for (;;) {
		GC_collect(gc, root);
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
		Object *result = eval(ast, gc, root);
		if (result) {
			Object_println(result);
		}
		Node_drop(ast);
	}
	GC_collect(gc, NULL);
	GC_drop(gc);
	return 0;
}
