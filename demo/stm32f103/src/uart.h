#ifndef __UART_H
#define __UART_H

#include "public.h"

#define USART1_USE_DMA

#define uart1_byte(BYTE) \
do { \
	if(READ_BIT(USART1->CR1, USART_CR1_UE | USART_CR1_TE) == (USART_CR1_UE | USART_CR1_TE)) \
	{ \
		while(!READ_BIT(USART1->SR, USART_SR_TXE)); \
		WRITE_REG(USART1->DR, (u8)BYTE); \
	} \
} while(0)

#ifdef USART1_USE_DMA
#define uart1_dma_wait() \
do { \
	while(READ_REG(DMA1_Channel4->CNDTR) && \
		READ_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN) && \
		!READ_BIT(DMA1_Channel4->CCR, DMA_CCR1_CIRC)); \
} while(0)
#else
#define uart1_dma_wait() do{} while(0)
#endif

extern void uart1_dma_init(void);
extern void uart1_text(uc8 *text);
extern void uart1_buf(uc8 *buf, u16 size);

#endif /* __UART_H */
