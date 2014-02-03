#include "public.h"
#include "task.h"
#include "spi.h"
#include "uart.h"
#include "debug.h"
#include "STM32_Init.h"
#include "MX25LXX.h"
#include <stdlib.h>

_TaskList HighTasks, LowTasks;

void SysTick_Handler(void)
{
	task_process(&HighTasks);
	task_dispatch(&HighTasks);
	task_dispatch(&LowTasks);
}

void led_task(void *dummy)
{
	GPIOA->ODR ^= BIT(2);
}

void uart_task(u32 *number)
{
	printlog("uart_task number:%d\n", ++(*number));
	
	if(*number > 10)
	{
		printlog("Kill uart_task...\n");
		task_del(&LowTasks, (void*)uart_task);
	}
}
/**************************************************************
Ö÷º¯Êý
**************************************************************/
int main(void)
{
	STM32_Init();
	uart1_dma_init();
	spi1_init();
	task_buf_init();
	task_list_init(&HighTasks);
	task_list_init(&LowTasks);
	
	task_add(&LowTasks, (void*)led_task, 0, TASK_SEC(0.5));
	task_add(&LowTasks, (void*)uart_task, task_param_alloc(sizeof(u32)), TASK_SEC(0.5));
	
	printlog("\nFrame for STM32 start...\n");
	printlog("MX25L3206E's ID 0x%06X\n", MX25LXX_ReadID());

	SysTick_Config(OSC / TASK_FREQ);
	
	while(1)
	{
		task_process(&LowTasks);
	}
}
