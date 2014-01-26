#include "debug.h"
#include "uart.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef _DEBUG

static char LogBuf[64];

void printlog(const char *format, ...)
{
	va_list arg_prt;
	va_start(arg_prt, format);
	vsprintf(LogBuf, format, arg_prt);
	va_end(arg_prt);
	uart_text((u8*)LogBuf);
}
#endif
