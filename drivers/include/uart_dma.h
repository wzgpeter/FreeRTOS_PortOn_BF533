//*************************************************************************************
//*************************************************************************************
//File Name: uart.c
//Notes:
//Author: wu, zhigang
//Date:   May-14-2017
//*************************************************************************************
//*************************************************************************************
#ifndef __UART_DMA_H__
#define __UART_DMA_H__


#define UART_DMA_NONE  (0)
#define UART_DMA_READ  (1)
#define UART_DMA_WRITE (2)


// UART_DMA_CMD_ENABLE
#define UART_DMA_DISABLED  (0x00)
#define UART_DMA_ENABLED   (0x01)


#define UART_DMA_IOCTL_UNLOCKED  (0)
#define UART_DMA_IOCTL_LOCKED    (1)

#define UART_DMA_NO_WAIT_DMA_RUN_CLEAR   (0x00)
#define UART_DMA_WAIT_DMA_RUN_CLEAR      (0x01)


#define UART_IER_CLEAR_ALL_INT   0x0000
#define UART_DMA_DMA_CTL_POR_VALUE (0x0000)
#define UART_DMA_LSR_ERROR_MASK  (0x1E)



//Error codes
#define UART_DMA_OK                    (0)
#define UART_DMA_ERR_NO_FREE_OCB      (-1)
#define UART_DMA_ERR_INVALID_IOCTL    (-2)
#define UART_DMA_ERR_BAD_IOCTL_VALUE  (-3)
#define UART_DMA_ERR_NOT_READY        (-4)
#define UART_DMA_ERR_BAD_PORT         (-5)
#define UART_DMA_ERR_INVALID_STATE    (-6)
#define UART_DMA_ERR_OVERRUN_ERROR    (-7)
#define UART_DMA_ERR_PARITY_ERROR     (-8)
#define UART_DMA_ERR_FRAMING_ERROR    (-9)
#define UART_DMA_ERR_BREAK_INTERRUPT  (-10)
#define UART_DMA_ERR_NULL_OS_SEM_PTR  (-11)
#define UART_DMA_ERR_NULL_OS_FLAG_PTR (-12)
#define UART_DMA_ERR_CANCELLED        (-13)



#define UART_DMA_REGISTERS		\
{ 								\
    pDMA6_CONFIG,               \
    (uint32_t*)pDMA6_START_ADDR,\
    pDMA6_X_COUNT,              \
    pDMA6_X_MODIFY,             \
    pDMA6_IRQ_STATUS,	        \
                                \
    pDMA7_CONFIG,               \
    (uint32_t*)pDMA7_START_ADDR,\
    pDMA7_X_COUNT,              \
    pDMA7_X_MODIFY,             \
    pDMA7_IRQ_STATUS,	        \
                                \
    ik_ivg10,                   \
    pSIC_ISR,                   \
    pSIC_IMASK,                 \
    pDMA6_PERIPHERAL_MAP,       \
    pDMA7_PERIPHERAL_MAP,       \
                                \
    ik_ivg7,                    \
    pSIC_ISR,                   \
    pSIC_IMASK,                 \
}


#define UART_PORT_REGISTERS    \
{                              \
    pUART_LCR,                 \
    pUART_MCR,                 \
    pUART_LSR,                 \
    pUART_THR,                 \
    pUART_RBR,                 \
    pUART_IER,                 \
    pUART_DLL,                 \
    pUART_DLH,                 \
    pUART_SCR,                 \
    pUART_GCTL,                \
}


typedef struct _tagUART_DMA_CONFIG
{
    volatile uint16_t *rx_dma_config;
    volatile uint32_t *rx_dma_start_addr;
    volatile uint16_t *rx_dma_x_count;
    volatile int16_t  *rx_dma_x_modify;
    volatile uint16_t *rx_dma_irq_status;

    volatile uint16_t *tx_dma_config;
    volatile uint32_t *tx_dma_start_addr;
    volatile uint16_t *tx_dma_x_count;
    volatile int16_t  *tx_dma_x_modify;
    volatile uint16_t *tx_dma_irq_status;

    //for Tx and Rx interrupts
    interrupt_kind    ivg_level;
    volatile uint32_t *sic_isr;
    volatile uint32_t *sic_imask;
    volatile uint16_t *rx_pmap;
    volatile uint16_t *tx_pmap;

    //For Status interrupts
    interrupt_kind    ivg_err_level;
    volatile uint32_t *sic_err_isr;
    volatile uint32_t *sic_err_imask;
} UART_DMA_CONFIG;


typedef struct _tagUART_PORT_CONFIG
{
    volatile uint16_t *lcr;
    volatile uint16_t *mcr;
    volatile uint16_t *lsr;
    volatile uint16_t *thr;
    volatile uint16_t *rbr;
    volatile uint16_t *ier;
    volatile uint16_t *dll;
    volatile uint16_t *dlh;
    volatile uint16_t *scr;
    volatile uint16_t *gctl;
}UART_PORT_CONFIG;



typedef struct _tagUART_DMA_CONTEXT
{
    uint16_t port_enabled;
    uint8_t  rw;         //specify read or write operation

    SemaphoreHandle_t read_lock;
    SemaphoreHandle_t write_lock;

    UART_DMA_CONFIG  *dma;
    UART_PORT_CONFIG *port;
}UART_DMA_CONTEXT;



void* uart_dma_init( void );
BaseType_t uart_dma_enable(UART_DMA_CONTEXT *uart_dma_ocb);
BaseType_t uart_dma_read(void *ocb, void *buffer, uint32_t len);
BaseType_t uart_dma_write(void *ocb, void *buffer, uint32_t len);


#endif //__UART_DMA_H__

