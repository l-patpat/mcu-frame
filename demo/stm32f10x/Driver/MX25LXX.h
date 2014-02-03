#ifndef __MX25LXX_H
#define __MX25LXX_H

extern long MX25LXX_ReadID(void);
extern unsigned char MX25LXX_ReadStatus(void);
extern void MX25LXX_Read(long address, unsigned char *buf, unsigned short size, char fast);
extern void MX25LXX_StreamReadStart(long address, char fast);
extern unsigned char MX25LXX_StreamRead(void);
extern void MX25LXX_StreamReadStop(void);
extern void MX25LXX_FastRead(long address, unsigned char *buf, unsigned short size);
extern void MX25LXX_WriteEnable(char enable);
extern void MX25LXX_PageProgram(long address, unsigned char *buf, unsigned short size);
extern void MX25LXX_SectorErase(long address);
extern void MX25LXX_BlockErase(long address);
extern void MX25LXX_ChipErase(void);

#endif /* __MX25L3206E_H */
