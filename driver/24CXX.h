#ifndef __EEPROM_H
#define __EEPROM_H

/**************************************************************
EEPROM½øÈëµÍ¹¦ºÄ
**************************************************************/
extern void eeprom_sleep();
/**************************************************************
¶ÁEEPROM
**************************************************************/
extern char eeprom_read(unsigned short address, unsigned char *buf, unsigned short size);
/**************************************************************
Ð´EEPROM
**************************************************************/
extern char eeprom_write(unsigned short address, unsigned char *buf, unsigned short size);

#endif /* __EEPROM_H */
