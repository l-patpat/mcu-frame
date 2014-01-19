#include "operation.h"

void (*operation)(void) = 0;

_Button Button = {0, 0, BN_NONE, BN_NONE, BN_NONE};

void operation_scan(void)
{
	Button.Pre = Button.Cur;
	//Button.Cur = ~P0;
	if(Button.Cur != BN_NONE && Button.Cur == Button.Pre)
	{
		Button.SlowDown++;
		if(Button.KeepTime < 255)
			Button.KeepTime++;
	}
	else
	{
		Button.KeepTime = 0;
		Button.SlowDown = 0;
	}
}

void operation_init(void)
{
	operation = demo_operation;
}

void demo_operation(void)
{
	operation_scan();
	if(Button.Cur == (BN_BTN7 | BN_BTN8)) //优先级最高的全局组合键
	{
		if(Button.KeepTime == 2*8)
		{
			//BN_BTN7+BN_BTN8长按2秒后执行
		}
		return;
	}
	Button.Tmp = Button.Cur;
	//if(根据运行状态禁用某些按键)
	//{
		Button.Tmp &= ~(BN_BTN5 | BN_BTN6 | BN_BTN7 | BN_BTN8); //禁用了BN_BTN5，BN_BTN6，BN_BTN7，BN_BTN8键
	//}
	switch(Button.Tmp)
	{
		//无按键
		case BN_NONE:
			if(Button.Pre == BN_BTN1)
			{
				//BN_BTN1松开
			}
			break;
		//BN_BTN1
		case BN_BTN1:
			if(Button.Pre == BN_NONE)
			{
				//BN_BTN1按下
			}
			else if(Button.Pre == BN_BTN1 && Button.KeepTime == 1*8)
			{
				//BN_BTN1长按了1秒
			}
			break;
		//BN_BTN2
		case BN_BTN2:
			if(Button.Pre == BN_NONE)
			{
				//BN_BTN2按下
			}
			else if(Button.Pre == BN_BTN1 && Button.SlowDown >= 0.5*8)
			{
				Button.SlowDown = 0;
				//BN_BTN2长按时每0.5秒执行一次
			}
			break;
	}
}

