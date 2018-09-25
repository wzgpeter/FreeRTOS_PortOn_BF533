//*************************************************************************************
//*************************************************************************************
//File Name: uart.c
//Notes:
//Author: wu, zhigang
//Date:   May-14-2017
//*************************************************************************************
//*************************************************************************************
#include <defbf533.h>
#include <string.h>
#include <math.h>
#include "clocks.h"
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "uart_dma.h"
#include "uartTask.h"


UART_DMA_CONTEXT uart_dma_ctx = {0};

UART_PORT_CONFIG uart_port_cfg = UART_PORT_REGISTERS;

UART_DMA_CONFIG uart_dma_cfg = UART_DMA_REGISTERS;


//*************************************************************************************
//* Function: uart_dma_rx_isr
//* Description: handle the uart DMA RX interruption 
//* Return: NONE
//*************************************************************************************
static void uart_dma_rx_isr(void *usr_data)
{
    UART_DMA_CONFIG *uart_dma_cfg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uart_dma_cfg = (UART_DMA_CONFIG *)usr_data;

    if( mmr_read16(uart_dma_cfg->rx_dma_irq_status) & DMA_DONE )
    {
        // clear interrupt
        mmr_write16(uart_dma_cfg->rx_dma_irq_status, DMA_DONE);

        /* Post the event flag */
        xTaskNotifyFromISR(xUartTaskHandle, RX_BIT, eSetBits, &xHigherPriorityTaskWoken);

        //switch in the higher priority task, if higher task is ready
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    return;
}



//*************************************************************************************
//* Function: uart_dma_tx_isr
//* Description: handle the uart DMA TX interruption 
//* Return: NONE
//*************************************************************************************
static void uart_dma_tx_isr(void *usr_data)
{
    UART_DMA_CONFIG *uart_dmd_cfg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    uart_dmd_cfg = (UART_DMA_CONFIG *)usr_data;

    if( mmr_read16(uart_dmd_cfg->tx_dma_irq_status) & DMA_DONE )
    {
        // clear interrupt
        mmr_write16(uart_dmd_cfg->tx_dma_irq_status, DMA_DONE);

        /* Post the event flag */
        xTaskNotifyFromISR(xUartTaskHandle, TX_BIT, eSetBits, &xHigherPriorityTaskWoken);

        //switch in the higher priority task, if higher task is ready
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return;
}



//******************************************************************************
//* Function:    uart_err_isr
//* Description: Handles UART Errors
//* Params:      usr_data - Pointer to the application configuration passed
//* Return: NONE
//******************************************************************************/
static void uart_err_isr(void *usr_data)
{
    UART_DMA_CONTEXT *uart_dma_ocb;
    UART_PORT_CONFIG *uart_port;
    uint16_t lsr;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* BF54x has seperate error bits for each peripheral,
    hence we can handle it normally here */
    uart_dma_ocb = (UART_DMA_CONTEXT *)usr_data;
    uart_port    = uart_dma_ocb->port;

    //Clear the interrupt status. LSR register (Write-1-To-Clear)
    lsr = mmr_read16(uart_port->lsr);

    if( UART_DMA_READ == uart_dma_ocb->rw )
    {
        /* Post the event flag */
        xTaskNotifyFromISR(xUartTaskHandle, ERR_BIT, eSetBits, &xHigherPriorityTaskWoken);
    }
}



#define UART_DMA_TX_INT     (1<<15)
#define UART_DMA_RX_INT     (1<<14)
#define UART_DMA_ERR_INT    (1<<6)

EX_INTERRUPT_HANDLER(uart_dma_isr)
{
    uint32_t ipend;
    UART_DMA_CONFIG *dma;
    uint32_t sic_isr;
    
    ipend = mmr_read32(pIPEND);
    dma = (UART_DMA_CONFIG *)uart_dma_ctx.dma;
    sic_isr = mmr_read32(dma->sic_isr);
    
    if( sic_isr & UART_DMA_TX_INT )
    {
        uart_dma_tx_isr(dma);
    }
    else if( sic_isr & UART_DMA_RX_INT )
    {
        uart_dma_rx_isr(dma);
    }
    else if( sic_isr & UART_DMA_ERR_INT )
    {
        uart_err_isr(dma);
    }
}



//******************************************************************************
//* Function:     uart_dma_transfer
//* Description:  Performs a DMA transmit or DMA receive
//* Params:       ocb - Pointer to currently active UART DMA OCB
//*               buffer - Pointer to buffer to transmit or receive
//*               len - Length of buffer in bytes
//* Return:     Zero     = OK
//*             Negative = Error
//******************************************************************************/
static int32_t uart_dma_transfer(UART_DMA_CONTEXT *uart_dma_ocb,INT8 *buffer, int32_t len)
{
    uint8_t err;
    UART_DMA_CONFIG *uart_dma_cfg;
    const UART_PORT_CONFIG *port_registers;
    uint16_t dma_config;
    BaseType_t return_code;
    uint32_t ulNotifiedValue;

    //Initialize good return code to start with
    return_code = UART_DMA_OK;

    /* Create some useful shortcut pointers */
    uart_dma_cfg = uart_dma_ocb->dma;
    port_registers = uart_dma_ocb->port;

    /* Do a minimal check of the configuration */
    if (uart_dma_ocb->port_enabled != UART_DMA_ENABLED)
    {
        return ((INT32)UART_DMA_ERR_NOT_READY);
    }

    //Set default value for DMA config registers
    dma_config = (WDSIZE_8 | DI_EN | DMAEN);

    /* Configure the UART and DMA for write or read */
    if( UART_DMA_READ == uart_dma_ocb->rw )
    {
        /* Lock the RX DMA Channel  */
        xSemaphoreTake(uart_dma_ocb->read_lock, portMAX_DELAY);

        mmr_write32( uart_dma_cfg->rx_dma_start_addr, (U32)buffer );
        mmr_write16( uart_dma_cfg->rx_dma_x_count, len );
        mmr_write16( uart_dma_cfg->rx_dma_x_modify, sizeof(U8) );

        /* Clear DMA_ERR in DMA_IRQ_STATUS */
        mmr_write16( uart_dma_cfg->rx_dma_irq_status, DMA_ERR );

        //Clear out any Errors stored in the OCB
        mmr_write16(port_registers->lsr, 0x0000);

        /* Kick off the DMA and UART port */
        mmr_write16( uart_dma_cfg->rx_dma_config, (dma_config | WNR) );
        mmr_write16( port_registers->ier, mmr_read16( port_registers->ier ) | (ERBFI | ELSI) );

        /* Wait for DMA to complete */
        return_code = xTaskNotifyWait(pdFAIL,
                                      ULONG_MAX,
                                      &ulNotifiedValue,
                                      portMAX_DELAY
                                      );
        if(return_code != pdPASS)
            return_code = UART_DMA_ERR_NOT_READY;
    }
    else if( UART_DMA_WRITE == uart_dma_ocb->rw )
    {
        /* Lock the TX DMA Channel  */
        xSemaphoreTake(uart_dma_ocb->write_lock, portMAX_DELAY);

        mmr_write32( uart_dma_cfg->tx_dma_start_addr, (U32)buffer );
        mmr_write16( uart_dma_cfg->tx_dma_x_count, len );
        mmr_write16( uart_dma_cfg->tx_dma_x_modify, sizeof(U8) );

        /* Clear DMA_ERR in DMA_IRQ_STATUS */
        mmr_write16( uart_dma_cfg->tx_dma_irq_status, DMA_ERR );

        /* Kick off the DMA and UART port */
        mmr_write16( uart_dma_cfg->tx_dma_config, dma_config );
        mmr_write16( port_registers->ier, mmr_read16( port_registers->ier) | ETBEI );

        /* Wait for DMA to complete */
        return_code = xTaskNotifyWait(pdFAIL,
                                      ULONG_MAX,
                                      &ulNotifiedValue,
                                      portMAX_DELAY
                                      );
        if(return_code != pdPASS)
            return_code = UART_DMA_ERR_NOT_READY;
    }
    else
    {
        //Do Nothing...
    }


    if (uart_dma_ocb->rw == UART_DMA_WRITE)
    {
        /* Unlock the TX DMA channel */
        xSemaphoreGive(uart_dma_ocb->write_lock);
    }
    else if ( uart_dma_ocb->rw == UART_DMA_READ )
    {
        //Handle if there are any receive errors
        if ( mmr_read16(port_registers->lsr) & UART_DMA_LSR_ERROR_MASK )
        {
            if ( mmr_read16(port_registers->lsr) & OE )
            {
                return_code = UART_DMA_ERR_OVERRUN_ERROR;
            }
            else if ( mmr_read16(port_registers->lsr) & PE )
            {
                return_code = UART_DMA_ERR_PARITY_ERROR;
            }
            else if ( mmr_read16(port_registers->lsr) & FE )
            {
                return_code = UART_DMA_ERR_FRAMING_ERROR;
            }
            else if ( mmr_read16(port_registers->lsr) & BI )
            {
                return_code = UART_DMA_ERR_BREAK_INTERRUPT;
            }
            else
            {
                // do nothing
            }
        }

        /* Unlock the RX DMA channel */
        xSemaphoreGive( uart_dma_ocb->read_lock );
    }
    else
    {
        //Do Nothing...
    }

    //Reset the RW flag back to NONE
    uart_dma_ocb->rw = UART_DMA_NONE;

    return (return_code);
}



//******************************************************************************
//* Function:     uart_dma_read
//* Description:  Performs a UART DMA receive
//* Params:       ocb - Pointer to currently active driver services OCB
//*               buffer - Pointer to buffer to transmit
//*               len - Length of buffer in bytes
//* Return:       Zero     = OK
//*               Negative = Error
//******************************************************************************
BaseType_t uart_dma_read(void *ocb, void *buffer, uint32_t len)
{
    BaseType_t error;
    UART_DMA_CONTEXT *uart_dma_ocb;

    uart_dma_ocb = (UART_DMA_CONTEXT *)(ocb);

    uart_dma_ocb->rw = UART_DMA_READ;
    error = uart_dma_transfer(uart_dma_ocb, buffer, len);
    return error;
}



//******************************************************************************
//* Function:     uart_dma_write
//* Description:  Performs a UART DMA transmit
//* Params:       ocb - Pointer to currently active driver services OCB
//*               buffer - Pointer to buffer to transmit
//*               len - Length of buffer in bytes
//* Return:       Zero     = OK
//*               Negative = Error
//******************************************************************************
BaseType_t uart_dma_write(void *ocb, void *buffer, uint32_t len)
{
    BaseType_t error;

    UART_DMA_CONTEXT *uart_dma_ocb;

    uart_dma_ocb = (UART_DMA_CONTEXT *)(ocb);

    uart_dma_ocb->rw = UART_DMA_WRITE;
    error = uart_dma_transfer(uart_dma_ocb, buffer, len);

    return error;
}





//******************************************************************************
//* Function:     uart_dma_init
//* Description:  Performs a UART DMA transmit
//* Params:       ocb - Pointer to currently active driver services OCB
//*               buffer - Pointer to buffer to transmit
//*               len - Length of buffer in bytes
//* Return:       Zero     = OK
//*               Negative = Error
//******************************************************************************
void* uart_dma_init( void )
{
    //mapping the registers set
    uart_dma_ctx.port = (UART_PORT_CONFIG *)&uart_port_cfg;
    uart_dma_ctx.dma  = (UART_DMA_CONFIG *)&uart_dma_cfg;

    mmr_write16( uart_dma_cfg.rx_pmap, PMAP_UARTRX );
    mmr_write16( uart_dma_cfg.tx_pmap, PMAP_UARTTX );
    register_handler_ex(ik_ivg10, (ex_handler_fn)uart_dma_isr, EX_INT_ENABLE);
    mmr_write32( uart_dma_cfg.sic_imask, 
                            mmr_read32(uart_dma_cfg.sic_imask) | (UART_ERR_IRQ | DMA6_IRQ | DMA7_IRQ) );
    mmr_write32( uart_dma_cfg.sic_isr, 
                            mmr_read32( uart_dma_cfg.sic_isr) | (UART_ERR_IRQ | DMA6_IRQ | DMA7_IRQ) );
    
    uart_dma_ctx.write_lock = xSemaphoreCreateMutex();
    if( uart_dma_ctx.write_lock == NULL )
        return NULL;

    uart_dma_ctx.read_lock = xSemaphoreCreateMutex();
    if( uart_dma_ctx.read_lock == NULL )
        return NULL;
    
    uart_dma_enable( &uart_dma_ctx );
    
    return ((void *)&uart_dma_ctx);
}


BaseType_t uart_dma_enable(UART_DMA_CONTEXT *uart_dma_ocb)
{
    const UART_PORT_CONFIG *port_registers;
    BaseType_t err;
    int32_t retval = UART_DMA_OK;

    /* Create some useful shortcut pointers */
    port_registers = uart_dma_ocb->port;

    /* Lock the UART port  */
    //OSSemPend(uart_config->lock, 0, &err);

    //If UART port not already enabled, then enable here
    if (uart_dma_ocb->port_enabled == UART_DMA_DISABLED)
    {
        //Clear interrupts in IER
        mmr_write16( port_registers->ier, UART_IER_CLEAR_ALL_INT );

        //Set the UART port registers
        mmr_write16( port_registers->lcr, DLAB );
        mmr_write16( port_registers->dll,  UART_BAUD115200 & 0xff );
        mmr_write16( port_registers->dlh, (UART_BAUD115200 >> 8) & 0xff );

        //uart control register: 8-bit, 1-stop, no parity
        mmr_write16( port_registers->lcr, WLS(8) );

         //enable the port
        mmr_write16( port_registers->gctl, UCEN );
        
        //Set the Modem control reg with configured parameters
        mmr_write16( port_registers->mcr, 0 );

        uart_dma_ocb->port_enabled = UART_DMA_ENABLED;
    }
    else
    {
        /* If UART Port already enabled by some other OCB,
        cannot enable it again, return error  */
        retval = UART_DMA_ERR_INVALID_STATE;
    }

    /* Unlock the UART port */
    //OSSemPost(uart_config->lock);

    return retval;
}


