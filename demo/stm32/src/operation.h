#ifndef __OPERATION_H
#define __OPERATION_H

typedef struct _BUTTON {
	unsigned char KeepTime;
	unsigned char SlowDown;
	enum _Buttons {
		BN_NONE = 0,
		BN_BTN1 = 1<<0,
		BN_BTN2 = 1<<1,
		BN_BTN3 = 1<<2,
		BN_BTN4 = 1<<3,
		BN_BTN5 = 1<<4,
		BN_BTN6 = 1<<5,
		BN_BTN7 = 1<<6,
		BN_BTN8 = 1<<7
	} Cur, Pre, Tmp;
} _Button;

extern _Button Button;

extern void (*operation)(void);
extern void operation_init(void);
extern void demo_operation(void);

#endif /* __OPERATION_H */
