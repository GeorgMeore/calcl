#include <stdio.h>
#include <unistd.h>

#include "opts.h"
#include "iter.h"
#include "scanner.h"
#include "parse.h"
#include "node.h"
#include "infer.h"
#include "types.h"
#include "eval.h"


int main(int argc, char **argv)
{
	if (!parse_args(argc, argv)) {
		return 1;
	}
	int tty = isatty(0);
	Scanner scanner = Scanner_make(stdin);
	Context ctx = Context_make();
	while (!Scanner_eof(scanner)) {
		if (tty) {
			fprintf(stderr, "> ");
		}
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
			Node_println(ast);
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
	Scanner_destroy(scanner);
	Context_destroy(ctx);
	return 0;
}
