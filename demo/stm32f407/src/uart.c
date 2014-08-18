#include "uart.h"

void uart1_init(void)
{
	GPIO_InitTypeDef gpioInit;
	USART_InitTypeDef uartInit;
	USART_ClockInitTypeDef uartClockInit;
	
#ifdef USART1_USE_DMA

#endif
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioInit.GPIO_Mode = GPIO_Mode_AF;
	
	gpioInit.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &gpioInit);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	uartInit.USART_BaudRate = 115200;
	uartInit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	uartInit.USART_WordLength = USART_WordLength_8b;
	uartInit.USART_Parity = USART_Parity_No;
	uartInit.USART_StopBits = USART_StopBits_1;
	uartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &uartInit);
	
	USART_ClockStructInit(&uartClockInit);
	USART_ClockInit(USART1, &uartClockInit);
	
	USART_Cmd(USART1, ENABLE);
}

void uart1_buf(uc8 *buf, u16 size)
{
#ifdef USART_USE_DMA
	uart1_dma_wait();
	CLEAR_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN);
	WRITE_REG(DMA1_Channel4->CMAR, (u32)buf);
	WRITE_REG(DMA1_Channel4->CNDTR, size);
	SET_BIT(DMA1_Channel4->CCR, DMA_CCR1_EN);
	SET_BIT(USART1->CR3, USART_CR3_DMAT);
#else
	while(size--) uart1_byte(*buf++);
#endif
}

void uart1_text(uc8 *text)
{
#ifdef USART1_USE_DMA
	u32 size = 0;
	
	while(*text)
	{
		text++;
		size++;
	}
	uart1_buf(text - size, size);
#else
	while(*text) uart1_byte(*text++);
#endif
}
