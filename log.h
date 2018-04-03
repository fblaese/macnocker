#ifndef _DEBUG_H
#define _DEBUG_H

#if defined(DEBUG) || defined(TRACE)
#include <stdio.h>
#endif

#ifdef DEBUG
#define log_debug(...) printf(__VA_ARGS__)
#else
#define log_debug(...) do { } while (0);
#endif

#ifdef TRACE
#define log_trace(...) printf(__VA_ARGS__)
#else
#define log_trace(...) do { } while (0);
#endif

#endif // _DEBUG_H
