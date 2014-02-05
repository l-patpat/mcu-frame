#include "spi.h"

void spi1_init(void)
{
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);
	SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SPI1RST);
	CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SPI1RST);
	
	WRITE_REG(SPI1->CR1, SPI_CR1_SPE | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI);
}

u8 spi_byte(SPI_TypeDef *spi, u8 byte)
{
	if(READ_BIT(spi->CR1, SPI_CR1_SPE | SPI_CR1_MSTR) == (SPI_CR1_SPE | SPI_CR1_MSTR))
	{
		READ_REG(spi->DR);
		while(!READ_BIT(spi->SR, SPI_SR_TXE));
		WRITE_REG(spi->DR, byte);
		while(!READ_BIT(spi->SR, SPI_SR_RXNE));
	}
	return (u8)READ_REG(spi->DR);
}

void spi_buf_send(SPI_TypeDef *spi, u8 *buf, u16 size)
{
	if(spi == 0 || buf == 0 || size == 0)
	{
		return;
	}
	
	if(READ_BIT(spi->CR1, SPI_CR1_SPE | SPI_CR1_MSTR) == (SPI_CR1_SPE | SPI_CR1_MSTR))
	{
		while(size--)
		{
			while(!READ_BIT(spi->SR, SPI_SR_TXE));
			WRITE_REG(spi->DR, *buf);
			buf++;
		}
	}
}

void spi_buf_swap(SPI_TypeDef *spi, u8 *buf, u16 size)
{
	if(spi == 0 || buf == 0 || size == 0)
	{
		return;
	}
	
	if(READ_BIT(spi->CR1, SPI_CR1_SPE | SPI_CR1_MSTR) == (SPI_CR1_SPE | SPI_CR1_MSTR))
	{
		READ_REG(spi->DR);
		while(size--)
		{
			while(!READ_BIT(spi->SR, SPI_SR_TXE));
			WRITE_REG(spi->DR, *buf);
			while(!READ_BIT(spi->SR, SPI_SR_RXNE));
			*buf = (u8)READ_REG(spi->DR);
			buf++;
		}
	}
}
