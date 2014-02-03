#ifndef __DEBUG_H
#define __DEBUG_H
#include "public.h"

#ifdef _DEBUG
extern void printlog(const char *format, ...);
#else
#define printlog(...) do while(0)
#endif

#endif /* __DEBUG_H */
