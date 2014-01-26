#ifndef __DEBUG_H
#define __DEBUG_H
#include "public.h"

#ifdef _DEBUG
extern void printlog(const char *format, ...);
#else
#define printlog(...)
#endif

#endif /* __DEBUG_H */
