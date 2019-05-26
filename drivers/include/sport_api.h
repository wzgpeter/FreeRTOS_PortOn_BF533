//*************************************************************************************
//File Name: sport_api.h
//Notes:
//Author: wu, zhigang
//Date:   May-19-2019
//*************************************************************************************
//*************************************************************************************
#ifndef __SPORT_API_H__
#define __SPORT_API_H__

#define SPORT 	(0xA3)

#define SPORT0                      (0)
#define SPORT1                      (1)
#define SPORT_PORT_NUM				(2)

enum {
	SPORT_INIT = 0xB1,
	SPORT_DIRT,
	SPORT_MODE,
	SPORT_BAUDRATE,

	SPORT_CFG_PORT_RCR1,
	SPORT_CFG_PORT_RCR2,
	SPORT_CFG_PORT_TCR1,
	SPORT_CFG_PORT_TCR2,
	SPORT_CFG_PORT_MRCS0,
	SPORT_CFG_PORT_MTCS0,
	SPORT_CFG_PORT_MCMC1,
	SPORT_CFG_PORT_MCMC2,

	SPORT_CFG_DMA_RX_SADDR,
	SPORT_CFG_DMA_RX_XCOUNT,
	SPORT_CFG_DMA_RX_XMODIFY,
	SPORT_CFG_DMA_RX_YCOUNT,
	SPORT_CFG_DMA_RX_YMODIFY,
	SPORT_CFG_DMA_RX_CONFIG,
	SPORT_CFG_DMA_RX_PMAP,
	SPORT_CFG_DMA_TX_SADDR,
	SPORT_CFG_DMA_TX_XCOUNT,
	SPORT_CFG_DMA_TX_XMODIFY,
	SPORT_CFG_DMA_TX_YCOUNT,
	SPORT_CFG_DMA_TX_YMODIFY,
	SPORT_CFG_DMA_TX_CONFIG,
	SPORT_CFG_DMA_TX_PMAP,
	SPORT_CFG_DMA_SIC_IMASK,

	SPORT_CFG_END,
};


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


struct sport_dev;
struct sport_ops {
    void (*open)(struct sport_dev *dev, uint8_t port);
    void (*close)(struct sport_dev *dev, uint8_t port);
    void (*read)(struct sport_dev *dev, uint8_t *buf, uint32_t len);
    void (*write)(struct sport_dev *dev, uint8_t *buf, uint32_t len);
    void (*ioctl)(struct sport_dev *dev, uint8_t cmd, uint32_t val);
};


struct sport_dev
{
	uint8_t type;

	const struct sport_ops *ops;
	void *ctx;
};

struct sport_cfg
{
	uint16_t cmd;
	uint32_t val;
};


extern int32_t AudioRxBuffer[RX_BUFFER_NUM][RX_WORDS_PER_BLOCK];
extern int32_t AudioTxBuffer[TX_BUFFER_NUM][TX_WORDS_PER_BLOCK];

extern struct reg_interrupt_handler sport0_dma1_rx_int_cfg;
extern struct reg_interrupt_handler sport0_dma2_tx_int_cfg;

extern const struct sport_cfg sport0_rx_cfg[];
extern const struct sport_cfg sport0_tx_cfg[];

extern struct sport_dev * sport_get(uint8_t type, uint32_t index);
extern void sport_proc(struct sport_dev *dev);

extern void aud_open(struct sport_dev *dev, uint8_t port);
extern void aud_close(struct sport_dev *dev, uint8_t port);
extern void aud_read(struct sport_dev *dev, uint8_t *buf, uint32_t nLen);
extern void aud_write(struct sport_dev *dev, uint8_t *buf, uint32_t nLen);
extern void aud_ioctl(struct sport_dev *dev, uint8_t cmd, uint32_t val);

#endif //__SPORT_API_H__
