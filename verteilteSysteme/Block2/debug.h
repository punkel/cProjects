#ifndef _DEBUG_H
#define _DEBUG_H

#define TRUE 1
#define FALSE 0

#ifdef DEBUG
#define DP(...) fprintf(stdout, __VA_ARGS__ )
#else
#define DP(...) fprintf(stderr, __VA_ARGS__ )
#endif

#endif /* _DEBUG_H */
