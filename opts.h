#ifndef OPTS_INCLUDED
#define OPTS_INCLUDED

extern int debug;
extern int lazy;
extern int typed;

#define DEBUG_DEFAULT 0
#define LAZY_DEFAULT 0
#define TYPED_DEFAULT 0

int setopts(int argc, char **argv);

#endif // OPTS_INCLUDED
