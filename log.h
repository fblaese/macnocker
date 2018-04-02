#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG
#include <stdio.h>

#define log_debug(...) printf(__VA_ARGS__)
#else
#define log_debug(...) do { } while (0);
#endif

#endif // _DEBUG_H
