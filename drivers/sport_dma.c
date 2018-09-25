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
#include "clocks.h"
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sportTask.h"
#include "sport_dma.h"


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


int32_t AudioRxBuffer[RX_BUFFER_NUM][RX_WORDS_PER_BLOCK];
int32_t AudioTxBuffer[TX_BUFFER_NUM][TX_WORDS_PER_BLOCK];


static SPORT_PORT_CONFIG sport_port_cfg[NUM_PHYSICAL_SPORT_PORTS] =
{
    SPORT0_REG_INIT,
    SPORT1_REG_INIT
};


static SPORT_DMA_CONFIG sport_dma_cfg[NUM_PHYSICAL_SPORT_PORTS] = 
{
    SPORT0_DMA_INIT,
    SPORT1_DMA_INIT
};

SPORT_DMA_CONTEXT sport_ctx = {0};


#ifndef MCMEN
#define MCMEN		0x0010 	/*Multichannel Frame Mode Enable */
#endif




//*************************************************************************************
//* Function: sport_dma_rx_isr
//* Description: handle the sport DMA RX interruption 
//* Return: NONE
//*************************************************************************************
static void sport_dma_rx_isr(void *usr_data)
{
	uint8_t err;
    SPORT_DMA_CONTEXT *pSportCtx;
    SPORT_DMA_CONFIG  *pDma;
    SPORT_PORT_CONFIG *pPort;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    pSportCtx = (SPORT_DMA_CONTEXT *)usr_data;
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
    SPORT_DMA_CONTEXT *pSportCtx;
    SPORT_DMA_CONFIG  *pDma;
    SPORT_PORT_CONFIG *pPort;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    pSportCtx = (SPORT_DMA_CONTEXT *)usr_data;
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



EX_INTERRUPT_HANDLER(sport_dma_isr)
{
    uint32_t ipend;
    uint32_t sic_isr;

    SPORT_DMA_CONFIG  *pDma;
    SPORT_PORT_CONFIG *pPort;
    pDma  = sport_ctx.dma;
    pPort = sport_ctx.port;

    ipend = mmr_read32(pIPEND);
    sic_isr = mmr_read32(pDma->rx_sic.sic_isr);

    if(sic_isr & SPORT0_DMA1_RX_INT)
    {
        sport_dma_rx_isr((void *)&sport_ctx);
    }
    else if(sic_isr & SPORT0_DMA2_TX_INT)
    {
        sport_dma_tx_isr((void *)&sport_ctx);
    }
    else if(sic_isr & SPORT0_ERR_INT)
    {
        //TODO ...
    }
}



static void sport_dma_clear(SPORT_DMA_CONTEXT *pSportCtx)
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

    pSportCtx->rxblk_ptr = 1;
    pSportCtx->txblk_ptr = 1;

	return;
}



//*************************************************************************************
//* Function: sport_dma_init
//* Description: initialize the sport dma module 
//* Return: NONE
//*************************************************************************************
int16_t sport_dma_init(SPORT_DMA_CONTEXT *pSportCtx, uint8_t option_flag, uint8_t port)
{
    uint32_t i;


    pSportCtx->dma  = &sport_dma_cfg[port];
    pSportCtx->port = &sport_port_cfg[port];

    pSportCtx->dir = SPORT_DMA_READ | SPORT_DMA_WRITE;

    //clear registers to initilization state
    sport_dma_clear(pSportCtx);

    if( (option_flag & SPORT_DMA_I2S_RX_INIT)||
        (option_flag & SPORT_DMA_TDM_RX_INIT) )
    {
        
        //register the SPORT DMA rx ISR
        register_handler_ex(pSportCtx->dma->ivg_level, (ex_handler_fn)sport_dma_isr, EX_INT_ENABLE);
    }

    if( (option_flag & SPORT_DMA_I2S_TX_INIT)||
        (option_flag & SPORT_DMA_TDM_TX_INIT) )
    {
        
        //register the SPORT DMA rx ISR
        register_handler_ex(pSportCtx->dma->ivg_level, (ex_handler_fn)sport_dma_isr, EX_INT_ENABLE);
    }

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

    return true;
}

int16_t sport_dma_enable(SPORT_DMA_CONTEXT *pSportCtx, uint8_t port)
{
    if( pSportCtx->mode & SPORT_DMA_MODE_I2S_RX )
    {
        mmr_write16( pSportCtx->dma->rx_dma.config, mmr_read16(pSportCtx->dma->rx_dma.config) | DMAEN );     
        mmr_write16( pSportCtx->port->rcr1, mmr_read16(pSportCtx->port->rcr1) | RSPEN );
    }
   
    if( pSportCtx->mode & SPORT_DMA_MODE_I2S_TX )
    {
        mmr_write16( pSportCtx->dma->tx_dma.config, mmr_read16(pSportCtx->dma->tx_dma.config) | DMAEN );     
        mmr_write16( pSportCtx->port->tcr1, mmr_read16(pSportCtx->port->tcr1) | TSPEN );
    }
   
    if( pSportCtx->mode & SPORT_DMA_MODE_TDM_PRI )
    {
        mmr_write16( pSportCtx->dma->rx_dma.config, mmr_read16(pSportCtx->dma->rx_dma.config) | DMAEN );
        mmr_write16( pSportCtx->dma->tx_dma.config, mmr_read16(pSportCtx->dma->tx_dma.config) | DMAEN );
    }

    return true;
}

int16_t sport_dma_disable(SPORT_DMA_CONTEXT *pSportCtx, uint8_t port)
{
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

    return true;
}
