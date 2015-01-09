#ifndef __NRF24L01_HAL_H
#define __NRF24L01_HAL_H

#include "spi.h"
#include "debug.h"

#ifndef INLINE
#define INLINE						__inline	//如果编译器不支持内联函数可留空
#endif

#define CEH()						SET_IO(GPIOA, BIT(4))
#define CEL()						CLEAR_IO(GPIOA, BIT(4))
#define CSH()						SET_IO(GPIOC, BIT(4))
#define CSL()						CLEAR_IO(GPIOC, BIT(4))
#define IRQ()						READ_IO(GPIOC, BIT(5))

#define SPI_BYTE(BYTE)				spi_byte(SPI1, BYTE)
#define SPI_BUF_SEND(BUF, SIZE)		spi_buf_send(SPI1, BUF, SIZE)
#define SPI_BUF_SWAP(BUF, SIZE)		spi_buf_swap(SPI1, BUF, SIZE)

#define RX_PAYLOAD_WIDTH			32
#define TX_PAYLOAD_WIDTH			32

const unsigned char RxModeConfig[] = {
	6, REG_RX_ADDR_P0, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
	2, REG_EN_AA, 0x01,
	2, REG_EN_RXADDR, 0x01,
	2, REG_RF_CH, 40,
	2, REG_RX_PW_P0, RX_PAYLOAD_WIDTH,
	2, REG_RF_SETUP, 0x0F,
	2, REG_CONFIG, 0x0F,
	0
};

const unsigned char TxModeConfig[] = {
	6, REG_TX_ADDR, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
	6, REG_RX_ADDR_P0, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
	2, REG_EN_AA, 0x01,
	2, REG_EN_RXADDR, 0x01,
	2, REG_SETUP_RETR, 0x1A,
	2, REG_RF_CH, 40,
	2, REG_RF_SETUP, 0x0F,
	2, REG_CONFIG, 0x0E,
	0
};

static INLINE void NRF24L01_HAL_Init(void)
{
	GPIO_InitTypeDef gpioInit;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
	gpioInit.GPIO_Pin = GPIO_Pin_4;
	gpioInit.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &gpioInit);
	
	gpioInit.GPIO_Pin = GPIO_Pin_4;
	gpioInit.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOC, &gpioInit);
	
	gpioInit.GPIO_Pin = GPIO_Pin_5;
	gpioInit.GPIO_Mode = GPIO_Mode_IN;
	gpioInit.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &gpioInit);

	spi1_init(SPI_BaudRatePrescaler_8);
}

#endif /* __NRF24L01_HAL_H */
