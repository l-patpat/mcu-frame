#include "public.h"
#include "task.h"
#include "uart.h"
#include "debug.h"
#include "STM32_Init.h"
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
	GPIOA->ODR ^= 1<<2;
}

void uart_task(u32 *number)
{
	printlog("number:%d\n", (*number)++);
	
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
	uart_dma_init();
	task_buf_init();
	task_list_init(&HighTasks);
	task_list_init(&LowTasks);
	
	task_add(&LowTasks, (void*)led_task, 0, TASK_SEC(0.5));
	task_add(&LowTasks, (void*)uart_task, task_param_alloc(sizeof(u32)), TASK_SEC(0.5));
	
	SysTick_Config(72000 * 1000 / TASK_FREQ);
	
	while(1)
	{
		task_process(&LowTasks);
	}
}
