//*************************************************************************************
//*************************************************************************************
//File Name: spi_dma.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-19-2019
//*************************************************************************************
//*************************************************************************************
#include <defBF533.h>
#include <defbf533.h>
#include <string.h>
#include <math.h>
#include "sys_cfg.h"

#include "clocks.h"
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "spi_dma.h"
#include "ad1836.h"
#include "spi_api.h"


void spi_enable(struct spi_ctx *pSpiCtx)
{
    // Enable DMA
    mmr_write16(pSpiCtx->reg->config, (mmr_read16(pSpiCtx->reg->config) | DMAEN) );

    // Enable SPI
    mmr_write16(pSpiCtx->reg->spi_ctl, (mmr_read16(pSpiCtx->reg->spi_ctl) | SPE) );
}

static void spi_disable(struct spi_ctx *pSpiCtx)
{
    // Disable DMA
    mmr_write16(pSpiCtx->reg->config, ( mmr_read16(pSpiCtx->reg->config) | (~DMAEN) ) );

    // Disable SPI
    mmr_write16(pSpiCtx->reg->spi_ctl, (0x0000) );
}

//==========================================================================================

EX_INTERRUPT_HANDLER(spi_dma_isr)
{
    uint32_t ipend;
    uint32_t sic_isr;

    ipend = mmr_read32(pIPEND);
}


static void spi_ioctl(struct spi_dev *dev, uint16_t cmd, uint32_t val)
{
	struct spi_ctx *pSpiCtx = (struct spi_ctx *)dev->ctx;

	switch(cmd)
	{
	case SPI_CFG_FLG:
		mmr_write16(pSpiCtx->reg->spi_flg, (uint16_t)val);
		break;
	case SPI_CFG_BAUDRATE:
		mmr_write16(pSpiCtx->reg->spi_baud, (uint16_t)val);
		break;
	case SPI_CFG_CTRL:
		mmr_write16(pSpiCtx->reg->spi_ctl, (uint16_t)val);
		break;
	case SPI_CFG_PMAP:
		mmr_write16(pSpiCtx->reg->dma_pmap, (uint16_t)val);
		break;
	case SPI_DMA_CFG_SADDR:
		mmr_write32(pSpiCtx->reg->start_addr, (uint32_t)val);
		break;
	case SPI_DMA_CFG_XCOUNT:
		mmr_write16(pSpiCtx->reg->x_count, (uint16_t)val);
		break;
	case SPI_DMA_CFG_WDSIZE:
		mmr_write16(pSpiCtx->reg->config, (uint16_t)val);
		break;
	case SPI_DMA_CFG_XMODIFY:
		mmr_write16(pSpiCtx->reg->x_modify, (uint16_t)val);
		break;
	default:
		break;
	}
}


//confirm which SPI port will be opened
static void spi_open(struct spi_dev *dev, uint8_t port)
{
	struct spi_ctx *pSpiCtx = (struct spi_ctx *)dev;

	dev->xSem = xSemaphoreCreateBinary();
	
    //register the spi rx/tx interrupt
//	register_handler_ex(ik_ivg5, (ex_handler_fn)spi_dma_isr, EX_INT_ENABLE);    //tx int
//	register_handler_ex(ik_ivg6, (ex_handler_fn)spi_dma_isr, EX_INT_ENABLE);    //rx int

	//spi_enable(pSpiCtx);
}

//confirm which SPI port will be closed
static void spi_close(struct spi_dev *dev, uint8_t port)
{
	struct spi_ctx *pSpiCtx = (struct spi_ctx *)dev;

	spi_disable(pSpiCtx);
}


//do the spi read operation
static int32_t spi_read(struct spi_dev *dev, uint8_t *buf, uint32_t len)
{
	if (xSemaphoreTake(dev->xSem, 0) == pdTRUE)
	{
	    //Config Start Address
		spi_ioctl(dev, SPI_DMA_CFG_SADDR, (uint32_t)buf);

	    //Config X Count
		spi_ioctl(dev, SPI_DMA_CFG_XCOUNT, len);

		//Start DMA RX
	    spi_enable((struct spi_ctx *)dev->ctx);
		
		xSemaphoreGive(dev->xSem);

		return 0;
	}
	else
		return -1;
}

//do the spi write operation
static int32_t spi_write(struct spi_dev *dev, uint8_t *buf, uint32_t len)
{
	if (xSemaphoreTake(dev->xSem, 0) == pdTRUE)
	{
	    //Config TX Address
		spi_ioctl(dev, SPI_DMA_CFG_SADDR, (uint32_t)buf);

	    //Config X Count
		spi_ioctl(dev, SPI_DMA_CFG_XCOUNT, len);

		//Start DMA TX
	    spi_enable((struct spi_ctx *)dev->ctx);

		xSemaphoreGive(dev->xSem);
		
		return 0;
	}
	else
		return -1;
}

const struct spi_ops spi_dev_api = {
    .open = spi_open,
    .close = spi_close,
    .read = spi_read,
    .write = spi_write,
    .ioctl = spi_ioctl,
};

