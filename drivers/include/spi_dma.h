//*************************************************************************************
//*************************************************************************************
//File Name: spi.h
//Notes:
//Author: wu, zhigang
//Date:   Jul-15-2017
//*************************************************************************************
//*************************************************************************************
#ifndef __SPI_DMA_H__
#define __SPI_DMA_H__

typedef struct _spi_dma_registers
{
    //SPI registers
    volatile uint16_t *spi_flg;
    volatile uint16_t *spi_baud;
    volatile uint16_t *spi_ctl;

    //DMA registers
    volatile uint16_t *dma_pmap;
    volatile uint16_t *config;
    volatile uint32_t *start_addr;
    volatile uint16_t *x_count;
    volatile  int16_t *x_modify;
}SPI_DMA_REG;

#define SPI_REG_INIT     \
{                        \
    pSPI_FLG,            \
    pSPI_BAUD,           \
    pSPI_CTL,            \
                         \
    pDMA5_PERIPHERAL_MAP,\
    pDMA5_CONFIG,        \
    (uint32_t *)pDMA5_START_ADDR,    \
    pDMA5_X_COUNT,       \
    pDMA5_X_MODIFY       \
}


typedef struct _spi_dma_context
{
    SPI_DMA_REG *reg;
}SPI_DMA_CONTEXT;


struct spi {
    void (*open)(SPI_DMA_CONTEXT *pSpiCtx);
    void (*close)(SPI_DMA_CONTEXT *pSpiCtx);
    void (*read)(SPI_DMA_CONTEXT *pSpiCtx, void *buf, int32_t len);
    void (*write)(SPI_DMA_CONTEXT *pSpiCtx, void *buf, int32_t len);
    void (*ioctl)(SPI_DMA_CONTEXT *pSpiCtx);
};

void spi_init(SPI_DMA_CONTEXT *pSpiCtx);
void spi_enable(SPI_DMA_CONTEXT *pSpiCtx);
void spi_disable(SPI_DMA_CONTEXT *pSpiCtx);



#endif //__SPI_DMA_H__

