//*************************************************************************************
//File Name: interrupt_register.h
//Notes:
//Author: wu, zhigang
//Date:   May-19-2019
//*************************************************************************************
//*************************************************************************************
#ifndef __INTERRUPT_REGISTER_H__
#define __INTERRUPT_REGISTER_H__

typedef void (*isr_fcn)(void *);

struct reg_interrupt_handler
{
	interrupt_kind    ivg;		//the interrupt level
	volatile uint32_t *sic_isr;	//system interrupt status register
	volatile uint32_t *sic_imsk;
	isr_fcn fcn;				//the real ISR function
	void *usr_data;
};

struct int_tab_link{
	struct reg_interrupt_handler *int_handler;
	struct int_tab_link *next;
};

void interrupt_handler_init(SemaphoreHandle_t xsem);
uint32_t register_interrupt_handler(struct reg_interrupt_handler *int_handler);



#endif //__INTERRUPT_REGISTER_H__

