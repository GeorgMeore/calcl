#ifndef ANNOTATIONS_INCLUDED
#define ANNOTATIONS_INCLUDED

// These are some fake "type qualifiers" that can be used to annotate function pointer arguments.
// They obviously are not real qualifiers, they are used only as a mean of code documentation.

// The callee is responsible for the deallocation.
#define passed

// The caller is responsible for the deallocation.
// This is actually the "default" way and is not actually used anywhere.
#define shared

#endif // ANNOTATIONS_INCLUDED
