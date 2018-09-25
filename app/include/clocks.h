#ifndef  __CLOCKS_H__
#define __CLOCKS_H__


#define VCO_MULTIPLIER  40
#define CCLK_DIVIDER     1
#define SCLK_DIVIDER     4
 
// compiler only stuff below here
//#ifdef __ECC__

/*******************************************************************************
 *  INCLUDED HEADERS
 ******************************************************************************/
#include <sys\exception.h>
#include <cdefbf533.h>
#include <ccblkfn.h>
#include <sysreg.h>
#include <stdio.h>
#include "ctypes.h"

// Processor clock defines core
#define CLK_CLKIN              (27000000)  //(12288000)      // oscillator input
#define CLK_VCO                (CLK_CLKIN * VCO_MULTIPLIER)   // 432 Mhz programmed via PLL_CTL
#define CLK_CCLK               (CLK_VCO / CCLK_DIVIDER)       // 432 Mhz programmed via PLL_DIV
#define CLK_SCLK               (CLK_VCO / SCLK_DIVIDER)       // 144 Mhz programmed via PLL_DIV

#define OSCLK                  (1000)                         // OS timer interval in hz
#define OSCLK_PERIOD           (CLK_SCLK/OSCLK)               // used for timer register
#define OSCLK_TCOUNT           ((CLK_VCO)/(OSCLK))/(OSCLK_TSCALE)
//  SCLK cannot exceen 135Mhz

// UART baud rate defines
#define UART_BAUD9600          ((CLK_SCLK / 9600) / 16)
#define UART_BAUD19200         ((CLK_SCLK / 19200) / 16)
#define UART_BAUD38400         ((CLK_SCLK / 38400) / 16)
#define UART_BAUD57600         ((CLK_SCLK / 57600) / 16)
#define UART_BAUD115200        ((CLK_SCLK / 115200) / 16 / 4)

// SPI baud rate defines
#define SCK_DAC                1000000                        // SCK freq. for AD1836
#define SPI_BAUD_DAC           ((CLK_SCLK/SCK_DAC)/2)       // SPI_BAUD reg value
#define SCK_AD1940             90000                        // SCK freq. for AD1836
#define SPI_BAUD_AD1940        ((CLK_SCLK/SCK_AD1940)/2)    // SPI_BAUD reg value
#define SCK_EEPROM             90000                        // SCK freq. for AD1836
#define SPI_BAUD_EEPROM        ((CLK_SCLK/SCK_EEPROM)/2)    // SPI_BAUD reg value
#define SCK_TEMP_SENS          90000                        // SCK freq. for AD1836
#define SPI_BAUD_TEMP_SENS     ((CLK_SCLK/SCK_TEMP_SENS)/2) // SPI_BAUD reg value

// SPORT clock defines
#define AUDIO_CLK              (48000)                        // defines frame rate of audio transfers
                                                              
#define SPORT0_FSMULT          (256)                          // bits per frame sync
#define SPORT0_RCLK            (AUDIO_CLK * SPORT0_FSMULT)    // bit clock rate
#define SPORT0_CLKDIV          (((CLK_SCLK/SPORT0_RCLK)/2)-1)       // clock divider register 
#define SPORT0_FSDIV           (SPORT0_FSMULT-1)                  // frame sync divider register
// for now SPORT1 operates at same rate as SPORT0
#define SPORT1_FSMULT          SPORT0_FSMULT                          
#define SPORT1_TCLK            SPORT0_RCLK
#define SPORT1_CLKDIV          SPORT0_CLKDIV
#define SPORT1_FSDIV           SPORT0_FSDIV


// Periodic timer definitions
#define PERIODIC_TIMER_FREQ    1000                           // frequency of periodic timer 
#define ALT_TIMER_FREQ         10000                          // frequency of alt timer 
                                                              // interrupt in Hz
#define TICS_PER_SEC           PERIODIC_TIMER_FREQ
#define TICS_PER_OSTIC         (OSCLK / PERIODIC_TIMER_FREQ)
#define TICS_PER_MSEC          PERIODIC_TIMER_FREQ/1000
#define MSEC_PER_TIC           (1)

#define MSEC(x)                (TICS_PER_MSEC * (U32)(x))     

//#endif /* __ECC__ */

#endif /* __CLOCKS_H__ */
