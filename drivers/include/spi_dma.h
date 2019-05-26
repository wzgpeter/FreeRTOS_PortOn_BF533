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

struct spi_dma_reg
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
};


struct spi_ctx
{
    struct spi_dma_reg *reg;
};

extern const struct spi_ops spi_dev_api;

#endif //__SPI_DMA_H__
