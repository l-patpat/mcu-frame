#include "public.h"
#include "task.h"
#include "spi.h"
#include "uart.h"
#include "debug.h"
#include "NRF24L01.h"

s_TaskList HighTasks, LowTasks;

void led_init(void)
{
	GPIO_InitTypeDef gpioInit;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	gpioInit.GPIO_Mode = GPIO_Mode_OUT;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
	gpioInit.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOD, &gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOD, &gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOD, &gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOD, &gpioInit);
}

void led_task(u32 *step)
{
	switch(*step)
	{
		case 0:
			GPIOD->ODR ^= BIT(12);
			(*step)++;
			break;
		case 1:
			GPIOD->ODR ^= BIT(13);
			(*step)++;
			break;
		case 2:
			GPIOD->ODR ^= BIT(14);
			(*step)++;
			break;
		case 3:
			GPIOD->ODR ^= BIT(15);
		default:
			*step = 0;
	}
}

void rf_task(u8 *step)
{
	unsigned char buf[33];
	int i = sizeof(buf);
	
	switch(*step)
	{
		case 0:
			TX_Mode();
			printlog("NRF24L01 in TX mode");
			*step = 1;
			break;
		case 1:
			while(i--) buf[i] = 'A';
			buf[sizeof(buf) - 1] = 0;
			NRF24L01_TxPacket(buf);
		printlog("NRF24L01 TX:%s", buf);
			break;
	}		
}

void SysTick_Handler(void)
{
	task_process(&HighTasks);
	task_dispatch(&HighTasks);
	task_dispatch(&LowTasks);
}

int main(void)
{
	void *param;
	RCC_ClocksTypeDef rcc;
	
	led_init();
	uart1_init();
	NRF24L01_Init();
	
	task_buf_init();
	task_list_init(&HighTasks);
	task_list_init(&LowTasks);
	
	param = task_param_alloc(sizeof(u32));
	*(u8*)param = 0;
	task_add(&LowTasks, (void*)led_task, param, TASK_SEC(0.1));

	if(NRF24L01_Check())
	{
		printlog("Found NRF24L01\n");
		param = task_param_alloc(sizeof(u8));
		*(u8*)param = 0;
		task_add(&HighTasks, (void*)rf_task, param, TASK_SEC(1));
	}
	else
	{
		printlog("Not found NRF24L01\n");
	}

	printlog("\nMCU frame for STM32F407 start...\n");
	
	RCC_GetClocksFreq(&rcc);
	SysTick_Config(rcc.SYSCLK_Frequency / TASK_FREQ);
	
	while(1)
	{
		task_process(&LowTasks);
	}
	return 0;
}
