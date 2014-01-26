#include "public.h"
#include "task.h"
#include "uart.h"
#include "debug.h"
#include <stdlib.h>

_TaskList HighTasks, LowTasks;

void SysTick_Handler(void)
{
	task_process(&HighTasks);
	task_dispatch(&HighTasks);
	task_dispatch(&LowTasks);
}

void gpio_rcc_init(void)
{
	SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_IOPARST);
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
	CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_IOPARST);
}

void led_init(void)
{
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF2);
	SET_BIT(GPIOA->CRL, GPIO_CRL_MODE2);
}

void led_task(void *dummy)
{
	GPIOA->ODR ^= 1<<2;
}

void uart_task(u32 *number)
{
	task_del(&LowTasks, 0);
	printlog("number:%d\n", (*number)++);
}
/**************************************************************
Ö÷º¯Êý
**************************************************************/
int main(void)
{
	gpio_rcc_init();
	led_init();
	uart_init(115200);
	
	task_buf_init();
	task_list_init(&HighTasks);
	task_list_init(&LowTasks);
	task_add(&LowTasks, led_task, 0, TASK_SEC(0.5));
	task_add(&LowTasks, uart_task, task_param_alloc(sizeof(u32)), TASK_SEC(5));
	
	SysTick_Config(72000 * 1000 / TASK_FREQ);
	
	while(1)
	{
		task_process(&LowTasks);
	}
	return 0;
}
