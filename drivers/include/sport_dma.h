//*************************************************************************************
//*************************************************************************************
//File Name: sport_dma.h
//Notes:
//Author: wu, zhigang
//Date:   Jul-12-2017
//*************************************************************************************
//*************************************************************************************
#ifndef __SPORT_DMA_H__
#define __SPORT_DMA_H__


#define SPORT0                      (0)
#define SPORT1                      (1)
#define NUM_PHYSICAL_SPORT_PORTS    (2)


#define SPORT_RX_BIT      (0x1)
#define SPORT_TX_BIT      (0x2)
#define SPORT_ERR_BIT     (0x4)


#define SPORT0_ERR_INT      (1<<3)
#define SPORT0_DMA1_RX_INT  (1<<9)
#define SPORT0_DMA2_TX_INT  (1<<10)
#define SPORT1_ERR_INT      (1<<4)
#define SPORT1_DMA3_RX_INT  (1<<11)
#define SPORT1_DMA4_TX_INT  (1<<12)



#define SPORT_DMA_I2S_RX_INIT       (0x01)
#define SPORT_DMA_TDM_RX_INIT       (0x02)
#define SPORT_DMA_I2S_TX_INIT       (0x04)
#define SPORT_DMA_TDM_TX_INIT       (0x08)

#define SPORT_DMA_MODE_NONE			(0)
#define SPORT_DMA_MODE_I2S_RX		(1)
#define SPORT_DMA_MODE_I2S_TX		(2)
#define SPORT_DMA_MODE_TDM_PRI		(4)
#define SPORT_DMA_MODE_TDM_SEC		(8)

#define SPORT_DMA_READ              (0x01)
#define SPORT_DMA_WRITE             (0x02)



#define RX_BUFFER_NUM           (2)
#define RX_WORDS_PER_FRAME      (2)
#define RX_RAMES_PER_BLOCK      (32)
#define RX_WORDS_PER_BLOCK      (RX_WORDS_PER_FRAME * RX_RAMES_PER_BLOCK)
#define TX_BUFFER_NUM           (2)
#define TX_WORDS_PER_FRAME      (2)
#define TX_RAMES_PER_BLOCK      (32)
#define TX_WORDS_PER_BLOCK      (TX_WORDS_PER_FRAME * TX_RAMES_PER_BLOCK)




//============================================================================
//SPORT REG structure define
//============================================================================
typedef struct _sport_port_registers
{
   volatile uint16_t *tcr1;
   volatile uint16_t *tcr2;
   volatile uint16_t *rcr1;
   volatile uint16_t *rcr2;
   volatile uint16_t *stat;
   
   volatile uint16_t *tclkdiv;
   volatile uint16_t *rclkdiv;
   volatile uint16_t *tfsdiv;
   volatile uint16_t *rfsdiv;
   
   volatile uint16_t *mcmc1;
   volatile uint16_t *mcmc2;
   volatile uint16_t *chnl;
   
   volatile uint32_t *mrcs0;
   volatile uint32_t *mrcs1;
   volatile uint32_t *mrcs2;
   volatile uint32_t *mrcs3;
   
   volatile uint32_t *mtcs0;
   volatile uint32_t *mtcs1;
   volatile uint32_t *mtcs2;
   volatile uint32_t *mtcs3;
} SPORT_PORT_CONFIG;



//============================================================================
//SPORT DMA REG structure define
//============================================================================
typedef struct _sport_dma_registers
{
    volatile uint16_t *config;
    volatile uint32_t *next_desc_ptr;
    volatile uint32_t *start_addr;
    volatile uint16_t *x_count;
    volatile uint16_t *y_count;
    volatile  int16_t *x_modify;
    volatile  int16_t *y_modify;
    volatile uint32_t *curr_desc_ptr;
    volatile uint16_t *curr_y_count;
    volatile uint16_t *irq_status;
} SPORT_DMA_REGISTERS;


typedef struct sport_sic_config
{
   volatile uint32_t *sic_isr;
   volatile uint32_t *sic_imask;
   volatile uint16_t *dma_pmap;
} SPORT_SIC_CONFIG;


typedef struct _tagSPORT_DMA_CONFIG
{
	interrupt_kind      ivg_level;
	SPORT_DMA_REGISTERS rx_dma;
	SPORT_DMA_REGISTERS tx_dma;
	SPORT_SIC_CONFIG    rx_sic;
	SPORT_SIC_CONFIG    tx_sic;
} SPORT_DMA_CONFIG;


typedef struct _tagSPORT_DMA_CONTEXT
{
    uint16_t port_enabled;
    uint8_t  mode;       
    uint8_t  dir;     //the direction of transmit
    volatile uint8_t  rxblk_ptr;
    volatile uint8_t  txblk_ptr;

    SemaphoreHandle_t read_lock;
    SemaphoreHandle_t write_lock;

    SPORT_DMA_CONFIG  *dma;
    SPORT_PORT_CONFIG *port;
}SPORT_DMA_CONTEXT;


extern SPORT_DMA_CONTEXT sport_ctx;
extern int32_t AudioRxBuffer[RX_BUFFER_NUM][RX_WORDS_PER_BLOCK];
extern int32_t AudioTxBuffer[TX_BUFFER_NUM][TX_WORDS_PER_BLOCK];


int16_t sport_dma_init(SPORT_DMA_CONTEXT *pSportCtx, uint8_t option_flag, uint8_t port);
int16_t sport_dma_enable(SPORT_DMA_CONTEXT *pSportCtx, uint8_t port);
int16_t sport_dma_disable(SPORT_DMA_CONTEXT *pSportCtx, uint8_t port);

#endif //__SPORT_DMA_H__

