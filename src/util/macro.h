#ifndef _MACRO_H_
#define _MACRO_H_

#define STRINGIFY(X) # X
#define XSTRINGIFY(X) STRINGIFY(X)
#define XXSTRINGIFY(X) XSTRINGIFY(X)

#define CONCAT(X, Y) X ## Y

#endif
