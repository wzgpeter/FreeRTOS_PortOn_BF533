//*************************************************************************************
//*************************************************************************************
//File Name: sport_dma.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-12-2017
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


int32_t AudioRxBuffer[RX_BUFFER_NUM][RX_WORDS_PER_BLOCK];
int32_t AudioTxBuffer[TX_BUFFER_NUM][TX_WORDS_PER_BLOCK];


#ifndef MCMEN
#define MCMEN		0x0010 	/*Multichannel Frame Mode Enable */
#endif

static void sport_dma_rx_isr(void *usr_data);
static void sport_dma_tx_isr(void *usr_data);


struct reg_interrupt_handler sport0_dma1_rx_int_cfg = 
{
	.ivg = ik_ivg9,
	.sic_isr = pSIC_ISR,
	.sic_imsk = pSIC_IMASK,
	.fcn = sport_dma_rx_isr,
	.usr_data = NULL,
};

struct reg_interrupt_handler sport0_dma2_tx_int_cfg = 
{
	.ivg = ik_ivg9,
	.sic_isr = pSIC_ISR,
	.sic_imsk = pSIC_IMASK,
	.fcn = sport_dma_tx_isr,
	.usr_data = NULL,
};


//*************************************************************************************
//* Function: sport_dma_rx_isr
//* Description: handle the sport DMA RX interruption 
//* Return: NONE
//*************************************************************************************
static void sport_dma_rx_isr(void *usr_data)
{
	uint8_t err;
    struct sport_ctx *pSportCtx;
    SPORT_DMA_CONFIG  *pDma;
    SPORT_PORT_CONFIG *pPort;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    pSportCtx = (struct sport_ctx *)usr_data;
    pDma      = pSportCtx->dma;
    pPort     = pSportCtx->port;
   
    if( SPORT_DMA_READ == (pSportCtx->dir & SPORT_DMA_READ) )
    {
        if( mmr_read16(pDma->rx_dma.irq_status) & DMA_DONE )
        {    
            // clear interrupt
            mmr_write16(pDma->rx_dma.irq_status, DMA_DONE); 

            // Post the event flag
            xTaskNotifyFromISR(xSportTaskHandle, SPORT_RX_BIT, eSetBits, &xHigherPriorityTaskWoken);

            //switch in the higher priority task, if higher task is ready
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);      
		}
	}
}



//*************************************************************************************
//* Function: sport_dma_tx_isr
//* Description: handle the sport DMA TX interruption 
//* Return: NONE
//*************************************************************************************
static void sport_dma_tx_isr(void *usr_data)
{
    uint8_t err;
    struct sport_ctx *pSportCtx;
    SPORT_DMA_CONFIG  *pDma;
    SPORT_PORT_CONFIG *pPort;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    pSportCtx = (struct sport_ctx *)usr_data;
    pDma      = pSportCtx->dma;
    pPort     = pSportCtx->port;
   
    if( SPORT_DMA_WRITE == (pSportCtx->dir & SPORT_DMA_WRITE) )
    {
        if( mmr_read16(pDma->tx_dma.irq_status) & DMA_DONE )
        {
            pSportCtx->txblk_ptr ^= 1;

            // clear interrupt
            mmr_write16(pDma->tx_dma.irq_status, DMA_DONE);

            // Post the event flag
            xTaskNotifyFromISR(xSportTaskHandle, SPORT_TX_BIT, eSetBits, &xHigherPriorityTaskWoken);

            //switch in the higher priority task, if higher task is ready
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}


#if 0
EX_INTERRUPT_HANDLER(sport_dma_isr)
{
    uint32_t ipend;
    uint32_t sic_isr;

    SPORT_DMA_CONFIG  *pDma;
    SPORT_PORT_CONFIG *pPort;
    pDma  = sport_dma.dma;
    pPort = sport_dma.port;

    ipend = mmr_read32(pIPEND);
    sic_isr = mmr_read32(pDma->rx_sic.sic_isr);

    if(sic_isr & SPORT0_DMA1_RX_INT)
    {
        sport_dma_rx_isr((void *)&sport_dma);
    }
    else if(sic_isr & SPORT0_DMA2_TX_INT)
    {
        sport_dma_tx_isr((void *)&sport_dma);
    }
    else if(sic_isr & SPORT0_ERR_INT)
    {
        //TODO ...
    }
}
#endif


static void sport_dma_clear(struct sport_ctx *pSportCtx)
{
	// reset SPORT
	if (    (pSportCtx->mode & SPORT_DMA_MODE_I2S_RX)
         || (pSportCtx->mode & SPORT_DMA_MODE_TDM_PRI))
	{
		mmr_write16(pSportCtx->port->rcr1,  0);
		mmr_write16(pSportCtx->port->rcr2,  0);
		mmr_write32(pSportCtx->port->mrcs0, 0);
		mmr_write32(pSportCtx->port->mrcs1, 0);
		mmr_write32(pSportCtx->port->mrcs2, 0);
		mmr_write32(pSportCtx->port->mrcs3, 0);
		mmr_write16(pSportCtx->port->stat, 
                    mmr_read16(pSportCtx->port->stat) | RXNE | RUVF | ROVF);

		mmr_write32(pSportCtx->dma->rx_dma.next_desc_ptr, 0);
		mmr_write32(pSportCtx->dma->rx_dma.start_addr,    0);
		mmr_write16(pSportCtx->dma->rx_dma.x_count,       0);
		mmr_write16(pSportCtx->dma->rx_dma.x_modify,      0);
		mmr_write16(pSportCtx->dma->rx_dma.y_count,       0);
		mmr_write16(pSportCtx->dma->rx_dma.y_modify,      0);
	}
    
	if (   (pSportCtx->mode & SPORT_DMA_MODE_I2S_TX)
        || (pSportCtx->mode & SPORT_DMA_MODE_TDM_PRI) )
	{
		mmr_write16(pSportCtx->port->tcr1,  0);
		mmr_write16(pSportCtx->port->tcr2,  0);
		mmr_write16(pSportCtx->port->rcr1,  0);
		mmr_write32(pSportCtx->port->mtcs0, 0);
		mmr_write32(pSportCtx->port->mtcs1, 0);
		mmr_write32(pSportCtx->port->mtcs2, 0);
		mmr_write32(pSportCtx->port->mtcs3, 0);
		mmr_write16(pSportCtx->port->stat,
                    mmr_read16(pSportCtx->port->stat) | TXF | TUVF | TOVF);

		mmr_write16(pSportCtx->dma->tx_dma.config,        0);
		mmr_write32(pSportCtx->dma->tx_dma.next_desc_ptr, 0);
		mmr_write32(pSportCtx->dma->tx_dma.start_addr,    0);
		mmr_write16(pSportCtx->dma->tx_dma.x_count,       0);
		mmr_write16(pSportCtx->dma->tx_dma.x_modify,      0);
		mmr_write16(pSportCtx->dma->tx_dma.y_count,       0);
		mmr_write16(pSportCtx->dma->tx_dma.y_modify,      0);
	}

	if ( (pSportCtx->mode & SPORT_DMA_MODE_TDM_PRI) )
	{
		mmr_write16(pSportCtx->port->mcmc1, 0);
		mmr_write16(pSportCtx->port->mcmc2, 0);
	}
	
	return;
}


static void sport_ioctl(struct sport_dev *dev, uint8_t cmd, uint32_t val)
{
	struct sport_ctx *pSportCtx = (struct sport_ctx *)dev->ctx;
	
	switch(cmd)
	{	
	case SPORT_INIT:
		//sport_dma_init(pSportCtx, val);
		pSportCtx->rxblk_ptr = 1;
		pSportCtx->txblk_ptr = 1;

		sport0_dma1_rx_int_cfg.usr_data = (void *)pSportCtx;
		sport0_dma2_tx_int_cfg.usr_data = (void *)pSportCtx;
		
		mmr_write32(pSportCtx->dma->rx_sic.sic_imask,
						mmr_read32(pSportCtx->dma->rx_sic.sic_imask) | DMA1_IRQ );
		break;

	case SPORT_CFG_PORT_RCR1:
		mmr_write16( pSportCtx->port->rcr1, (uint16_t)val );
		break;
	case SPORT_CFG_PORT_RCR2:
		mmr_write16( pSportCtx->port->rcr2, (uint16_t)val );
		break;
	case SPORT_CFG_PORT_TCR1:
		mmr_write16( pSportCtx->port->tcr1, (uint16_t)val );
		break;
	case SPORT_CFG_PORT_TCR2:
		mmr_write16( pSportCtx->port->tcr2, (uint16_t)val );
		break;
	case SPORT_CFG_PORT_MRCS0:
		mmr_write32( pSportCtx->port->mrcs0, (uint32_t)val );
		break;
	case SPORT_CFG_PORT_MTCS0:
		mmr_write32( pSportCtx->port->mtcs0, (uint32_t)val );
		break;
	case SPORT_CFG_PORT_MCMC1:
		mmr_write16( pSportCtx->port->mcmc1, (uint16_t)val );
		break;
	case SPORT_CFG_PORT_MCMC2:
		mmr_write16( pSportCtx->port->mcmc2, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_RX_SADDR:
		mmr_write32( pSportCtx->dma->rx_dma.start_addr, (uint32_t)val);
		break;
	case SPORT_CFG_DMA_RX_XCOUNT:
		mmr_write16( pSportCtx->dma->rx_dma.x_count, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_RX_XMODIFY:
		mmr_write16( pSportCtx->dma->rx_dma.x_modify, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_RX_YCOUNT:
		mmr_write16( pSportCtx->dma->rx_dma.y_count, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_RX_YMODIFY:
		mmr_write16( pSportCtx->dma->rx_dma.y_modify, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_RX_CONFIG:
		mmr_write16( pSportCtx->dma->rx_dma.config, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_RX_PMAP:
		mmr_write16( pSportCtx->dma->rx_sic.dma_pmap, (uint16_t)val );
		break;
	
	case SPORT_CFG_DMA_TX_SADDR:
		mmr_write32( pSportCtx->dma->tx_dma.start_addr, (uint32_t)val);
		break;
	case SPORT_CFG_DMA_TX_XCOUNT:
		mmr_write16( pSportCtx->dma->tx_dma.x_count, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_TX_XMODIFY:
		mmr_write16( pSportCtx->dma->tx_dma.x_modify, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_TX_YCOUNT:
		mmr_write16( pSportCtx->dma->tx_dma.y_count, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_TX_YMODIFY:
		mmr_write16( pSportCtx->dma->tx_dma.y_modify, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_TX_CONFIG:
		mmr_write16( pSportCtx->dma->tx_dma.config, (uint16_t)val );
		break;
	case SPORT_CFG_DMA_TX_PMAP:
		mmr_write16( pSportCtx->dma->tx_sic.dma_pmap, (uint16_t)val );
		break;

	case SPORT_MODE:
		pSportCtx->mode = val;
		break;
	case SPORT_DIRT:
		pSportCtx->dir = val;
		break;

	default:
		break;
	}
}


const struct sport_cfg sport0_rx_cfg[] =
{
	//PORT CFG
	{SPORT_CFG_PORT_RCR1, (uint16_t)( RFSR | RCKFE)},
	{SPORT_CFG_PORT_RCR2, (uint16_t)((SLEN&31) | RXSE | RSFSE )},
	{SPORT_CFG_PORT_MRCS0, (uint32_t)0x00000000},
	{SPORT_CFG_PORT_MCMC1, (uint16_t)0x0000},
	{SPORT_CFG_PORT_MCMC2, (uint16_t)0x0000},
	//DMA RX
	{SPORT_CFG_DMA_RX_SADDR, (uint32_t)AudioRxBuffer},
	{SPORT_CFG_DMA_RX_XCOUNT, RX_WORDS_PER_BLOCK},
	{SPORT_CFG_DMA_RX_XMODIFY, 4},
	{SPORT_CFG_DMA_RX_YCOUNT, RX_BUFFER_NUM},
	{SPORT_CFG_DMA_RX_YMODIFY, 4},
	{SPORT_CFG_DMA_RX_CONFIG, (uint16_t)(WNR | WDSIZE_32 | DI_EN | FLOW_AUTO | DI_SEL | DMA2D)},
	{SPORT_CFG_DMA_RX_PMAP, PMAP_SPORT0RX},
	{SPORT_CFG_END, 0},
};

const struct sport_cfg sport0_tx_cfg[] =
{
	//PORT CFG
	{SPORT_CFG_PORT_TCR1, (uint16_t)(TFSR | TCKFE)},
	{SPORT_CFG_PORT_TCR2, (uint16_t)((SLEN&31) | TXSE | TSFSE)},
	{SPORT_CFG_PORT_MTCS0, (uint32_t)0x00000000},
	{SPORT_CFG_PORT_MCMC1, (uint16_t)0x0000},
	{SPORT_CFG_PORT_MCMC2, (uint16_t)0x0000},
	//DMA TX
	{SPORT_CFG_DMA_TX_SADDR, (uint32_t)AudioTxBuffer},
	{SPORT_CFG_DMA_TX_XCOUNT, TX_WORDS_PER_BLOCK},
	{SPORT_CFG_DMA_TX_XMODIFY, 4},
	{SPORT_CFG_DMA_TX_YCOUNT, TX_BUFFER_NUM},
	{SPORT_CFG_DMA_TX_YMODIFY, 4},
	{SPORT_CFG_DMA_TX_CONFIG, (uint16_t)(WDSIZE_32 | FLOW_AUTO | DMA2D)},
	{SPORT_CFG_DMA_TX_PMAP, PMAP_SPORT0TX},
	{SPORT_CFG_END, 0},
};

#if 0
//*************************************************************************************
//* Function: sport_dma_init
//* Description: initialize the sport dma module 
//* Return: NONE
//*************************************************************************************
static void sport_dma_init(struct sport_ctx *pSportCtx, uint32_t port)
{
    uint32_t i;

    //clear registers to initilization state
    sport_dma_clear(pSportCtx);

    mmr_write32(pSportCtx->dma->rx_sic.sic_imask,
                    mmr_read32(pSportCtx->dma->rx_sic.sic_imask) | DMA1_IRQ );

    //==================================
    //SPORT PORT Configuration

    //Active Low RFS; Require RFS for each data; Clock falling edge
    mmr_write16( pSportCtx->port->rcr1, ( RFSR | RCKFE) );

    //Serial word length=16
    mmr_write16( pSportCtx->port->rcr2, (SLEN&31) | RXSE | RSFSE );

    //Active Low RFS; Require TFS for each data; Clock falling edge
    mmr_write16( pSportCtx->port->tcr1, (TFSR | TCKFE) );

    //Serial word length=16
    mmr_write16( pSportCtx->port->tcr2, (SLEN&31) | TXSE | TSFSE );

    //Receive on Channel1 and Channel2
    mmr_write32( pSportCtx->port->mrcs0, (0x00000000) );

    //Transmit on Channel1 and Channel2
    mmr_write32( pSportCtx->port->mtcs0, (0x00000000) );

    
    mmr_write16( pSportCtx->port->mcmc1, (0x0000) );

    //Enable Multi-Chan Mode; 
    mmr_write16( pSportCtx->port->mcmc2, (0x0000) );


    //==================================
    //SPORT DMA Configuration

    //Set RX DMA start address
    mmr_write32( pSportCtx->dma->rx_dma.start_addr, ((uint32_t)AudioRxBuffer) );

    //Set RX X Count register and X Modify register
    mmr_write16( pSportCtx->dma->rx_dma.x_count, (RX_WORDS_PER_BLOCK) );
    mmr_write16( pSportCtx->dma->rx_dma.x_modify, (4) );

    //Set RX Y Count Register and Y Modify register
    mmr_write16( pSportCtx->dma->rx_dma.y_count, (RX_BUFFER_NUM) );
    mmr_write16( pSportCtx->dma->rx_dma.y_modify, (4) );


    //Set TX DMA start address
    mmr_write32( pSportCtx->dma->tx_dma.start_addr, ((uint32_t)AudioTxBuffer) );

    //Set TX X Count register and X Modify register
    mmr_write16( pSportCtx->dma->tx_dma.x_count, (TX_WORDS_PER_BLOCK) );
    mmr_write16( pSportCtx->dma->tx_dma.x_modify, (4) );

    //Set TX Y Count Register and Y Modify register
    mmr_write16( pSportCtx->dma->tx_dma.y_count, (TX_BUFFER_NUM) );
    mmr_write16( pSportCtx->dma->tx_dma.y_modify, (4) );


    //Set RX DMA Config register
    //Memory Write; 32-Bit; Allow Generate Interruption; AutoBuffer Mode; Interruption for Each Row; 2-Dimension
    mmr_write16( pSportCtx->dma->rx_dma.config, (WNR | WDSIZE_32 | DI_EN | FLOW_AUTO | DI_SEL | DMA2D) );

    //Set TX DMA Config register
    //Memory Read; 32-Bit; AutoBuffer Mode; No Interruption for Each Row; 2-Dimension;
    mmr_write16( pSportCtx->dma->tx_dma.config, (WDSIZE_32 | FLOW_AUTO | DMA2D) );

    //Map SPORT0 Receive DMA
    mmr_write16( pSportCtx->dma->rx_sic.dma_pmap, (PMAP_SPORT0RX) );

    //Map SPORT0 Transmit DMA
    mmr_write16( pSportCtx->dma->tx_sic.dma_pmap, (PMAP_SPORT0TX) );
}
#endif

static void sport_open(struct sport_dev *dev, uint8_t port)
{
	struct sport_ctx *pSportCtx = (struct sport_ctx *)dev->ctx;

	//enable Tx first
    if( pSportCtx->mode & SPORT_DMA_MODE_I2S_TX )
    {
        mmr_write16( pSportCtx->dma->tx_dma.config, mmr_read16(pSportCtx->dma->tx_dma.config) | DMAEN );     
        mmr_write16( pSportCtx->port->tcr1, mmr_read16(pSportCtx->port->tcr1) | TSPEN );
    }

	//enable Rx second
    if( pSportCtx->mode & SPORT_DMA_MODE_I2S_RX )
    {
        mmr_write16( pSportCtx->dma->rx_dma.config, mmr_read16(pSportCtx->dma->rx_dma.config) | DMAEN );     
        mmr_write16( pSportCtx->port->rcr1, mmr_read16(pSportCtx->port->rcr1) | RSPEN );
    }

    if( pSportCtx->mode & SPORT_DMA_MODE_TDM_PRI )
    {
        mmr_write16( pSportCtx->dma->rx_dma.config, mmr_read16(pSportCtx->dma->rx_dma.config) | DMAEN );
        mmr_write16( pSportCtx->dma->tx_dma.config, mmr_read16(pSportCtx->dma->tx_dma.config) | DMAEN );
    }
}

static void sport_close(struct sport_dev *dev, uint8_t port)
{
	struct sport_ctx *pSportCtx = (struct sport_ctx *)dev->ctx;
	
    if( (SPORT_DMA_READ         == pSportCtx->dir) ||
        (SPORT_DMA_MODE_TDM_PRI == pSportCtx->mode) )
    {
        mmr_write16( pSportCtx->dma->rx_dma.config, mmr_read16(pSportCtx->dma->rx_dma.config) & ~DMAEN );
        mmr_write16( pSportCtx->port->rcr1, mmr_read16(pSportCtx->port->rcr1) & ~RSPEN );
    }
    
    if( (SPORT_DMA_WRITE        == pSportCtx->dir) ||
        (SPORT_DMA_MODE_TDM_PRI == pSportCtx->mode) )
    {
        mmr_write16( pSportCtx->dma->tx_dma.config, mmr_read16(pSportCtx->dma->tx_dma.config) & ~DMAEN );
        mmr_write16( pSportCtx->port->tcr1, mmr_read16(pSportCtx->port->tcr1) & ~TSPEN );
    }
}

static void sport_read(struct sport_dev *dev, uint8_t *buf, uint32_t len)
{
	struct sport_ctx *pSportCtx = (struct sport_ctx *)dev->ctx;
}

static void sport_write(struct sport_dev *dev, uint8_t *buf, uint32_t len)
{
	struct sport_ctx *pSportCtx = (struct sport_ctx *)dev->ctx;
}

const struct sport_ops sport_dev_api = {
    .open = sport_open,
    .close = sport_close,
    .read = sport_read,
    .write = sport_write,
    .ioctl = sport_ioctl,
};

