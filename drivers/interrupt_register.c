//*************************************************************************************
//File Name: interrupt_register.c
//Notes:
//Author: wu, zhigang
//Date:   May-19-2019
//*************************************************************************************
//*************************************************************************************
#include <defbf533.h>
#include <string.h>
#include <math.h>
#include "clocks.h"
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sportTask.h"
#include "interrupt_register.h"

#if 0
#define EVT_EMU		(0x0001)
#define EVT_RST		(0x0002)
#define EVT_NMI		(0x0004)
#define EVT_EVX		(0x0008)
#endif

#define MAX_NUM_INT_HANDLER		(16)
#define EMPTY_ENTRY				(0)

struct reg_interrupt_handler int_handler;

SemaphoreHandle_t Local_xSem;


struct int_tab_link int_handler_tab[MAX_NUM_INT_HANDLER] = {};

static uint8_t int_tab_start_idx[MAX_NUM_INT_HANDLER];

void interrupt_handler_init(SemaphoreHandle_t xsem)
{
	memset(int_tab_start_idx, 0, sizeof(int_tab_start_idx));
	memset(int_handler_tab, 0, sizeof(int_handler_tab));
	Local_xSem = xsem;
}

EX_INTERRUPT_HANDLER(isr_com_fcn)
{
	struct int_tab_link *node;
	uint32_t ipend;
    uint32_t sic_isr;
	uint8_t ivg;
	unsigned int cpu_sr;

	// Get copy of ipend register
	ipend = mmr_read32(pIPEND);

	// Mask off the reset and interrupt enable bits
	ipend &= ~(EVT_RST | EVT_NMI | EVT_EVX);

	ivg = 0;
	while((ipend & 0x00000001) == 0)
	{
		ipend = ipend >> 1;
		ivg++;
	}

	portTICK_TYPE_ENTER_CRITICAL();
	if (int_tab_start_idx[ivg] != EMPTY_ENTRY)
	{
		node = &int_handler_tab[int_tab_start_idx[ivg]];
		while(node != NULL)
		{
			if ((1 << node->int_handler->ivg) & mmr_read32(node->int_handler->sic_isr))
			{
				if (node->int_handler->fcn)
				{
					node->int_handler->fcn(node->int_handler->usr_data);
				}
			}

			// trace to next node
			node = node->next;
		}
	}
	portTICK_TYPE_EXIT_CRITICAL();

	return;
}


uint32_t register_interrupt_handler(struct reg_interrupt_handler *int_handler)
{
	uint32_t i;
	uint32_t ret = 0;
	unsigned int cpu_sr;

	if (xSemaphoreTake(Local_xSem, 0) == pdTRUE)
	{
		// find the null slot in the hanlder table
		i = 0;
		while((int_handler_tab[i].int_handler != NULL) && (i < MAX_NUM_INT_HANDLER))
		{
			i++;
		}
		
		if (i < MAX_NUM_INT_HANDLER) 
		{
			int_handler_tab[i].int_handler = int_handler;
			int_handler_tab[i].next = NULL;

			if (int_tab_start_idx[int_handler->ivg] == EMPTY_ENTRY)
			{	// this is the first entry for this interrupt level
				int_tab_start_idx[int_handler->ivg] = i;
				register_handler_ex(int_handler->ivg, (ex_handler_fn)isr_com_fcn, EX_INT_ENABLE);
			}
			else
			{
				struct int_tab_link *node = &int_handler_tab[int_tab_start_idx[int_handler->ivg]];

				//trace to end of link
				while(node->next != NULL)
				{
					node = node->next;
				}

				// Insert the new node at the end of list
				portTICK_TYPE_ENTER_CRITICAL();
				node->next = &int_handler_tab[i];
				portTICK_TYPE_EXIT_CRITICAL();
			}
		}
		else
		{
			ret = 1;
		}
		xSemaphoreGive(Local_xSem);
	}
	else
	{
		ret = 1;
	}

	return ret;
}





