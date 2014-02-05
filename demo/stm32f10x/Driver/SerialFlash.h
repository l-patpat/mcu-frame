#ifndef __SerialFlash_H
#define __SerialFlash_H

extern long SerialFlash_ReadID(void);
extern unsigned char SerialFlash_ReadStatus(void);
extern char SerialFlash_IsBusy(void);
extern char SerialFlash_IsWriteEnable(void);
extern void SerialFlash_Read(long address, unsigned char *buf, unsigned short size, char fast);
extern void SerialFlash_StreamReadStart(long address, char fast);
extern unsigned char SerialFlash_StreamRead(void);
extern void SerialFlash_StreamReadStop(void);
extern void SerialFlash_WriteEnable(char enable);
extern void SerialFlash_PageProgram(long address, unsigned char *buf, unsigned short size);
extern void SerialFlash_SectorErase(long address);
extern void SerialFlash_BlockErase1(long address);
extern void SerialFlash_BlockErase2(long address);
extern void SerialFlash_ChipErase(void);

#endif /* __MX25L3206E_H */
