#ifndef __SPI_H
#define __SPI_H

#include "public.h"

extern void spi1_init(void);
extern u8 spi_byte(SPI_TypeDef *spi, u8 byte);
extern void spi_buf_send(SPI_TypeDef *spi, u8 *buf, u16 size);
extern void spi_buf_swap(SPI_TypeDef *spi, u8 *buf, u16 size);

#endif /* __SPI_H */
