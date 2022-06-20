#ifndef _RODA_UTIL_MACRO_H_
#define _RODA_UTIL_MACRO_H_

#define STRINGIFY(X) # X
#define XSTRINGIFY(X) STRINGIFY(X)
#define XXSTRINGIFY(X) XSTRINGIFY(X)

#define CONCAT(X, Y) X ## Y

#endif
