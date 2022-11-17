#include "opts.h"


int debug = 0;

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
				case 'd':
					debug = 1;
					break;
				default:
					return -1;
			}
		}
	}
	return optind;
}
