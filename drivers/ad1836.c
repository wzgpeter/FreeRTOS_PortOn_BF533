//*************************************************************************************
//*************************************************************************************
//File Name: ad1836.c
//Notes:
//Author: wu, zhigang
//Date:   Jul-15-2017
//*************************************************************************************
//*************************************************************************************
#include <stdlib.h>
#include <string.h>
#include <defBF533.h>
#include "stdint.h"
#include "initialise.h"
#include "clocks.h"
#include "freeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ad1836.h"
#include "spi_api.h"


// names for codec registers, used for iCodec1836TxRegs[]
#define DAC_CONTROL_1		0x0000
#define DAC_CONTROL_2		0x1000
#define DAC_VOLUME_0		0x2000
#define DAC_VOLUME_1		0x3000
#define DAC_VOLUME_2		0x4000
#define DAC_VOLUME_3		0x5000
#define DAC_VOLUME_4		0x6000
#define DAC_VOLUME_5		0x7000
#define ADC_0_PEAK_LEVEL	0x8000
#define ADC_1_PEAK_LEVEL	0x9000
#define ADC_2_PEAK_LEVEL	0xA000
#define ADC_3_PEAK_LEVEL	0xB000
#define ADC_CONTROL_1		0xC000
#define ADC_CONTROL_2		0xD000
#define ADC_CONTROL_3		0xE000


short sCodecAD1836TxRegs[CODEC_AD1836_REGS_LENGTH] = 
{
    DAC_CONTROL_1   | 0x0000,
    DAC_CONTROL_2   | 0x003C,
    DAC_VOLUME_0    | 0x03FF,
    DAC_VOLUME_1    | 0x03FF,
    DAC_VOLUME_2    | 0x03FF,
    DAC_VOLUME_3    | 0x03FF,
    DAC_VOLUME_4    | 0x03FF,
    DAC_VOLUME_5    | 0x03FF,
    ADC_CONTROL_1   | 0x0000,
    ADC_CONTROL_2   | 0x0000,
    ADC_CONTROL_3   | 0x0000,
};



void resetAD1836(void)
{
    uint32_t i;
    
    //Write to Port A to reset AD1836
    *pFlashA_PortA_Data = 0x00;

    //Write to Port A to enable AD1836
    *pFlashA_PortA_Data = 0x01;

    //Wait for AD1836 recover from reset
    for(i=0; i<0xF000; i++)
        asm("nop;");
}


void enableAD1836(void)
{
	struct spi_dev *dev;
    uint32_t i;

	// open the SPI port
	dev = spi_get(SPI, SPI0);
	sbus_open(dev, SPI0);

	// Config the SPI as write mode
	i = 0;
	while(spi_write_cfg[i].cmd != SPI_CFG_END)
	{
		sbus_ioctl(dev, spi_write_cfg[i].cmd, spi_write_cfg[i].val);
		i++;
	}

	// config AD by SPI bus
	sbus_write(dev, (uint8_t *)sCodecAD1836TxRegs, CODEC_AD1836_REGS_LENGTH);

    //Wait for transmitting finished
    for(i=0; i<0xF000; i++) 
		asm("nop;");
}


void Init_AD1836(void)
{
    resetAD1836();

    enableAD1836();
}




