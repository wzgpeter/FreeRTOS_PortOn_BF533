//*************************************************************************************
//*************************************************************************************
//File Name: spi_dma.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-15-2017
//*************************************************************************************
//*************************************************************************************

#include <defbf533.h>
#include <string.h>
#include <math.h>
#include "clocks.h"
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "spi_dma.h"
#include "ad1836.h"




SPI_DMA_REG spi_dma_cfg = SPI_REG_INIT;


static void spi_init(SPI_DMA_CONTEXT *pSpiCtx)
{
    pSpiCtx->reg = &spi_dma_cfg;

    //=============================================
    // Config SPI PORT register
    
    //Enable PF4
    mmr_write16(pSpiCtx->reg->spi_flg, FLS4);

    //Set Baud Rate SCK = HCLK/(2*SPIBAUD) = 2MHz
    mmr_write16(pSpiCtx->reg->spi_baud, 16);

    //SPI DMA Write; 16-bit data; MSB first; SPI Master
    mmr_write16(pSpiCtx->reg->spi_ctl, TDBR_DMA | SIZE | MSTR);

    //DMA5 for SPI
    mmr_write16(pSpiCtx->reg->dma_pmap, PMAP_SPI);

    //=============================================
    // Config SPI DMA register

    // 16-bit
    mmr_write16(pSpiCtx->reg->config, WDSIZE_16);

    // Start Address
    mmr_write32(pSpiCtx->reg->start_addr, (uint32_t)sCodecAD1836TxRegs);

    // X Count
    mmr_write16(pSpiCtx->reg->x_count, CODEC_AD1836_REGS_LENGTH);

    // X Modify: 16-Bit
    mmr_write16(pSpiCtx->reg->x_modify, 2);
}


static void spi_enable(SPI_DMA_CONTEXT *pSpiCtx)
{
    // Enable DMA
    mmr_write16(pSpiCtx->reg->config, (mmr_read16(pSpiCtx->reg->config) | DMAEN) );

    // Enable SPI
    mmr_write16(pSpiCtx->reg->spi_ctl, (mmr_read16(pSpiCtx->reg->spi_ctl) | SPE) );
}

static void spi_disable(SPI_DMA_CONTEXT *pSpiCtx)
{
    // Enable DMA
    mmr_write16(pSpiCtx->reg->config, ( mmr_read16(pSpiCtx->reg->config) | (~DMAEN) ) );

    // Enable SPI
    mmr_write16(pSpiCtx->reg->spi_ctl, (0x0000) );
}

//==========================================================================================

static void spi_open(SPI_DMA_CONTEXT *pSpiCtx)
{
    pSpiCtx->reg = &spi_dma_cfg;

    //=============================================
    // Config SPI PORT register
    
    //Enable PF4
    mmr_write16(pSpiCtx->reg->spi_flg, FLS4);

    //Set Baud Rate SCK = HCLK/(2*SPIBAUD) = 2MHz
    mmr_write16(pSpiCtx->reg->spi_baud, 16);

    //SPI DMA Write; 16-bit data; MSB first; SPI Master
    mmr_write16(pSpiCtx->reg->spi_ctl, TDBR_DMA | SIZE | MSTR);

    //DMA5 for SPI
    mmr_write16(pSpiCtx->reg->dma_pmap, PMAP_SPI);

    //=============================================
    // Config SPI DMA register

    // 16-bit
    mmr_write16(pSpiCtx->reg->config, WDSIZE_16);

    // Start Address
    mmr_write32(pSpiCtx->reg->start_addr, (uint32_t)sCodecAD1836TxRegs);

    // X Count
    mmr_write16(pSpiCtx->reg->x_count, CODEC_AD1836_REGS_LENGTH);

    // X Modify: 16-Bit
    mmr_write16(pSpiCtx->reg->x_modify, 2);
}

static void spi_close(SPI_DMA_CONTEXT *pSpiCtx)
{
    // Enable DMA
    mmr_write16(pSpiCtx->reg->config, ( mmr_read16(pSpiCtx->reg->config) | (~DMAEN) ) );

    // Enable SPI
    mmr_write16(pSpiCtx->reg->spi_ctl, (0x0000) );
}

static void spi_read(SPI_DMA_CONTEXT *pSpiCtx, void *buf, int32_t len)
{
}

static void spi_write(SPI_DMA_CONTEXT *pSpiCtx, void *buf, int32_t len)
{
}

static void spi_ioctl(SPI_DMA_CONTEXT *pSpiCtx)
{
}

struct spi spi_ops = {
    .open = spi_open,
    .close = spi_close,
    .read = spi_read,
    .write = spi_write,
    .ioctl = spi_ioctl,
};
