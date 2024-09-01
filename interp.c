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
#include "arena.h"


#define TMP_ARENA_PAGE_SIZE 4096

int main(int argc, char **argv)
{
	if (!parse_args(argc, argv)) {
		return 1;
	}
	int tty = isatty(0);
	Scanner scanner = Scanner_make(stdin);
	Context ctx = Context_make();
	Arena tmp = Arena_make(TMP_ARENA_PAGE_SIZE);
	while (!Scanner_eof(scanner)) {
		Arena_reset(&tmp);
		if (tty) {
			fprintf(stderr, "> ");
		}
		Node *ast = parse(&scanner, &tmp);
		if (!ast) {
			continue;
		}
		Type *type = NULL;
		if (typed) {
			type = infer(ast, &ctx, &tmp);
			if (!type) {
				continue;
			}
		}
		if (debug) {
			Node_println(ast);
		}
		Object *result = eval(ast, &ctx);
		if (!result) {
			continue;
		}
		Object_print(result);
		if (type) {
			printf(" :: ");
			Type_print(type);
		}
		printf("\n");
	}
	Scanner_destroy(scanner);
	Context_destroy(ctx);
	Arena_destroy(tmp);
	return 0;
}
