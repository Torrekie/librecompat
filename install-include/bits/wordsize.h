#include <stdint.h>

#if __LP64__
#ifndef __WORDSIZE
#define __WORDSIZE 64
#endif
#else
#ifndef __WORDSIZE
#define __WORDSIZE 32
#endif
#endif
