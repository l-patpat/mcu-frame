#include "MX25LXX.h"
#include "public.h"
#include "spi.h"
#include "debug.h"

#define CS_SET()	do{ WRITE_REG(GPIOB->BSRR, BIT(0)); }while(0)
#define CS_CLR()	do{ WRITE_REG(GPIOB->BRR, BIT(0)); }while(0)

#define SPI_BYTE(BYTE) spi_byte(SPI1, BYTE)
#define SPI_BUF_SEND(BUF, SIZE) spi_buf_send(SPI1, BUF, SIZE)
#define SPI_BUF_SWAP(BUF, SIZE) spi_buf_swap(SPI1, BUF, SIZE)

enum ENUM_COMMAND {
	CMD_RDID	= 0x9F,
	CMD_RDSR	= 0x05,
	CMD_READ	= 0x03,
	CMD_FREAD	= 0x0B,
	CMD_WRDI	= 0x04,
	CMD_WREN	= 0x06,
	CMD_PP		= 0x02,
	CMD_SE		= 0x20,
	CMD_BE		= 0x52,
	CMD_CE		= 0x60
};

long MX25LXX_ReadID(void)
{
	unsigned char buf[4];
	
	buf[0] = CMD_RDID;
	CS_CLR();
	SPI_BUF_SWAP(buf, sizeof(buf));
	CS_SET();
	
	return (long)buf[1]<<16 | (long)buf[2]<<8 | (long)buf[3];
}

unsigned char MX25LXX_ReadStatus(void)
{
	unsigned char status;
	
	CS_CLR();
	status = SPI_BYTE(CMD_RDSR);
	CS_SET();
	
	return status;
}

void MX25LXX_Read(long address, unsigned char *buf, unsigned short size, char fast)
{
	CS_CLR();
	SPI_BYTE(fast ? CMD_FREAD : CMD_READ);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	if(fast)
	{
		SPI_BYTE(0);
	}
	SPI_BUF_SWAP(buf, size);
	CS_SET();
}

void MX25LXX_StreamReadStart(long address, char fast)
{
	CS_CLR();
	SPI_BYTE(fast ? CMD_FREAD : CMD_READ);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	if(fast)
	{
		SPI_BYTE(0);
	}
}

unsigned char MX25LXX_StreamRead(void)
{
	return SPI_BYTE(0);
}

void MX25LXX_StreamReadStop(void)
{
	CS_SET();
}

void MX25LXX_WriteEnable(char enable)
{
	CS_CLR();
	SPI_BYTE(enable ? CMD_WREN : CMD_WRDI);
	CS_SET();
}

void MX25LXX_PageProgram(long address, unsigned char *buf, unsigned short size)
{
	CS_CLR();
	SPI_BYTE(CMD_PP);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	SPI_BUF_SEND(buf, size);
	CS_SET();
}

void MX25LXX_SectorErase(long address)
{
	CS_CLR();
	SPI_BYTE(CMD_SE);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	CS_SET();
}

void MX25LXX_BlockErase(long address)
{
	CS_CLR();
	SPI_BYTE(CMD_BE);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	CS_SET();
}

void MX25LXX_ChipErase(void)
{
	CS_CLR();
	SPI_BYTE(CMD_CE);
	CS_SET();
}
