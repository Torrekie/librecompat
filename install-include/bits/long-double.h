#ifndef __NO_LONG_DOUBLE_MATH
/* Darwin arm64 sizeof(long double) == sizeof(double) */
#define __NO_LONG_DOUBLE_MATH 1
#endif
#define __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI 0
