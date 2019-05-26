//*************************************************************************************
//File Name: spi_api.c
//Notes:
//Author: wu, zhigang
//Date:   May-19-2019
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

#include "spi_dma.h"
#include "spi_api.h"
#include "ad1836.h"

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


struct spi_dma_reg spi_dma_cfg[SPI_PORT_NUM] = { SPI_REG_INIT };


static struct spi_ctx context[SPI_PORT_NUM] = {	
	{
		.reg = &spi_dma_cfg[SPI0],
	}, 
};


static struct spi_dev spi[SPI_PORT_NUM] = {
	{
		.type = SPI,
		.ops = &spi_dev_api,
		.ctx = &context[SPI0],
	}
};

struct spi_dev *spi_get(uint8_t type, uint8_t port)
{
	if (type == SPI && port <SPI_PORT_NUM)
		return &spi[port];

	return NULL;
}

const struct spi_cfg spi_read_cfg[] = 
{
	{SPI_CFG_FLG, 			FLS4},							//Enable PF4
	{SPI_CFG_BAUDRATE, 		16},							//Set Baud Rate SCK = HCLK/(2*SPIBAUD) = 2MHz
	{SPI_CFG_CTRL, 			TDBR_DMA | SIZE | MSTR},		//SPI DMA Write; 16-bit data; MSB first; SPI Master
	{SPI_CFG_PMAP,			PMAP_SPI},						//DMA5 for SPI
	{SPI_DMA_CFG_SADDR, 	(uint32_t)sCodecAD1836TxRegs},	// Start Address
	{SPI_DMA_CFG_XCOUNT,	CODEC_AD1836_REGS_LENGTH},		// X Count
	{SPI_DMA_CFG_WDSIZE, 	WDSIZE_16},						// 16-bit
	{SPI_DMA_CFG_XMODIFY,	2},								// X Modify: 16-Bit
	{SPI_CFG_END, 			NULL},							// Indicate END of configuration
};

const struct spi_cfg spi_write_cfg[] = 
{
	{SPI_CFG_FLG, 			FLS4},							//Enable PF4
	{SPI_CFG_BAUDRATE, 		16},							//Set Baud Rate SCK = HCLK/(2*SPIBAUD) = 2MHz
	{SPI_CFG_CTRL, 			TDBR_DMA | SIZE | MSTR},		//SPI DMA Write; 16-bit data; MSB first; SPI Master
	{SPI_CFG_PMAP,			PMAP_SPI},						//DMA5 for SPI
	{SPI_DMA_CFG_SADDR, 	(uint32_t)sCodecAD1836TxRegs},	// Start Address
	{SPI_DMA_CFG_XCOUNT,	CODEC_AD1836_REGS_LENGTH},		// X Count
	{SPI_DMA_CFG_WDSIZE, 	WDSIZE_16},						// 16-bit
	{SPI_DMA_CFG_XMODIFY,	2},								// X Modify: 16-Bit
	{SPI_CFG_END, 			NULL},							// Indicate END of configuration
};


void sbus_open(struct spi_dev *dev, uint8_t port)
{
	return dev->ops->open(dev, port);
}

void sbus_close(struct spi_dev *dev, uint8_t port)
{
	return dev->ops->close(dev, port);
}

int32_t sbus_read(struct spi_dev *dev, uint8_t *buf, uint32_t nLen)
{
	return dev->ops->read(dev, buf, nLen);
}

int32_t sbus_write(struct spi_dev *dev, uint8_t *buf, uint32_t nLen)
{
	return dev->ops->write(dev, buf, nLen);
}

void sbus_ioctl(struct spi_dev *dev, uint16_t cmd, uint32_t val)
{
	return dev->ops->ioctl(dev, cmd, val);
}

