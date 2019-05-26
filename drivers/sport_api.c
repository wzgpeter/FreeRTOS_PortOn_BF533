//*************************************************************************************
//File Name: sport_api.c
//Notes:
//Author: wu, zhigang
//Date:   May-19-2019
//*************************************************************************************
//*************************************************************************************
#include <defbf533.h>
#include <string.h>
#include <math.h>
#include "sys_cfg.h"

#include "clocks.h"
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sportTask.h"
#include "sport_dma.h"
#include "sport_api.h"
#include "interrupt_register.h"


#define SPORT0_REG_INIT \
{                       \
    pSPORT0_TCR1,       \
    pSPORT0_TCR2,       \
    pSPORT0_RCR1,       \
    pSPORT0_RCR2,       \
    pSPORT0_STAT,       \
                        \
    pSPORT0_TCLKDIV,    \
    pSPORT0_RCLKDIV,    \
    pSPORT0_TFSDIV,     \
    pSPORT0_RFSDIV,     \
                        \
    pSPORT0_MCMC1,      \
    pSPORT0_MCMC2,      \
    pSPORT0_CHNL,       \
                        \
    pSPORT0_MRCS0,      \
    pSPORT0_MRCS1,      \
    pSPORT0_MRCS2,      \
    pSPORT0_MRCS3,      \
                        \
    pSPORT0_MTCS0,      \
    pSPORT0_MTCS1,      \
    pSPORT0_MTCS2,      \
    pSPORT0_MTCS3       \
}


#define SPORT1_REG_INIT \
{                       \
    pSPORT1_TCR1,       \
    pSPORT1_TCR2,       \
    pSPORT1_RCR1,       \
    pSPORT1_RCR2,       \
    pSPORT1_STAT,       \
                        \
    pSPORT1_TCLKDIV,    \
    pSPORT1_RCLKDIV,    \
    pSPORT1_TFSDIV,     \
    pSPORT1_RFSDIV,     \
                        \
    pSPORT1_MCMC1,      \
    pSPORT1_MCMC2,      \
    pSPORT1_CHNL,       \
                        \
    pSPORT1_MRCS0,      \
    pSPORT1_MRCS1,      \
    pSPORT1_MRCS2,      \
    pSPORT1_MRCS3,      \
                        \
    pSPORT1_MTCS0,      \
    pSPORT1_MTCS1,      \
    pSPORT1_MTCS2,      \
    pSPORT1_MTCS3       \
}



#define SPORT0_DMA_INIT                 \
{                                       \
    ik_ivg9,                            \
                                        \
    /* RX DMA */                        \
    pDMA1_CONFIG,                       \
    (uint32_t *)pDMA1_NEXT_DESC_PTR,    \
    (uint32_t *)pDMA1_START_ADDR,       \
    pDMA1_X_COUNT,                      \
    pDMA1_Y_COUNT,                      \
    pDMA1_X_MODIFY,                     \
    pDMA1_Y_MODIFY,                     \
    (uint32_t *)pDMA1_CURR_DESC_PTR,    \
    pDMA1_CURR_Y_COUNT,                 \
    pDMA1_IRQ_STATUS,                   \
                                        \
    /* TX DMA */                        \
    pDMA2_CONFIG,                       \
    (uint32_t *)pDMA2_NEXT_DESC_PTR,    \
    (uint32_t *)pDMA2_START_ADDR,       \
    pDMA2_X_COUNT,                      \
    pDMA2_Y_COUNT,                      \
    pDMA2_X_MODIFY,                     \
    pDMA2_Y_MODIFY,                     \
    (uint32_t *)pDMA2_CURR_DESC_PTR,    \
    pDMA2_CURR_Y_COUNT,                 \
    pDMA2_IRQ_STATUS,                   \
                                        \
    pSIC_ISR,                           \
    pSIC_IMASK,                         \
    pDMA1_PERIPHERAL_MAP,               \
    pSIC_ISR,                           \
    pSIC_IMASK,                         \
    pDMA2_PERIPHERAL_MAP,               \
}

#define SPORT1_DMA_INIT                 \
{                                       \
    ik_ivg9,                            \
                                        \
    /* RX DMA */                        \
    pDMA3_CONFIG,                       \
    (uint32_t *)pDMA3_NEXT_DESC_PTR,    \
    (uint32_t *)pDMA3_START_ADDR,       \
    pDMA3_X_COUNT,                      \
    pDMA3_Y_COUNT,                      \
    pDMA3_X_MODIFY,                     \
    pDMA3_Y_MODIFY,                     \
    (uint32_t *)pDMA3_CURR_DESC_PTR,    \
    pDMA3_CURR_Y_COUNT,                 \
    pDMA3_IRQ_STATUS,                   \
                                        \
    /* TX DMA */                        \
    pDMA4_CONFIG,                       \
    (uint32_t *)pDMA4_NEXT_DESC_PTR,    \
    (uint32_t *)pDMA4_START_ADDR,       \
    pDMA4_X_COUNT,                      \
    pDMA4_Y_COUNT,                      \
    pDMA4_X_MODIFY,                     \
    pDMA4_Y_MODIFY,                     \
    (uint32_t *)pDMA4_CURR_DESC_PTR,    \
    pDMA4_CURR_Y_COUNT,                 \
    pDMA4_IRQ_STATUS,                   \
                                        \
    pSIC_ISR,                           \
    pSIC_IMASK,                         \
    pDMA3_PERIPHERAL_MAP,               \
    pSIC_ISR,                           \
    pSIC_IMASK,                         \
    pDMA4_PERIPHERAL_MAP,               \
}


static SPORT_PORT_CONFIG sport_port_cfg[SPORT_PORT_NUM] =
{
    SPORT0_REG_INIT,
    SPORT1_REG_INIT
};


static SPORT_DMA_CONFIG sport_dma_cfg[SPORT_PORT_NUM] = 
{
    SPORT0_DMA_INIT,
    SPORT1_DMA_INIT
};


static struct sport_ctx context[SPORT_PORT_NUM] = 
{	
	{
		.dma = &sport_dma_cfg[SPORT0],
		.port = &sport_port_cfg[SPORT0],
	}, 
	{
		.dma = &sport_dma_cfg[SPORT1],
		.port = &sport_port_cfg[SPORT1],
	} 
};

static struct sport_dev sport[SPORT_PORT_NUM] = 
{
	{
		.type = SPORT,
		.ops = &sport_dev_api,
		.ctx = &context[SPORT0],
	},
	{
		.type = SPORT,
		.ops = &sport_dev_api,
		.ctx = &context[SPORT1],
	}
};

struct sport_dev * sport_get(uint8_t type, uint32_t port)
{
	if (port < SPORT_PORT_NUM && type == SPORT)
		return &sport[port];

	return NULL;
}

void sport_proc(struct sport_dev *dev)
{
	uint32_t i;

	struct sport_ctx *pCtx = (struct sport_ctx *)dev->ctx;
	for(i=0; i<RX_WORDS_PER_BLOCK; i++)
	{
		AudioTxBuffer[pCtx->txblk_ptr][i] = AudioRxBuffer[pCtx->rxblk_ptr][i];
	}
	
	pCtx->rxblk_ptr ^= 1;
}


void aud_open(struct sport_dev *dev, uint8_t port)
{
	return dev->ops->open(dev, port);
}

void aud_close(struct sport_dev *dev, uint8_t port)
{
	return dev->ops->close(dev, port);
}

void aud_read(struct sport_dev *dev, uint8_t *buf, uint32_t nLen)
{
	return dev->ops->read(dev, buf, nLen);
}

void aud_write(struct sport_dev *dev, uint8_t *buf, uint32_t nLen)
{
	return dev->ops->write(dev, buf, nLen);
}

void aud_ioctl(struct sport_dev *dev, uint8_t cmd, uint32_t val)
{
	return dev->ops->ioctl(dev, cmd, val);
}

