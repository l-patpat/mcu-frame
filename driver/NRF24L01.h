#ifndef __NRF24L01_H
#define __NRF24L01_H

void NRF24L01_Init(void);
void NRF24L01_RxMode(void);
void NRF24L01_TxMode(void);
void NRF24L01_RxPacket(unsigned char *buf);
void NRF24L01_TxPacket(unsigned char *buf);
void NRF24L01_TxPacketNoACK(unsigned char *buf, unsigned char len);
unsigned char NRF24L01_TxOK();
unsigned char NRF24L01_RxOK();
unsigned char NRF24L01_Check(void);

#endif /* __NRF24L01_H */
