#include "NRF24L01.h"

enum E_COMMAND {
	CMD_R_REGISTER			= 0x00,	//读寄存器
	CMD_W_REGISTER			= 0x20,	//写寄存器
	CMD_R_RX_PAYLOAD		= 0x61,	//从FIFO读接收到的数据
	CMD_W_TX_PAYLOAD		= 0xA0,	//写负载数据
	CMD_FLUSH_TX			= 0xE1,	//清空TX FIFO
	CMD_FLUSH_RX			= 0xE2,	//清空RX FIFO
	CMD_REUSE_TX_PL			= 0xE3,	//重新发送TX FIFO中的数据
	CMD_R_RX_PL_WID			= 0x60,	//读取收到的数据字节数
	CMD_W_ACK_PAYLOAD		= 0xA8,	//写ACK负载数据
	CMD_W_TX_PAYLOAD_NOACK	= 0xB0,	//写负载数据（无应答）
	CMD_NOP					= 0xFF	//空操作
};

enum E_REGISTER {
	REG_CONFIG				= 0x00,
	REG_EN_AA				= 0x01,
	REG_EN_RXADDR			= 0x02,
	REG_SETUP_AW			= 0x03,
	REG_SETUP_RETR			= 0x04,
	REG_RF_CH				= 0x05,
	REG_RF_SETUP			= 0x06,
	REG_STATUS				= 0x07,
	REG_OBSERVE_TX			= 0x08,
	REG_RSSI				= 0x09,
	REG_RX_ADDR_P0			= 0x0A,
	REG_RX_ADDR_P1			= 0x0B,
	REG_RX_ADDR_P2			= 0x0C,
	REG_RX_ADDR_P3			= 0x0D,
	REG_RX_ADDR_P4			= 0x0E,
	REG_RX_ADDR_P5			= 0x0F,
	REG_TX_ADDR				= 0x10,
	REG_RX_PW_P0			= 0x11,
	REG_RX_PW_P1			= 0x12,
	REG_RX_PW_P2			= 0x13,
	REG_RX_PW_P3			= 0x14,
	REG_RX_PW_P4			= 0x15,
	REG_RX_PW_P5			= 0x16,
	REG_FIFO_STATUS			= 0x17,
	REG_DYNPD				= 0x1C,
	REG_FEATURE				= 0x1D
};

enum E_CONFIG {
	CONFIG_MASK_RX_DR		= 0x40,
	CONFIG_MASK_TX_DS		= 0x20,
	CONFIG_MASK_MAX_RT		= 0x10,
	CONFIG_EN_CRC			= 0x08,
	CONFIG_CRCO				= 0x04,
	CONFIG_PWR_UP			= 0x02,
	CONFIG_PRIM_RX			= 0x01
};

enum E_STATUS {
	STATUS_RX_DR			= 0x40,
	STATUS_TX_DS			= 0x20,
	STATUS_MAX_RT			= 0x10,
	STATUS_TX_FULL			= 0x01
};

#include "NRF24L01_HAL.h"

static INLINE void SendCMD(unsigned char cmd)
{
   	CSL();
  	SPI_BYTE(cmd);
  	CSH();
}

static INLINE unsigned char Read(unsigned char reg)
{
	unsigned char value;
 	CSL();
  	SPI_BYTE(reg);
  	value = SPI_BYTE(0xFF);
  	CSH();
  	return value;
}

static INLINE void Write(unsigned char reg, unsigned char value)
{
   	CSL();
  	SPI_BYTE(reg);
  	SPI_BYTE(value);
  	CSH();
}

static void ReadBuf(unsigned char reg, unsigned char *buf, unsigned char len)
{
	if(!buf || !len)
		return;
	
  	CSL();
  	SPI_BYTE(reg);
 	while(len--)
	{
		*buf++ = SPI_BYTE(0xFF);
	}
  	CSH();
}

static void WriteBuf(unsigned char reg, unsigned char *buf, unsigned char len)
{
	if(!buf || !len)
		return;
	
 	CSL();
  	SPI_BYTE(reg);
 	while(len--)
	{
		SPI_BYTE(*buf++);
	}
  	CSH();
}

void NRF24L01_Init(void)
{
	NRF24L01_HAL_Init();
	CEL();
	CSH();
}

unsigned char NRF24L01_Check(void)
{
	unsigned char buf[5];
	unsigned char i;
	
	for(i = 0; i < 5; i++)
	{
		buf[i] = 0xA5;
	}
	
	WriteBuf(CMD_W_REGISTER | REG_TX_ADDR, buf, 5);
	ReadBuf(REG_TX_ADDR, buf, 5);
	
	for(i = 0; i < 5; i++)
	{
		if(buf[i] != 0xA5)
			return 0;
	}
	return 1;
}

void NRF24L01_SetTxAddress(unsigned char *address, unsigned char length)
{
	if(length < 3 || length > 5)
		return;
	
  	WriteBuf(CMD_W_REGISTER | REG_TX_ADDR, address, length);
}

void NRF24L01_SetRxAddress(unsigned char channel, unsigned char *address, unsigned char length)
{
	if(channel > 5)
		return;
	if(length < 3 || length > 5)
		return;
	
  	WriteBuf(CMD_W_REGISTER | (REG_RX_ADDR_P0 + channel), address, length);
}

void NRF24L01_Config(const unsigned char *buf)
{
	unsigned char len = *buf++;

	if(!buf)
		return;
	
	CSL();
	while(len--)
	{
		SPI_BYTE(*buf++);
		if(len == 0)
		{
			CSH();
			len = *buf++;
			CSL();
		}
	}
	CSH();
}

void NRF24L01_ShutDown()
{
	unsigned char config;
	
	CEL();
	config = Read(CMD_R_REGISTER | REG_CONFIG);
	if(config & CONFIG_PWR_UP)
	{
		Write(CMD_W_REGISTER | REG_CONFIG, config & ~CONFIG_PWR_UP);
	}
}

void NRF24L01_RxMode(void)
{
	CEL();
  	NRF24L01_Config(RxModeConfig);
  	CEH();
}

void NRF24L01_TxMode(void)
{
	CEL();
  	NRF24L01_Config(TxModeConfig);
}

void NRF24L01_RxPacket(unsigned char *buf)
{
	unsigned char status;
	
	status = Read(CMD_R_REGISTER | REG_STATUS);
	if(status & STATUS_RX_DR)
	{
		Write(CMD_W_REGISTER | REG_STATUS, status & ~(STATUS_TX_DS | STATUS_MAX_RT));
		ReadBuf(CMD_R_RX_PAYLOAD, buf, );
	}
}

void NRF24L01_TxPacket(unsigned char *buf)
{
	CEL();
  	WriteBuf(CMD_W_TX_PAYLOAD, buf, TX_PAYLOAD_WIDTH);
 	CEH();
}

void NRF24L01_TxPacketNoACK(unsigned char *buf, unsigned char len)
{
	CEL();
  	WriteBuf(CMD_W_TX_PAYLOAD_NOACK, buf, len);
 	CEH();
}

unsigned char NRF24L01_TxOK()
{
	unsigned char status;
	
	status = Read(CMD_R_REGISTER | REG_STATUS);
	
	if(!(status & STATUS_TX_DS))
		return 0;
	if(status & STATUS_MAX_RT)
		return 0;
	
	return 1;
}

unsigned char NRF24L01_RxOK()
{
	unsigned char status;
	
	status = Read(CMD_R_REGISTER | REG_STATUS);
	
	if(!(status & STATUS_TX_DS))
		return 0;
	if(status & STATUS_MAX_RT)
		return 0;
	
	return 1;
}
