#ifndef OPTS_INCLUDED
#define OPTS_INCLUDED

extern int debug;
extern int lazy;

#define DEBUG_DEFAULT 0
#define LAZY_DEFAULT 0

int setopts(int argc, char **argv);

#endif // OPTS_INCLUDED
