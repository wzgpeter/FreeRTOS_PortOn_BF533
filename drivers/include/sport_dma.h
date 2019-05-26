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


#define NUM_PHYSICAL_SPORT_PORTS    (2)


#define SPORT_RX_BIT      (0x1)
#define SPORT_TX_BIT      (0x2)
#define SPORT_ERR_BIT     (0x4)



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


struct sport_ctx
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
};

extern const struct sport_ops sport_dev_api;

#endif //__SPORT_DMA_H__
