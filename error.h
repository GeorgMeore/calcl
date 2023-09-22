#ifndef ERROR_INCLUDED
#define ERROR_INCLUDED

#include <stdio.h>

// ERROR_PREFIX is supposed to be defined by the includer

#define error(message) \
	(fprintf(stderr, ERROR_PREFIX ": " message "\n"))

#define errorf(fmt, args...) \
	(fprintf(stderr, ERROR_PREFIX ": " fmt "\n", args))

#endif // ERROR_INCLUDED
