#ifndef __FLASH_DEF_H
#define __FLASH_DEF_H

#if FLASH_SIZE==0x100000
/*
**    +-------------------------------+<- End of flash 0x200FFFFF
**    |                               |
**    |           EQ data             |
**    |        1 64K byte page        |
**    |                               |
**    +-------------------------------+<- 0x200F0000
**    |                               |
**    |           DEC data            |
**    |        1 64K byte page        |
**    |                               |
**    +-------------------------------+<- 0x200E0000
**    |                               |
**    |       Application image       |
**    |        13 64K byte pages      |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20010000
**    |                               |
**    |           Reserved            |
**    |        1 32K byte page        |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20008000
**    |                               |
**    |           Reserved            |
**    |        1 8K byte page         |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20006000
**    |                               |
**    |           DTCP Keys           |
**    |        1 8K byte page         |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20004000
**    |                               |
**    |      Boot Loader in FLASH     |
**    |        1 16K byte page        |
**    |                               |
**    |                               |
**    +-------------------------------+<- Start of FLASH 0x2000000
*****************************************************************************
*/
   // Definitions that could be modified by the makefile for re-defining the 
   // flash range that is part of the checksum calculation
   #ifndef SREC_START_CHECKSUM
   #define SREC_START_CHECKSUM      (0x00010000)
   #endif

   #ifndef SREC_END_CHECKSUM
   #define SREC_END_CHECKSUM        (0x00100000)
   #endif

   // definitions for 1.0M flash
   #define START_OF_FLASH           (0x20000000)      // start of application flash
   #define END_OF_FLASH             (0x200FFFFF)      // last valid flash address
   #define START_OF_CODE            (START_OF_FLASH + SREC_START_CHECKSUM)  // flash space on the blackfin
   #define END_OF_CODE              (START_OF_FLASH + SREC_END_CHECKSUM)    // End of code space -- used for code checksum
   #define START_OF_FLASH_CRC       (START_OF_FLASH)  // start of application flash

   // sizes reported to external programmer
   #define FLASH_PAGE_SIZE          (256)             // bytes per page
#else
/*
**    +-------------------------------+<- End of flash 0x2007FFFF
**    |                               |
**    |           EQ data             |
**    |        1 64K byte page        |
**    |                               |
**    +-------------------------------+<- 0x20070000
**    |                               |
**    |       Application image       |
**    |        6 64K byte pages       |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20010000
**    |                               |
**    |           Reserved            |
**    |        1 32K byte page        |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20008000
**    |                               |
**    |           Reserved            |
**    |        1 8K byte page         |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20006000
**    |                               |
**    |           DTCP Keys           |
**    |        1 8K byte page         |
**    |                               |
**    |                               |
**    +-------------------------------+<- 0x20004000
**    |                               |
**    |      Boot Loader in FLASH     |
**    |        1 16K byte page        |
**    |                               |
**    |                               |
**    +-------------------------------+<- Start of FLASH 0x2000000
*****************************************************************************
*/
   // Definitions that could be modified by the makefile for re-defining the 
   // flash range that is part of the checksum calculation
   #ifndef SREC_START_CHECKSUM
   #define SREC_START_CHECKSUM      (0x00010000)
   #endif

   #ifndef SREC_END_CHECKSUM
   #define SREC_END_CHECKSUM        (0x00080000)
   #endif

   // definitions for 0.5M flash
   #define START_OF_FLASH           (0x20000000)      // start of application flash
   #define END_OF_FLASH             (0x2007FFFF)      // last valid flash address
   #define START_OF_CODE            (START_OF_FLASH + SREC_START_CHECKSUM)  // flash space on the blackfin
   #define END_OF_CODE              (START_OF_FLASH + SREC_END_CHECKSUM)    // End of code space -- used for code checksum
   #define START_OF_FLASH_CRC       (START_OF_FLASH)  // start of application flash

   // sizes reported to external programmer
   #define FLASH_PAGE_SIZE          (256)             // bytes per page
#endif

// definitions independent of flash size
#define SHARC_START_OF_FLASH        (0x03000000)      // for backwards-compatibility

#endif 
