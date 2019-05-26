//*************************************************************************************
//File Name: spi_api.h
//Notes:
//Author: wu, zhigang
//Date:   May-19-2019
//*************************************************************************************
//*************************************************************************************
#ifndef __SPI_API_H__
#define __SPI_API_H__

#define SPI		(0xB3)

#define SPI0			(0x0)
#define SPI_PORT_NUM	(0x1)

enum {
	SPI_CFG_FLG = 0x1,
	SPI_CFG_BAUDRATE,
	SPI_CFG_CTRL,
	SPI_CFG_PMAP,
	SPI_DMA_CFG_SADDR,
	SPI_DMA_CFG_XCOUNT,
	SPI_DMA_CFG_WDSIZE,
	SPI_DMA_CFG_XMODIFY,
	SPI_DIRT,
	SPI_MODE,
	SPI_CFG_END = 0xFF,
};


struct spi_cfg {
	uint16_t cmd;
	uint32_t val;
};
extern const struct spi_cfg spi_read_cfg[];
extern const struct spi_cfg spi_write_cfg[];

struct spi_dev;
struct spi_ops {
    void (*open)(struct spi_dev *dev, uint8_t port);
    void (*close)(struct spi_dev *dev, uint8_t port);
    int32_t (*read)(struct spi_dev *dev, uint8_t *buf, uint32_t len);
    int32_t (*write)(struct spi_dev *dev, uint8_t *buf, uint32_t len);
    void (*ioctl)(struct spi_dev *dev, uint16_t cmd, uint32_t val);
};

struct spi_dev
{
	uint8_t type;

	const struct spi_ops *ops;
	void *ctx;

	SemaphoreHandle_t xSem;
};

extern struct spi_dev *spi_get(uint8_t type, uint8_t port);

extern void sbus_open(struct spi_dev *dev, uint8_t port);
extern void sbus_close(struct spi_dev *dev, uint8_t port);
extern int32_t sbus_read(struct spi_dev *dev, uint8_t *buf, uint32_t nLen);
extern int32_t sbus_write(struct spi_dev *dev, uint8_t *buf, uint32_t nLen);
extern void sbus_ioctl(struct spi_dev *dev, uint16_t cmd, uint32_t val);

#endif //__SPI_API_H__
