#ifndef __UART_H
#define __UART_H

#include "public.h"

extern void uart_byte(u8 byte);
extern void uart_init(u32 band);
extern void uart_text(u8 *text);
extern void uart_buf(u8 *buf, u16 size);

#endif /* __UART_H */
