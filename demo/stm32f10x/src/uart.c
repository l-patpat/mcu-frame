#include "uart.h"

void uart_dma_init()
{
#ifdef USART_USE_DMA
	SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
	SET_BIT(DMA1->IFCR,
		DMA_IFCR_CTEIF4 | DMA_IFCR_CHTIF4 | DMA_IFCR_CTCIF4 | DMA_IFCR_CGIF4);
	
	WRITE_REG(DMA1_Channel4->CPAR, (u32)&(USART1->DR));
	CLEAR_REG(DMA1_Channel4->CCR);
	SET_BIT(DMA1_Channel4->CCR,
		DMA_CCR1_PL_0 | DMA_CCR1_MINC | DMA_CCR1_DIR);
#endif
}

void uart_buf(u8 *buf, u16 size)
{
#ifdef USART_USE_DMA
	uart_dma_wait();
	CLEAR_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN);
	WRITE_REG(DMA1_Channel4->CMAR, (u32)buf);
	WRITE_REG(DMA1_Channel4->CNDTR, size);
	SET_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN);
	SET_BIT(USART1->CR3, USART_CR3_DMAT);
#else
	while(size--) uart_byte(*buf++);
#endif
}

void uart_text(u8 *text)
{
#ifdef USART_USE_DMA
	u32 size = 0;
	
	while(*text)
	{
		text++;
		size++;
	}
	uart_buf(text - size, size);
#else
	while(*text) uart_byte(*text++);
#endif
}
