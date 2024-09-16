#include "opts.h"

#include "error.h"


#define ERROR_PREFIX "arguments error"

#define DEBUG_DEFAULT 0
#define LAZY_DEFAULT  0
#define TYPED_DEFAULT 0

int debug = DEBUG_DEFAULT;
int lazy  = LAZY_DEFAULT;
int typed = TYPED_DEFAULT;

#define usage(name) \
	(fprintf(stderr, "usage: %s [-dlt]\n", name))

int parse_args(int argc, char **argv)
{
	for (int optind = 1; optind < argc; optind++) {
		char *arg = argv[optind];
		if (arg[0] != '-') {
			errorf("unexpected positional argument: '%s'", arg);
			usage(argv[0]);
			return 0;
		}
		for (arg++; *arg; arg++) {
			switch (*arg) {
				case 'd': debug = 1; break;
				case 'l': lazy = 1;  break;
				case 't': typed = 1; break;
				default:
					errorf("unknown flag: '%s'", arg);
					usage(argv[0]);
					return 0;
			}
		}
	}
	return 1;
}
