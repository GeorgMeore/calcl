#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include "scanner.h"
#include "parse.h"
#include "node.h"
#include "infer.h"
#include "types.h"
#include "arena.h"
#include "codegen.h"
#include "opts.h"


#define TMP_ARENA_PAGE_SIZE 4096

// TODO: proper error handling

int main(int argc, char **argv)
{
	if (!parse_args(argc, argv)) {
		return 1;
	}
	Scanner scanner = Scanner_make(stdin);
	Arena tmp = Arena_make(TMP_ARENA_PAGE_SIZE);
	TypeEnv *tenv = TYPEENV_EMPTY;
	compile_begin();
	while (!Scanner_eof(scanner)) {
		Arena_reset(&tmp);
		Node *ast = parse(&scanner, &tmp);
		if (!ast) {
			continue;
		}
		if (typed) {
			Type *type = infer(ast, &tenv, &tmp);
			if (!type) {
				break;
			}
		}
		compile(ast);
	}
	compile_end();
	Scanner_destroy(scanner);
	TypeEnv_drop(tenv);
	Arena_destroy(tmp);
	return 0;
}
