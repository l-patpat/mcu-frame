#include "SerialFlash.h"
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
	CMD_BE1		= 0x52,
	CMD_BE2		= 0xD8,
	CMD_CE		= 0x60
};

enum ENUM_STATUS_REGISTER {
	SR_WIP		= 0x01,
	SR_WEL		= 0x02,
	SR_BP0		= 0x04,
	SR_BP1		= 0x08,
	SR_BP2		= 0x10,
	SR_BP3		= 0x20,
	SR_SRWD		= 0x80
};

long SerialFlash_ReadID(void)
{
	unsigned char buf[4];
	
	buf[0] = CMD_RDID;
	CS_CLR();
	SPI_BUF_SWAP(buf, sizeof(buf));
	CS_SET();
	
	return (long)buf[1]<<16 | (long)buf[2]<<8 | (long)buf[3];
}

unsigned char SerialFlash_ReadStatus(void)
{
	unsigned char status;
	
	CS_CLR();
	SPI_BYTE(CMD_RDSR);
	status = SPI_BYTE(0);
	CS_SET();
	
	return status;
}

char SerialFlash_IsBusy(void)
{
	char status = SerialFlash_ReadStatus();
	return status & SR_WIP;
}

char SerialFlash_IsWriteEnable(void)
{
	return SerialFlash_ReadStatus() & SR_WEL;
}

void SerialFlash_Read(long address, unsigned char *buf, unsigned short size, char fast)
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

void SerialFlash_StreamReadStart(long address, char fast)
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

unsigned char SerialFlash_StreamRead(void)
{
	return SPI_BYTE(0);
}

void SerialFlash_StreamReadStop(void)
{
	CS_SET();
}

void SerialFlash_WriteEnable(char enable)
{
	CS_CLR();
	SPI_BYTE(enable ? CMD_WREN : CMD_WRDI);
	CS_SET();
}

void SerialFlash_PageProgram(long address, unsigned char *buf, unsigned short size)
{
	CS_CLR();
	SPI_BYTE(CMD_PP);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	SPI_BUF_SEND(buf, size);
	CS_SET();
}

void SerialFlash_SectorErase(long address)
{
	CS_CLR();
	SPI_BYTE(CMD_SE);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	CS_SET();
}

void SerialFlash_BlockErase1(long address)
{
	CS_CLR();
	SPI_BYTE(CMD_BE1);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	CS_SET();
}

void SerialFlash_BlockErase2(long address)
{
	CS_CLR();
	SPI_BYTE(CMD_BE2);
	SPI_BYTE((unsigned char)(address>>16));
	SPI_BYTE((unsigned char)(address>>8));
	SPI_BYTE((unsigned char)(address & 0xFF));
	CS_SET();
}

void SerialFlash_ChipErase(void)
{
	CS_CLR();
	SPI_BYTE(CMD_CE);
	CS_SET();
}
