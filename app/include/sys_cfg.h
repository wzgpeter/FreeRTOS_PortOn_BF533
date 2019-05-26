//*************************************************************************************
//File Name: sport_api.c
//Notes:
//Author: wu, zhigang
//Date:   May-21-2019
//*************************************************************************************
//*************************************************************************************
#ifndef __SYS_CFG_H__
#define __SYS_CFG_H__

#define PLL_WAKEUP_INT		(1 << 0)
#define DMA_ERR_INT			(1 << 1)
#define PPI_ERR_INT			(1 << 2)
#define SPORT0_ERR_INT      (1 << 3)
#define SPORT1_ERR_INT		(1 << 4)
#define SPI_ERR_INT			(1 << 5)
#define UART_ERR_INT		(1 << 6)
#define RT_CLOCK_INT		(1 << 7)
#define PPI_DMA0_INT		(1 << 8)
#define SPORT0_RX_DMA1_INT  (1 << 9)
#define SPORT0_TX_DMA2_INT  (1 << 10)
#define SPORT1_RX_DMA3_INT  (1 << 11)
#define SPORT1_TX_DMA4_INT  (1 << 12)
#define SPI_DMA5_INT		(1 << 13)
#define UART_RX_DMA6_INT	(1 << 14)
#define UART_TX_DMA7_INT	(1 << 15)

#endif //__SYS_CFG_H__

