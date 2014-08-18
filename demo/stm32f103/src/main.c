#include "public.h"
#include "task.h"
#include "spi.h"
#include "uart.h"
#include "debug.h"
#include "STM32_Init.h"
#include "SerialFlash.h"
#include "NRF24L01.h"
#include "simple_server.h"
#include <stdlib.h>

s_TaskList HighTasks, LowTasks;

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
	printlog("uart_task number:%d\n", (*number)++);
	
	if(*number > 10)
	{
		printlog("Kill uart_task...\n");
		task_del(&HighTasks, (void*)uart_task);
	}
}

void flash_task(u8 *step)
{
	unsigned short i;
	unsigned char buf[256];
	
	switch(*step)
	{
		case 0:
			if(!SerialFlash_IsBusy())
			{
				printlog("flash_task step0: Sector erase at address 0x000000\n");
				SerialFlash_WriteEnable(1);
				SerialFlash_SectorErase(0x000000);
				*step = 1;
			}
			else
			{
				printlog("flash_task step0: Is busy, wait\n");
			}
			break;
		case 1:
			if(!SerialFlash_IsBusy())
			{
				printlog("flash_task step1: Page program at address 0x000000, size 256\n");
				for(i = 0; i < sizeof(buf); i++)
				{
					buf[i] = i + 1;
				}
				SerialFlash_WriteEnable(1);
				SerialFlash_PageProgram(0x000000, buf, sizeof(buf));
				*step = 2;
			}
			else
			{
				printlog("flash_task step1: Is busy, wait\n");
			}
			break;
		case 2:
			if(!SerialFlash_IsBusy())
			{
				printlog("flash_task step2: Fast read at address 0x000000, size 256\n");
				for(i = 0; i < sizeof(buf); i++)
				{
					buf[i] = 0;
				}
				SerialFlash_Read(0x000000, buf, sizeof(buf), 1);
				
				for(i = 0; i < 8; i++)
				{
					printlog("0x%02X ", buf[i]);
				}
				printlog("\nKill flash_task...\n");
				task_del(&HighTasks, flash_task);
			}
			else
			{
				printlog("flash_task step2: Is busy, wait\n");
			}
			break;
	}
}

void rf_task(u8 *step)
{
	unsigned char buf[33];
	
	switch(*step)
	{
		case 0:
			RX_Mode();
			printlog("NRF24L01 in RX mode\n");
			*step = 1;
			break;
		case 1:
			if(NRF24L01_RxPacket(buf))
			{
				buf[32]=0;
				printlog("NRF24L01 RX:%s", buf);
			}
			break;
	}
}
/**************************************************************
Ö÷º¯Êý
**************************************************************/
int main(void)
{
	void *param;
	
	STM32_Init();
	uart1_dma_init();
	spi1_init(SPI_CR1_BR_DIV_8);
	spi2_init(SPI_CR1_BR_DIV_2);
	NRF24L01_Init();
	
	printlog("\nFrame for STM32 start...\n");
	
	task_buf_init();
	task_list_init(&HighTasks);
	task_list_init(&LowTasks);
	
	task_add(&HighTasks, (void*)led_task, 0, TASK_SEC(0.5));
	task_add(&HighTasks, (void*)uart_task, task_param_alloc(sizeof(u32)), TASK_SEC(0.5));

	if(NRF24L01_Check())
	{
		printlog("Found NRF24L01\n");
		param = task_param_alloc(sizeof(u8));
		*(u8*)param = 0;
		task_add(&HighTasks, (void*)rf_task, param, 2);
	}
	else
	{
		printlog("Not found NRF24L01\n");
	}

	SerialFlash_Select(FLASH_DEVICE_1);
	if((SerialFlash_ReadID() & 0xFF0000) == 0xC20000) //MXIC flash manufacturer ID is 0xC2
	{
		printlog("Found MXIC flash, ID 0x%06X\n", SerialFlash_ReadID());
		param = task_param_alloc(sizeof(u8));
		*(u8*)param = 0;
		task_add(&HighTasks, (void*)flash_task, param, 2);
	}
	else
	{
		printlog("Not found MXIC flash\n");
	}

	SysTick_Config(OSC / TASK_FREQ);
	
	simple_server();
	
	while(1)
	{
		task_process(&LowTasks);
	}
}
