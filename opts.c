#include "opts.h"


int debug = DEBUG_DEFAULT;
int lazy = LAZY_DEFAULT;
int typed = TYPED_DEFAULT;

int setopts(int argc, char **argv)
{
	int optind;
	for (optind = 1; optind < argc; optind++) {
		char *arg = argv[optind];
		if (arg[0] != '-') {
			break;
		}
		for (arg++; *arg; arg++) {
			switch (*arg) {
				case 'd': debug = 1; break;
				case 'l': lazy = 1;  break;
				case 't': typed = 1; break;
				default:
					return -1;
			}
		}
	}
	return optind;
}
