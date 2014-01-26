#include "uart.h"

#define USART_USE_DMA

#define UART_BYTE(BYTE) \
do { \
	while(READ_BIT(USART1->CR1, USART_CR1_UE|USART_CR1_TE) == (USART_CR1_UE|USART_CR1_TE) && \
		!READ_BIT(USART1->SR, USART_SR_TXE)); \
	WRITE_REG(USART1->DR, (u8)BYTE); \
} while(0)

void uart_init(u32 band)
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
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
	SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST);
	CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_USART1RST);

	CLEAR_BIT(GPIOA->CRH, GPIO_CRH_CNF9);
	SET_BIT(GPIOA->CRH, GPIO_CRH_CNF9_1);
	SET_BIT(GPIOA->CRH, GPIO_CRH_MODE9);
	
	WRITE_REG(USART1->BRR, OSC / band);
	SET_BIT(USART1->CR1, USART_CR1_UE|USART_CR1_TE);
}

void uart_byte(u8 byte)
{
	UART_BYTE(byte);
}

void uart_buf(u8 *buf, u16 size)
{
#ifdef USART_USE_DMA
	while(READ_REG(DMA1_Channel4->CNDTR) && READ_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN));
	CLEAR_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN);
	WRITE_REG(DMA1_Channel4->CMAR, (u32)buf);
	WRITE_REG(DMA1_Channel4->CNDTR, size);
	SET_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN);
	SET_BIT(USART1->CR3, USART_CR3_DMAT);
#else
	while(size--) UART_BYTE(*buf++);
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
	while(*text) UART_BYTE(*text++);
#endif
}
