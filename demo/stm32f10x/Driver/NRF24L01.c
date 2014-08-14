#include "NRF24L01.h"
#include "spi.h"
#include "debug.h"

#define SPI_BYTE(BYTE) spi_byte(SPI1, BYTE)
#define SPI_BUF_SEND(BUF, SIZE) spi_buf_send(SPI1, BUF, SIZE)
#define SPI_BUF_SWAP(BUF, SIZE) spi_buf_swap(SPI1, BUF, SIZE)

const unsigned char TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
const unsigned char RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址

//初始化24L01的IO口
void NRF24L01_Init(void)
{
	NRF24L01_CEL(); 	//使能24L01
	NRF24L01_CSH();	//SPI片选取消		  		 		  
}
//检测24L01是否存在
unsigned char NRF24L01_Check(void)
{
#define DUMMY_BYTE 0xA5
	unsigned char buf[5]={DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE,DUMMY_BYTE};
	unsigned char i;
	
	NRF24L01_Write_Buf(CMD_WRITE_REG+TX_ADDR, buf, 5);
	NRF24L01_Read_Buf(TX_ADDR, buf, 5);
	printlog("NRF24L01 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
		buf[0], buf[1], buf[2], buf[3], buf[4]);
	
	for(i=0; i<5; i++)
	{
		if(buf[i] != DUMMY_BYTE)
			return 0;
	}
	return 1;
}	 	 
//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
unsigned char NRF24L01_Write_Reg(unsigned char reg,unsigned char value)
{
	unsigned char status;	
   	NRF24L01_CSL();                 //使能SPI传输
  	status = SPI_BYTE(reg);//发送寄存器号 
  	SPI_BYTE(value);      //写入寄存器的值
  	NRF24L01_CSH();                 //禁止SPI传输	   
  	return(status);       			//返回状态值
}
//读取SPI寄存器值
//reg:要读的寄存器
unsigned char NRF24L01_Read_Reg(unsigned char reg)
{
	unsigned char reg_val;	    
 	NRF24L01_CSL();          //使能SPI传输		
  	SPI_BYTE(reg);   //发送寄存器号
  	reg_val= SPI_BYTE(0XFF);//读取寄存器内容
  	NRF24L01_CSH();          //禁止SPI传输		    
  	return(reg_val);           //返回状态值
}	
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
unsigned char NRF24L01_Read_Buf(unsigned char reg,unsigned char *pBuf,unsigned char len)
{
	unsigned char status, ctr;	       
  	NRF24L01_CSL();           //使能SPI传输
  	status = SPI_BYTE(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(ctr=0; ctr < len; ctr++)
	{
		pBuf[ctr] = SPI_BYTE(0XFF);//读出数据
	}
  	NRF24L01_CSH();       //关闭SPI传输
  	return status;        //返回读到的状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
unsigned char NRF24L01_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char len)
{
	unsigned char status, ctr;	    
 	NRF24L01_CSL();          //使能SPI传输
  	status = SPI_BYTE(reg);//发送寄存器值(位置),并读取状态值
  	for(ctr=0; ctr<len; ctr++)
	{
		SPI_BYTE(*pBuf++); //写入数据	
	}
  	NRF24L01_CSH();       //关闭SPI传输
  	return status;          //返回读到的状态值
}				   
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
unsigned char NRF24L01_TxPacket(unsigned char *txbuf)
{
	unsigned char sta;
	NRF24L01_CEL();
  	NRF24L01_Write_Buf(CMD_WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF24L01_CEH();//启动发送	   
	while(0 != NRF24L01_IRQ());//等待发送完成
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值	   
	NRF24L01_Write_Reg(CMD_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)//达到最大重发次数
	{
		NRF24L01_Write_Reg(CMD_FLUSH_TX,0xff);//清除TX FIFO寄存器 
		return MAX_TX; 
	}
	if(sta&TX_OK)//发送完成
	{
		return TX_OK;
	}
	return 0xff;//其他原因发送失败
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
unsigned char NRF24L01_RxPacket(unsigned char *rxbuf)
{
	unsigned char sta;		    							   
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(CMD_WRITE_REG+STATUS, sta); //清除TX_DS或MAX_RT中断标志
	if(sta & RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(CMD_RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(CMD_FLUSH_RX, 0xFF);//清除RX FIFO寄存器 
		return 1;
	}	   
	return 0;
}					    
//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void RX_Mode(void)
{
	NRF24L01_CEL();	  
  	NRF24L01_Write_Buf(CMD_WRITE_REG+RX_ADDR_P0,(unsigned char*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
	  
  	NRF24L01_Write_Reg(CMD_WRITE_REG+EN_AA,0x01);    //使能通道0的自动应答    
  	NRF24L01_Write_Reg(CMD_WRITE_REG+EN_RXADDR,0x01);//使能通道0的接收地址  	 
  	NRF24L01_Write_Reg(CMD_WRITE_REG+RF_CH,40);	     //设置RF通信频率		  
  	NRF24L01_Write_Reg(CMD_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
  	NRF24L01_Write_Reg(CMD_WRITE_REG+RF_SETUP,0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(CMD_WRITE_REG+CONFIG, 0x0f);//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  	NRF24L01_CEH(); //CE为高,进入接收模式 
}						 
//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了		   
//CE为高大于10us,则启动发送.	 
void TX_Mode(void)
{														 
	NRF24L01_CEL();	    
  	NRF24L01_Write_Buf(CMD_WRITE_REG+TX_ADDR,(unsigned char*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址 
  	NRF24L01_Write_Buf(CMD_WRITE_REG+RX_ADDR_P0,(unsigned char*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  

  	NRF24L01_Write_Reg(CMD_WRITE_REG+EN_AA,0x01);     //使能通道0的自动应答    
  	NRF24L01_Write_Reg(CMD_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址  
  	NRF24L01_Write_Reg(CMD_WRITE_REG+SETUP_RETR,0x1a);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
  	NRF24L01_Write_Reg(CMD_WRITE_REG+RF_CH,40);       //设置RF通道为40
  	NRF24L01_Write_Reg(CMD_WRITE_REG+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(CMD_WRITE_REG+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	NRF24L01_CEH();//CE为高,10us后启动发送
}		  




