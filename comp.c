#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include "iter.h"
#include "scanner.h"
#include "parse.h"
#include "node.h"
#include "infer.h"
#include "types.h"
#include "arena.h"
#include "codegen.h"


#define TMP_ARENA_PAGE_SIZE 4096

int main(void)
{
	Scanner scanner = Scanner_make(stdin);
	Context ctx = Context_make();
	Arena tmp = Arena_make(TMP_ARENA_PAGE_SIZE);
	compile_begin();
	while (!Scanner_eof(scanner)) {
		Arena_reset(&tmp);
		Node *ast = parse(&scanner, &tmp);
		if (!ast) {
			break;
		}
		Type *type = infer(ast, &ctx, &tmp);
		if (!type) {
			break;
		}
		compile(ast);
	}
	compile_end();
	Scanner_destroy(scanner);
	Context_destroy(ctx);
	Arena_destroy(tmp);
	return 0;
}
