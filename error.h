#ifndef ERROR_INCLUDED
#define ERROR_INCLUDED

#include <stdio.h>

#define error(message) \
	(fprintf(stderr, message "\n"))

#define errorf(fmt, args...) \
	(fprintf(stderr, fmt "\n", args))

#endif // ERROR_INCLUDED
