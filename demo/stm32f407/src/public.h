#ifndef __PUBLIC_H
#define __PUBLIC_H

#include "stm32f4xx.h"

#define OSC		168000000

#define BIT(N)	(1<<(N))

#define SET_IO(GPIO, BIT)		do { WRITE_REG(GPIO->BSRRL, BIT); } while(0)

#define CLEAR_IO(GPIO, BIT)		do { WRITE_REG(GPIO->BSRRH, BIT); } while(0)

#define READ_IO(GPIO, BIT)		READ_BIT(GPIO->IDR, BIT)

#endif /* __PUBLIC_H */
