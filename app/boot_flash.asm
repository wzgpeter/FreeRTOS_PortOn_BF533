
/*******************************************************************************/
/*                                                                             */
/* Boot ROM for the Blackfin products and silicon revisions                    */
/*                                                                             */
/*     ADSP-BF531-0.3                                                          */
/*     ADSP-BF532-0.3                                                          */
/*     ADSP-BF533-0.3                                                          */
/*     ADSP-BF531-0.4                                                          */
/*     ADSP-BF532-0.4                                                          */
/*     ADSP-BF533-0.4                                                          */
/*     ADSP-BF538-0.0                                                          */
/*     ADSP-BF539-0.0                                                          */
/*                                                                             */
/*     for documentation see application note                                  */
/*                                                                             */
/*        ADSP-BF533 Blackfin® Booting Process (EE-240)                        */
/*                                                                             */
/*     at http://www.analog.com/ee-notes                                       */ 
/*                                                                             */
/* (c) Analog Devices Inc. (2004)                                              */
/*                                                                 Hiren Desai */
/*******************************************************************************/

#include <defBF533.h>

.EXTERN ldf_stack_end;

#ifndef SREC_APP_START_ADDRESS
#define SREC_APP_START_ADDRESS     (0x00010000)
#endif


#define SYSMMR_BASE  0xFFC00000
#define COREMMR_BASE 0xFFE00000
#define L1_Code		 0xFFA00000		// L1 Code SRAM
// Async Bank 0 - Starts at 0x20000000 because that is where flash is memory mapped
#define ASYNC_Bank0  (0x20000000 + SREC_APP_START_ADDRESS)		
#define SYSCFG_VALUE 0x30			// For Anomaly #22.



/*******************************************************************************/
/*                                                                             */
/*  Data section for intermediate storage of DMA headers.                      */
/*  These 16 address locations must not be used by the application, or at      */ 
/*  least they cannot be initialized at boot time                              */
/*  The ZERO byte is used for zero filling data.  With max access rates of     */
/*  14MHz for internal flash, 133MHz for external SDRAM, and 500MHz for L1,    */
/*  this section must be placed in L1 memory for optimal performance           */
/*                                                                             */ 
/*******************************************************************************/


.SECTION tempdata;
.VAR reserved1;				// 0xFF807FF0
.VAR HeaderTargetAddress;	// 0xFF807FF4
.VAR HeaderByteCount;		// 0xFF807FF8
.BYTE2 HeaderFlags;			// 0xFF807FFB
.BYTE2 TrashCan;			// 0xFF807FFE

.BYTE ZERO[1] = 0x0;		// source data for ZEROFILL Memory DMA


/*******************************************************************************/
/*******************************************************************************/
/*                                                                             */
/*  ROM entrypoint. Resides at address 0xEF000000                              */
/*                                                                             */
/*******************************************************************************/
/*******************************************************************************/

.SECTION seg_flash;
.GLOBAL _bootkernel;
_bootkernel:


/*******************************************************************************/
/*                                                                             */
/*  Initialize global register variables                                       */
/*                                                                             */
/*******************************************************************************/

	R1 = SYSCFG_VALUE;
	SYSCFG = R1;				// For Anomaly #22.

	SP.H = ldf_stack_end;		// Set up supervisor stack in the end of L1 
	SP.L = ldf_stack_end;		// Scratch Memory

	P0.H = HI(COREMMR_BASE);	// P0 Points to the beginning of CORE MMR Space
	P0.L = LO(COREMMR_BASE);

	P1.H = HI(SYSMMR_BASE);		// P1 Points to the beginning of SYSTEM MMR 
	P1.L = LO(SYSMMR_BASE);		// Space

	P3.H = HeaderTargetAddress;	// P3 Points to temporary L1 memory for
	P3.L = HeaderTargetAddress;	// intermediate storage of block headers
	
	P4.H = HI(L1_Code);			// P4 points to L1 Instruction Memory for   
	P4.L = LO(L1_Code);			// ADSP-BF533 (0xFFA00000)

	P5.H = HI(EVT1);			// P5 points to the RESET vector within the 
	P5.L = LO(EVT1);			// Event Vector Table
		

/*******************************************************************************/
/*                                                                             */
/*  Degrade interrupt level from reset down to IVG15                           */
/*  Continue to operate in supervisor mode                                     */
/*                                                                             */
/*******************************************************************************/

	P2.H = SERVICE_IVG15;		// Place the address of start code in IVG15 of 
	P2.L = SERVICE_IVG15;		// EVT
	[P0+LO(EVT15)] = P2;										

	R1.L = LO(EVT_IVG15);
	[P0+LO(IMASK)] = R1;		// Set (enable) IVG15
	
	RAISE 15;					// Invoke IVG15 interrupt
	
	P2.L = WAIT_FOR_IVG15;
	RETI = P2;					// Set Reset Return Address 
	RTI;						// Return from Reset Interrupt
	
WAIT_FOR_IVG15:	           		// Wait here till IVG15 interrupt is latched
	JUMP WAIT_FOR_IVG15;		// The only instruction user mode 

SERVICE_IVG15:	
	[--SP] = RETI;				// RETI must be pushed to enable interrupts.  										


/*******************************************************************************/
/*                                                                             */
/*  Test BMODE Pins                                                            */
/*                                                                             */
/*******************************************************************************/
	
	R7 = W[P1+LO(SYSCR)] (Z);
	R1 = 0x000E;
	R0 = R7 & R1;

	
/*******************************************************************************/
/*                                                                             */
/*  R4.L = DMA Config Value for SPI DMA and Destination Memory DMA             */
/*  R4.H = SPI Control Register Value                                          */
/*                                                                             */
/*******************************************************************************/
	
	R4.L = DI_EN | WDSIZE_8 | WNR | DMAEN;	// 0x0083		

	R4.H = SPE | MSTR | (TIMOD & 2);		// 0x5002, CPHA = CPOL = 0
											// DMA receive mode (TIMOD = 10) 


/*******************************************************************************/
/*                                                                             */
/*  Global DMA Settings                                                        */
/*     MDMA0 for parallel flash boot and zero fill                             */
/*     DMA5 for SPI boot                                                       */
/*                                                                             */
/*******************************************************************************/
												
	R1 = 0x0;
	W[P1+LO(MDMA_S0_X_MODIFY)] = R1;	// MDMA Source Modify = 0 for Zero Fill
	R1 += 1;
	W[P1+LO(DMA5_X_MODIFY)] = R1;		// SPI DMA Modify = 1
	W[P1+LO(MDMA_D0_X_MODIFY)] = R1;	// MDMA Destination Modify = 1
	R3 = R1;							// Source DMAConfig Value (8-bit words)


/*******************************************************************************/
/*                                                                             */
/*  Test Bit 4 of SYSCR Register whether need to reboot on software reset.     */
/*  Otherwise bypass boot sequence and jump to reset vector stored in EVT1     */
/*                                                                             */
/*******************************************************************************/

	CC = BITTST(R7,4);
	R7 = 0x0;							// Set R7 = 0x0 for SPI Master Mode Boot
	IF CC JUMP SWRESET;				


/*******************************************************************************/
/*                                                                             */
/*  Test BMODE pins and jump to respective boot threads                        */
/*                                                                             */
/*  BMODE[1:0] = 00 Bypass (no-boot) mode, bootkernel is not even invoked      */
/*               01 Boot from 8-bit or 16-bit parallel Flash                   */
/*               10 SPI slave boot from host                                   */
/*               11 SPI master boot from 8/16/24-bit SPI Memory or DataFlash   */
/*                                                                             */
/*******************************************************************************/
		
HARD_RESET:							
	R0 = R0 >> 1;					// BMODE[1:0] = SYSCR[2:1]	
	CC = R0 == 0x1;					
	IF CC JUMP FLASH_BOOT;			// BMODE = 01, Boot from 8-/16-bit Flash

	CC = R0 == 0x2;
	IF CC JUMP SPI_SLAVE_BOOT;		// BMODE = 010, SPI Slave Boot
	
	JUMP SPI_BOOT;					// BMODE = 011, Boot from 8-/16-/24-bit SPI 
									// Serial ROM or ATMEL SPI DataFlash
				
/*******************************************************************************/
/*******************************************************************************/
/*                                                                             */
/*  Boot from 8-bit or 16-bit parallel flash thread                      .     */
/*                                                                             */
/*******************************************************************************/
/*******************************************************************************/
														
FLASH_BOOT:


/*******************************************************************************/
/*                                                                             */
/*  First test whether 8-bit or 16-bit device connected to EBIU, /MS0    .     */
/*    8-bit boot stream starts with an 0x40 byte                               */
/*    16-bit boot streams start with an 0x60 byte                              */
/*                                                                             */
/*  Subroutine FLASH_8_OR_16_BIT tests upper nibble of first byte and          */
/*  arranges DMA settings accordingly. This causes an extra EBIU access        */
/*                                                                             */
/*******************************************************************************/

	CALL FLASH_8_OR_16_BIT;

		
/*******************************************************************************/
/*                                                                             */
/*  R0 functions as Next Boot Block Pointer                              .     */
/*                                                                             */
/*******************************************************************************/

	R0 = P0;						// Set R0 to the start of Async Bank 0 
									// (0x20000000)


/*******************************************************************************/
/*                                                                             */
/*  Parse Boot Stream Block by Block                                     .     */
/*                                                                             */
/*  First DMA 10-bytes header in and evaluate flags                            */
/*                                                                             */
/*  Bit     0: ZEROFILL                                                        */
/*  Bit     1: RESVECT                                                         */
/*  Bit     2: reserved                                                        */
/*  Bit     3: INIT                                                            */
/*  Bit     4: IGNORE                                                          */
/*  Bits  8-5: PFLAG (not used in flash mode)                                  */
/*  Bits 14-9: reserved                                                        */
/*  Bits   15: FINAL                                                           */
/*                                                                             */
/*  Then program the reset vector as required per derivative                   */
/*                                                                             */
/*  ADSP-BF531: 0xFFA08000                                                     */
/*  ADSP-BF532: 0xFFA08000                                                     */
/*  ADSP-BF533: 0xFFA00000                                                     */
/*                                                                             */
/*  Note that all parts use the identical bootkernel                           */
/*                                                                             */
/*******************************************************************************/
		
FGRAB_HEADER:
	CALL FDMA_HEADER;
	CALL SET_RESET_VECTOR;
	P0 = R1;							// P0 = Destination Address

	
/*******************************************************************************/
/*                                                                             */
/*  Test IGNORE Flag                                                     .     */
/*                                                                             */
/*  If set don't perform further DMA. Only update Next Boot Block Pointer in   */
/*  R0 by the block's byte count, which is multiplied by 2 in 8-bit boot mode. */
/*  Finally test also for INIT and FINAL flag which ar mutually exclusive for  */
/*  legacy reasons.                                                            */
/*                                                                             */
/*******************************************************************************/
	
FCHECK_IGNORE_BIT:
	CC = BITTST(R5,4);
	IF !CC JUMP FCHECK_INIT_BIT;
	
	CC = BITTST(R4,16);					// Multiply count value by 2 so address 
	IF CC JUMP UPDATE_ADDR;				// increments by 2 for 8-bit part 
	R2 = R2 << 1;						// (EBIU is 16 bits wide)										
										
UPDATE_ADDR:	
	R0 = R0 + R2;						// Update Source Base Address
	CC = BITTST(R5,3);					// Check Init Bit within FLAG
	IF !CC JUMP FCHECK_LAST_SECTION;	// Ignore this block and check if this 
										// is the last one
	CALL(P0);							// If Init Bit is set, Call Init Code
	JUMP FCHECK_LAST_SECTION;			// Check if this is the last block
     

/*******************************************************************************/
/*                                                                             */
/*  Test INIT Flag                                                       .     */
/*                                                                             */
/*  If set issue a CALL instruction to the blocks target address after having  */
/*  it booted in.                                                              */
/*                                                                             */
/*  An INIT Block cannot be the FINAL block within the loader stream. This is  */
/*  for legacy support of VisualDSP++ 3.1 and 3.5 which sets the FINAL flag    */
/*  for INIT blocks always.                                                    */
/*                                                                             */
/*******************************************************************************/
	
FCHECK_INIT_BIT:
	CC = BITTST(R5,3);
	IF !CC JUMP FCHECK_ZERO_FILL_BIT;
	
	CALL FDMA;							// DMA Init Code into internal memory
	R0 = [P1+LO(MDMA_S0_CURR_ADDR)];	// Get Source Base Address
	CALL(P0);							// Call Init Code
	JUMP FGRAB_HEADER;					// Grab the next header 	

	
/*******************************************************************************/
/*                                                                             */
/*  Test ZEROFILL Flag                                                   .     */
/*                                                                             */
/*  If set simply patch the MDMA Source Pointer and set Source Modifier zero   */
/*                                                                             */
/*******************************************************************************/
	
FCHECK_ZERO_FILL_BIT:
	CC = BITTST(R5,0);
	IF !CC JUMP FDO_DMA;				// Zero Fill?
	
	R0.H = ZERO;	
	R0.L = ZERO;						// Source Base Address = ZERO Location
    R6 = 0x0;
	W[P1+LO(MDMA_S0_X_MODIFY)] = R6;	// Source Modify = 0 for Zero Fill
FDO_DMA:
	
/*******************************************************************************/
/*                                                                             */
/*  Finally perform the Memory DMA                                       .     */
/*                                                                             */
/*******************************************************************************/

	CALL FDMA;

/*******************************************************************************/
/*                                                                             */
/*  Restore all registers that have potentially been patched:            .     */
/*    DMA source modify is 2 for 8-bit mode and 1 for 16-bit mode              */
/*    R0 points to Next Boot Block again                                       */
/*                                                                             */
/*******************************************************************************/
		
	R0 = 0x2;
	CC = BITTST(R4,16);
	IF CC R0 = R3;						// R0 = 0x1 (R3 is 0x1)
	W[P1+LO(MDMA_S0_X_MODIFY)] = R0;	// Source Modify = 2 for 8-bit part OR 
										// Source Modify = 1 for 16-bit part
	
	CC = BITTST(R5,0);					// If Zero Fill, Restore R0 to Flash
	IF CC R0 = R7;						// Memory 
										
	R7 = [P1+LO(MDMA_S0_CURR_ADDR)];	// If Not Zero Fill, R0 = Current Source
	IF !CC R0 = R7; 					// Base Address										

	
/*******************************************************************************/
/*                                                                             */
/*  Test FINAL Flag                                                      .     */
/*                                                                             */
/*  Either fetch next block header or exit and jump to reset vector            */
/*                                                                             */
/*******************************************************************************/
	
FCHECK_LAST_SECTION:	
	CC = BITTST(R5,15);					// Last Section?
	IF !CC JUMP FGRAB_HEADER;			// If not, Jump to grab header
	JUMP BOOT_END;

FLASH_BOOT.END:
	

/*******************************************************************************/
/*                                                                             */
/*  FDMA Subroutine for Flash Boot Mode                                  .     */
/*                                                                             */
/*  Sets up a MDMA sequence and waits until MDMA is done.                      */
/*  This functions is used to fetch block headers, to boot payload data, as    */
/*  well as to perform zero-initialization.                                    */
/*                                                                             */
/*  Supports byte counts from 0 to 65536                                       */
/*                                                                             */
/*******************************************************************************/

FDMA:
	CC = R2 == 0x0;
	IF CC JUMP FDMA.EXIT;				// If the COUNT = 0, skip the DMA
		
	[P1+LO(MDMA_S0_START_ADDR)] = R0;	// Set Source Base Address
	W[P1+LO(MDMA_S0_X_COUNT)] = R2;		// Set Source Count
	W[P1+LO(MDMA_S0_CONFIG)] = R3;		// Set Source:  DMAConfig = DMA Enable, 
										// Memory Read,  8-Bit Transfers, 1-D DMA, 
										// Flow = Stop
	
	[P1+LO(MDMA_D0_START_ADDR)] = R1;	// Set Destination Base Address
	W[P1+LO(MDMA_D0_X_COUNT)] = R2;		// Set Destination Count
	W[P1+LO(MDMA_D0_CONFIG)] = R4;		// Set Destination DMAConfig = DMA Enable, 
										// Memory Write, 8-Bit Transfers, 1-D DMA, 
										// Flow = Stop, Interrupt on Completion
	
	IDLE;								// Wait for DMA to Complete
	
	W[P1+LO(MDMA_D0_IRQ_STATUS)] = R3;	// Write 1 to clear DMA interrupt (R3 = 1)
	
FDMA.EXIT:
	RTS;
FDMA.END:
	

/*******************************************************************************/
/*                                                                             */
/*  FDMA_HEADER Subroutine for Flash Boot Mode                           .     */
/*                                                                             */
/*  Fetches the next 10-byte Block Header, and updated working registers       */
/*  accordingly. L1 memory is used for temporary storage.                      */
/*                                                                             */
/*******************************************************************************/
	
FDMA_HEADER:
	[--SP] = RETS;
	R1 = P3;						// intermediate L1 storage address
	R2 = 0xA;						// 10-bytes header
	
	CALL FDMA;						// perform Memory DMA
	
	R1 = [P3];						// Target Address
	R2 = [P3+0x4];					// Byte Count
	R5 = W[P3+0x8](Z);				// Flags

	R0 = [P1+LO(MDMA_S0_CURR_ADDR)];// Get Source Base Address
	R7 = R0;						// Save Source Base Address for zero fill
	
	RETS = [SP++];
	RTS;
FDMA_HEADER.END:	

/*******************************************************************************/
/*                                                                             */
/*  FLASH_8_OR_16_BIT Subroutine for Flash Boot Mode                     .     */
/*                                                                             */
/*  It determines whether an 8- or 16-bit flash/prom is connected to /MS0 and  */
/*  sets up DMA parameters as required.                                        */
/*                                                                             */
/*  16-bit flash boot has not been supported by 0.2 silicon yet.               */
/*                                                                             */
/*  For 16-bit boot the very first byte of the boot stream is 0x60 by          */
/*  definition, for 8-bit mode it is 0x40. Earlier silicon revisions required  */
/*  the first byte to be zero. Therefore this functions tests the upper nibble */
/*  of the first byte: if it equals 6 then 16-bit mode is performed. Otherwise */
/*  8-bit mode is assumed.                                                     */
/*                                                                             */
/*  The target address field of the first IGNORE blocks reads to the kernel:   */
/*                                                                             */
/*	Address         8-bit flash     8-bit flash (rev 0.2)   16-bit flash       */
/*  0x20000000      xx  40          xx  00                  00  60             */ 
/*  0x20000002      xx  00          xx  00                  FF  80             */
/*  0x20000004      xx  80          xx  80                  cc  cc             */
/*  0x20000006      xx  FF          xx  FF                  cc  cc             */
/*                                                                             */
/*******************************************************************************/

FLASH_8_OR_16_BIT:
	P0.H = HI(ASYNC_Bank0);
	P0.L = LO(ASYNC_Bank0);
	R1 = B[P0](Z);						// Read the 1st byte from flash memory
	R2 = 0xF0;
	R1 = R1 & R2;						// Extract bits[7:4] of the first byte
	R2 = 0x60;
	CC = R1 == R2;						// Are bits[7:4] = 6.  If so, it's a 
										// 16-bit part otherwise 8-bit part

FLASH_16_BIT:
	R4.H = 0x1;							// R4.H = 0x1 for 16-bit parts  
										// (used to increment pointer by 1 for  
										// ignore blocks)
										
	W[P1+LO(MDMA_S0_X_MODIFY)] = R3;	// Source Modify = 1 for 16-bit parts 
										// (R3 = 0x1)
	IF CC JUMP FLASH_8_OR_16_BIT.EXIT;

FLASH_8_BIT:
	R4.H = 0x0;							// R4.H = 0x0 for 8-bit part 
										// (used to increment pointer by 2 for 
										// ignore block)
	R0 = 0x2;
	W[P1+LO(MDMA_S0_X_MODIFY)] = R0;	// Source Modify = 2 for 8-bit parts

FLASH_8_OR_16_BIT.EXIT:
	RTS;
FLASH_8_OR_16_BIT.END:
	

/*******************************************************************************/
/*******************************************************************************/
/*                                                                             */
/*  Boot from SPI memory (SPI Master Boot) thread                        .     */
/*                                                                             */
/*******************************************************************************/
/*******************************************************************************/

SPI_BOOT:								

/*******************************************************************************/
/*                                                                             */
/*  First, determine which device is connected                           .     */
/*                                                                             */
/*******************************************************************************/

	CALL SPI_DEVICE_DETECTION;
	

/*******************************************************************************/
/*                                                                             */
/*  Parse Boot Stream Block by Block                                     .     */
/*                                                                             */
/*  First DMA 10-bytes header in and evaluate flags                            */
/*                                                                             */
/*  Bit     0: ZEROFILL                                                        */
/*  Bit     1: RESVECT                                                         */
/*  Bit     2: reserved                                                        */
/*  Bit     3: INIT                                                            */
/*  Bit     4: IGNORE                                                          */
/*  Bits  8-5: PFLAG                                                           */
/*  Bits 14-9: reserved                                                        */
/*  Bits   15: FINAL                                                           */
/*                                                                             */
/*  Then program the reset vector as required per derivative                   */
/*                                                                             */
/*  ADSP-BF531: 0xFFA08000                                                     */
/*  ADSP-BF532: 0xFFA08000                                                     */
/*  ADSP-BF533: 0xFFA00000                                                     */
/*                                                                             */
/*  Note that all parts use the identical bootkernel                           */
/*                                                                             */
/*******************************************************************************/

GRAB_HEADER:
	CALL SPI_ADDRESS;					// Address passed in R3. Initial value
										// is zero.
	CALL DMA_HEADER;					// Fetch header to intermediate memory
										// R1 = Target Address 
										// R2 = Byte Count
										// R5 = Flags
	CALL SET_RESET_VECTOR;	
	P0 = R1;							// P0 = Destination Address
	CALL DISABLE_SPI;					// Disable SPI Interface	
	R3 += 0xA;							// Increment external address by header

	
/*******************************************************************************/
/*                                                                             */
/*  Test IGNORE Flag                                                     .     */
/*                                                                             */
/*  If set incremenent Source Address Pointer R3 and don't perform any DMA.    */
/*  Enable INIT flag and FINAL flag to be active at the same time.             */
/*                                                                             */
/*******************************************************************************/
	
CHECK_IGNORE_BIT:
	CC = BITTST(R5,4);
	IF !CC JUMP CHECK_INIT_BIT;
	
	R3 = R3 + R2;						// Update Source Base Address
	CC = BITTST(R5,3);					// Check if the INIT flag is set 
	IF !CC JUMP CHECK_LAST_SECTION;		// otherwise test for FINAL flag
	
	CALL(P0);							// If INIT flag is set, Call Init Code
	JUMP CHECK_LAST_SECTION;			// Check if this is the last block
	
	
/*******************************************************************************/
/*                                                                             */
/*  Test INIT Flag                                                       .     */
/*                                                                             */
/*  If set DMA data in and issue a CALL instruction to the target address.     */
/*  INIT flag and FINAL flag are mutually exclusive for legacy support.        */
/*  This is due to VisualDSP++  3.1 and 3.5 which set the FINAL flag for Init  */
/*  Blocks always.                                                             */
/*                                                                             */
/*******************************************************************************/
	
CHECK_INIT_BIT:
	CC = BITTST(R5,3);
	IF !CC JUMP CHECK_ZERO_FILL_BIT;
	
	CALL SPI_ADDRESS;					// Before setting up SPI DMA, we need 
										// to address the SPI memory	
	CALL SPI_DMA;						// DMA Init Code into internal memory
	CALL DISABLE_SPI;					// Disable SPI Interface	
	R3 = R3 + R2;						// Update Source Base address
	CALL(P0);							// Call Init Code
	JUMP GRAB_HEADER;					// Grab the next header 
											
	
/*******************************************************************************/
/*                                                                             */
/*  Test ZEROFILL Flag                                                   .     */
/*                                                                             */
/*  If set patch Source Address and start Flash Boot Memory DMA                */
/*                                                                             */
/*******************************************************************************/

CHECK_ZERO_FILL_BIT:
	CC = BITTST(R5,0);		
	IF !CC JUMP DO_DMA;					// Zero Fill?
	
	R0.H = ZERO;
	R0.L = ZERO;
	[--SP] = R3;						// Save R3 because FDMA routine uses R3
	R3 = 0x1;							// R3 = Source DMA Config Value
	CALL FDMA;
	R3 = [SP++];						// Restore R3
	JUMP CHECK_LAST_SECTION;			// Check if this is the last block

	
/*******************************************************************************/
/*                                                                             */
/*  DMA payload data in                                                  .     */
/*                                                                             */
/*******************************************************************************/
	
DO_DMA:
	CALL SPI_ADDRESS;					// Before setting up SPI DMA, we need 
										// to address the SPI memory	
	CALL SPI_DMA;
	CALL DISABLE_SPI;					// Disable SPI Interface	
	R3 = R3 + R2;						// increment external address by count 
										// value	


/*******************************************************************************/
/*                                                                             */
/*  Test FINAL Flag                                                      .     */
/*                                                                             */
/*  Either fetch next block header or exit and jump to reset vector            */
/*                                                                             */
/*******************************************************************************/
	
CHECK_LAST_SECTION:
	CC = BITTST(R5,15);					// Last Section?
	IF CC JUMP BOOT_END;
	JUMP GRAB_HEADER;					// If not set continue

		
SPI_BOOT.END:	
	
	
/*******************************************************************************/
/*                                                                             */
/*  SPI_DEVICE_DETECTION Subroutine for SPI Master Boot Mode             .     */
/*                                                                             */
/*  The following type of SPI devices are supported:                           */
/*    SPI Flash/EEPROM with 8-bit addressing scheme  (1 address byte)          */
/*    SPI Flash/EEPROM with 16-bit addressing scheme (2 address bytes)         */
/*    SPI Flash/EEPROM with 24-bit addressing scheme (3 address bytes)         */
/*    Atmel DataFlash                                                          */
/*                                                                             */
/*******************************************************************************/
	
SPI_DEVICE_DETECTION:
	[--SP] = RETS;						// Save return address onto stack
	

/*******************************************************************************/
/*                                                                             */
/*  Set Bit Rate                                                         .     */
/*                                                                             */
/*******************************************************************************/

#ifdef _DSP_MODE_
     R1 = 0x0005;					// Smaller Value for faster simulation.
     								// for testing only
#else
     R1 = 0x85;
#endif
	W[P1+LO(SPI_BAUD)] = R1;		// set baud rate register


/*******************************************************************************/
/*                                                                             */
/*  Use PF2 as SPI chip select.                                          .     */
/*                                                                             */
/*******************************************************************************/
	
	R0 = 0x4;
	W[P1+LO(FIO_DIR)] = R0;		// set PF2 as an output, this is our CS line


/*******************************************************************************/
/*                                                                             */
/*  Start with device detection and return result in R6                  .     */
/*                                                                             */
/*  R6 = 0x3 for 8-bit  SPI Memory                                             */
/*  R6 = 0x2 for 16-bit SPI Memory                                             */
/*  R6 = 0x1 for 24-bit SPI Memory                                             */
/*  R6 = 0x0 for ATMEL DataFlash Memory                                        */
/*                                                                             */
/*******************************************************************************/

PART_SELECT:
	R3 = 0x0;						// Initial address of external memory
	R1 = 0xFF;						// Test value for receive byte
	R6 = 0x3;						// set R6 = 0x3 for 8-bit part
	
	R0 = SPE | MSTR | (TIMOD & 1);	// 0x5001;
	W[P1+LO(SPI_CTL)] = R0;			// enable SPI, non-DMA TX Mode, 8 bits
	
	R0 = PF2;
	W[P1+LO(FIO_FLAG_C)] = R0;		// assert CS low
	
	R0 += -1;						// send out control word (R0 = 0x03)
	CALL SINGLE_BYTE_TRANSFER;
	
	R0 = R3;						// send out address byte (address = 0)
	CALL SINGLE_BYTE_TRANSFER;
    
/*******************************************************************************/
/*                                                                             */
/*  If the byte received in the third transfer is not equal 0xFF then it is    */
/*  an 8-bit addressable SPI memory. 16- and 24-bit devices don't send data    */
/*  yet and the MISO line is still in high impedance state. That is why a      */
/*  pull-up resistor is required on the MISO signal.                           */
/*                                                                             */
/*******************************************************************************/

EIGHT_BIT_CHECK:
	R0 = R3;						// send out dummy byte for 8-bit devices									
	CALL SINGLE_BYTE_TRANSFER;		// or second address byte otherwise   	

	CC = R0 == R1;					// does the received byte equal 0xFF ? 
	IF !CC JUMP SPI_DEVICE_DETECTION.EXIT;	// otherwise it is an 8-bit part
	
	
/*******************************************************************************/
/*                                                                             */
/*  If the byte received in the fourth transfer is not equal 0xFF then it is   */
/*  a 16-bit addressable SPI memory. 24-bit devices don't send data yet and    */
/*  the MISO line is still in high impedance state.                            */
/*                                                                             */
/*******************************************************************************/

SIXTEEN_BIT_CHECK:
	R0 = R3;						// send out dummy byte for 16-bit devices
	CALL SINGLE_BYTE_TRANSFER;		// or third byte otherwise

	CC = R0 == R1;					// does the received byte equal 0xFF ? 
	R6 += -1;						// set R6 = 0x2 for 16-bit parts
	IF !CC JUMP SPI_DEVICE_DETECTION.EXIT;	// if not 0xFF it is a 16-bit part


/*******************************************************************************/
/*                                                                             */
/*  If the byte received in the fifth transfer is not equal 0xFF then it is    */
/*  a 24-bit addressable SPI memory. Atmel DataFlash devices still tri-state   */
/*  the MISO pin.                                                              */
/*                                                                             */
/*******************************************************************************/
	
TWENTYFOUR_BIT_CHECK:
	R0 = R3;						// send out dummy byte
	CALL SINGLE_BYTE_TRANSFER;	

	CC = R0 == R1;					// does the received byte equal 0xFF ?
	R6 += -1;						// set R6 = 0x1 for 24-bit parts
	IF !CC JUMP SPI_DEVICE_DETECTION.EXIT;	// if not 0xFF it is a 24-bit part

ATMEL_PART:	R6 += -1;				// if all of the tests above fail, we are
	CALL READ_STATUS_REGISTER;		// going to assume we have an ATMEL 
									// DataFlash Memory connected

SPI_DEVICE_DETECTION.EXIT:
	CALL DISABLE_SPI;
	
	RETS = [SP++];					// Restore return address from stack
	RTS;

SPI_DEVICE_DETECTION.END:
	
	
/*******************************************************************************/
/*                                                                             */
/*  DMA_HEADER Subroutine for SPI Master and Slave Boot Mode                   */
/*                                                                             */
/*  It fetches the next 10-byte header in by DMA. L1 memory is used for        */
/*  intermediate storage, but then result is copied to registers:              */
/*                                                                             */
/*  R1 = Target Address (4 bytes)                                              */
/*  R2 = Byte Count (4 bytes)                                                  */
/*  R5 = Flags (2 bytes)                                                       */
/*                                                                             */
/*******************************************************************************/
	
DMA_HEADER:
	[--SP] = RETS;

	R1 = P3;							// DMA Destination = 0xFF807FF4
	R2 = 0xA;							// DMA Count = 10

	CALL SPI_DMA;

	R1 = [P3];							// Target Address
	R2 = [P3+0x4];						// Byte Count
	R0 = W[P3+0x8](Z);					// Flags
	
	R5.L = 0x1001;						// Deposit 1 bit at bit position = 16
	R5 = DEPOSIT(R0, R5);				// After this instruction R5.L = R0.L  
										// and bit0 of R5.H will be retrieved
	RETS = [SP++];
	RTS;
	
DMA_HEADER.END:


/*******************************************************************************/
/*                                                                             */
/*  SPI_DMA Subroutine for SPI Master and Slave Boot Mode                      */
/*                                                                             */
/*  It starts a SPI DMA and wait until it completes                            */
/*  It supports byte counts from 0 to 65536 bytes                              */
/*                                                                             */
/*******************************************************************************/

SPI_DMA:
	[P1+LO(DMA5_START_ADDR)] = R1;	// Set Destination Base Address
	W[P1+LO(DMA5_X_COUNT)] = R2;	// Set Destination Count

	CC = R2 == 0x0;
	IF CC JUMP SPI_DMA.EXIT;		// If the COUNT = 0, skip the DMA

	W[P1+LO(DMA5_CONFIG)] = R4;	 	// Set Destination DMAConfig = DMA Enable, 
									// Memory Write, 1-D DMA, Flow = Stop,  
									// Interrupt on Completion

	R0 = R4 >> 16;
	W[P1+LO(SPI_CTL)] = R0;			// enable SPI, DMA RX Mode, 8-bit data

	IDLE;							// Wait for DMA to Complete

	R0 = 0x1;
	W[P1+LO(DMA5_IRQ_STATUS)] = R0;	// Write 1 to clear DMA interrupt request
	
	CC = BITTST(R7,0);				// R7 = 0x0 for SPI Master Mode Boot; 
	IF CC JUMP SPI_DMA.EXIT;		// R7 = 0x1 for SPI Slave Mode Boot
	
	W[P1+LO(DMA5_CONFIG)] = R7;		// Disable DMA only for SPI Master Mode Boot 
									// R7 is already 0x0

SPI_DMA.EXIT:
	RTS;
	
SPI_DMA.END:	


/*******************************************************************************/
/*                                                                             */
/*  SPI_ADDRESS Subroutine for SPI Master Boot Mode                            */
/*                                                                             */
/*  Sends a new Address Command to SPI device, Address passed in R3            */
/*                                                                             */
/*  Recall:                                                                    */
/*  R6 = 0x3 for 8-bit  SPI Memory                                             */
/*  R6 = 0x2 for 16-bit SPI Memory                                             */
/*  R6 = 0x1 for 24-bit SPI Memory                                             */
/*  R6 = 0x0 for ATMEL DataFlash Memory                                        */
/*                                                                             */
/*******************************************************************************/

SPI_ADDRESS:
	[--SP] = RETS;					// Save return address onto the stack
	
	I0 = R3;						// save logical address in I0
	
	R0 = SPE | MSTR | (TIMOD & 1);	// 0x5001;
	W[P1+LO(SPI_CTL)] = R0;			// enable SPI, non-DMA TX mode, 8 bit
	
	R0 = PF2;
	W[P1+LO(FIO_FLAG_C)] = R0;		// assert CS (PF2) low
	
	CC = R6 < 1;					// if ATMEL DataFlash
	IF !CC JUMP ADDRESS_8_16_24;
	
ATMEL:
	CALL ADDRESS_DECODE;			// convert logical address into a page number 
									// and byte number
	R0 = 0xE8;						// control word for ATMEL SPI Memory
	CALL SINGLE_BYTE_TRANSFER;		// send out control word

	JUMP ADDRESS_24;	
	
ADDRESS_8_16_24:					// Normal SPI devices
	R0 = 0x3;						// control word for 8-/16-/24-bit SPI Memory
	CALL SINGLE_BYTE_TRANSFER;		// send out control word
	
	CC = R6 < 2;
	IF !CC JUMP ADDRESS_16;
	
ADDRESS_24:
	R0 = R3 >> 16;					// send out upper address byte
	CALL SINGLE_BYTE_TRANSFER;	
	
ADDRESS_16:
	CC = R6 < 3;
	IF !CC JUMP ADDRESS_8;

	R0 = R3 >> 8;					// send out middle address byte
	CALL SINGLE_BYTE_TRANSFER;		

ADDRESS_8:
	R0 = R3;						// send out lower address byte
	CALL SINGLE_BYTE_TRANSFER;

	CC = R6 < 1;
	IF !CC JUMP SPI_ADDRESS.EXIT;
		
	// send out 4 don't care bytes for ATMEL SPI Memory
	P2 = 0x4;															
	LSETUP(ATMEL_Begin, ATMEL_End) LC0=P2;
	ATMEL_Begin:	R0 = R3;			//send out byte
					CALL SINGLE_BYTE_TRANSFER;
	ATMEL_End:		NOP;
	
	R3 = I0;						// restore logical address from I0	


SPI_ADDRESS.EXIT:
	R0 = SPE | MSTR | (TIMOD & 0);	// 0x5000;
	W[P1+LO(SPI_CTL)] = R0;			// set TIMOD = 00 for SPI port
		
	RETS = [SP++];					// Restore return address from the stack
	RTS;

SPI_ADDRESS.END:	


/*******************************************************************************/
/*                                                                             */
/*  SINGLE_BYTE_TRANSFER Subroutine for SPI Master Boot Mode                   */
/*                                                                             */
/*  Initiates a single transfer by transmitting the value stored in R0.        */
/*  The received value is returned in R0 again.                                */
/*                                                                             */
/*******************************************************************************/

SINGLE_BYTE_TRANSFER:
	W[P1+LO(SPI_TDBR)] = R0;		// send out byte and start transfer
	
WAIT_FOR_RXS:		
	R0 = W[P1+LO(SPI_STAT)] (Z);	// test bit 5 (RXS) of SPI_STAT register to 
	CC = BITTST(R0,5);				// see if the RX Data Buffer is full, 
	IF !CC JUMP WAIT_FOR_RXS;		// if 0 do test again		
	
	R0 = W[P1+LO(SPI_RDBR)] (Z);	// read buffer and return in R0
	
	RTS;

SINGLE_BYTE_TRANSFER.END:	


/*******************************************************************************/
/*                                                                             */
/*  DISABLE_SPI Subroutine for SPI Master Boot Mode                            */
/*                                                                             */
/*  It disables the SPI port and holds it disabled for 500ns                   */
/*                                                                             */
/*******************************************************************************/

DISABLE_SPI:
	R0 = 0x0000;
	W[P1+LO(SPI_CTL)] = R0;			// disable SPI

//	R0 = W[P1+LO(SPI_RDBR)] (Z);	// dummy read to clear RXS not required
									
	R0 += PF2;
	W[P1+LO(FIO_FLAG_S)] = R0;		// de-assert CS
		
	P2 = 0x01A4;					// Delay of 500ns at SCLK=600MHz
	LSETUP(DELAY_LOOP, DELAY_LOOP) LC0=P2;
	DELAY_LOOP: NOP;					

	RTS;
	
DISABLE_SPI.END:	
	

/*******************************************************************************/
/*                                                                             */
/*  ADDRESS_DECODE Subroutine for SPI Master Boot Mode                         */
/*                                                                             */
/*  It converts a logical address to a page number and a byte number for the   */
/*  ATMEL SPI DataFlash.                                                       */
/*  AT45DB041 has 11 page bits and 9 byte bits (264 bytes/page), R5.H = 0x1    */
/*  AT45DB081 has 12 page bits and 9 byte bits (264 bytes/page), R5.H = 0x1    */
/*  AT45DB161 has 12 page bits and 10 byte bits (528 bytes/page),R5.H = 0x0    */
/*                                                                             */
/*******************************************************************************/

ADDRESS_DECODE:
	R7 = 0x108;					// 264 bytes per page for AT45DB041 and AT45DB081
	
	CC = BITTST(R5, 16);
	IF CC JUMP ATMEL_041_081;
	
ATMEL_161:
	R7 = 0x210;					// 528 bytes per page for AT45DB161
	
ATMEL_041_081:
	R0 = 0x0;					// R0 holds the page number
	
COMPARE:
	CC = R3 <= R7;				// Is the logical address less than bytes/page?
	IF CC JUMP LESS_THAN_PAGE;
	
ADD_PAGE_SUBTRACT:
	R0 += 1;					// if not, add 1 to page number
	R3 = R3 - R7;				// and subtract bytes/page from logical address
	JUMP COMPARE;				// compare again

LESS_THAN_PAGE:	
	CC = R3 == R7;					// if so, check if we have exactly 1 page
	IF !CC JUMP FINISHED_DECODING;
	R3 = 0x0;						// if exact page, byte number = 0
	R0 += 1;						// and add 1 to page number

FINISHED_DECODING:
	R0 = R0 << 10;					// left shift page number by 10 for AT45DB161
	CC = BITTST(R5,16);
	IF !CC JUMP ADDRESS_161;
	R0 = R0 >> 1;					// left shift page number by 9 for AT45DB041 
									// and AT45DB081	
ADDRESS_161:
	R3 = R0|R3;						// OR page number and byte number
	
	R7 = 0x0;						// restore R7 to 0x0 for SPI Slave use
	RTS;
	
ADDRESS_DECODE.END:	
	

/*******************************************************************************/
/*                                                                             */
/*  READ_STATUS_REGISTER Subroutine for SPI Master Boot Mode                   */
/*                                                                             */
/*  It reads the status register of an Atmel DataFlash device to determine     */
/*  which device is connected.                                                 */
/*                                                                             */
/*  AT45DB041 has bits 5:2 set to 0111 (set R5.H = 0x1)                        */
/*  AT45DB081 has bits 5:2 set to 1001 (set R5.H = 0x1)                        */
/*  AT45DB161 has bits 5:2 set to 1011 (set R5.H = 0x0)                        */
/*                                                                             */
/*******************************************************************************/

READ_STATUS_REGISTER:
	[--SP] = RETS;					// Save return address onto stack

	CALL DISABLE_SPI;				// disable SPI for new transfer
	
	R0 = SPE | MSTR | (TIMOD & 1);	// 0x5001;
	W[P1+LO(SPI_CTL)] = R0;			// enable SPI, non-DMA TX Mode, 8 bits
	
	R0 = PF2;
	W[P1+LO(FIO_FLAG_C)] = R0;		// assert CS low
	
	R0 = 0xD7;						// send out control word to 
	CALL SINGLE_BYTE_TRANSFER;		// read status register
	CALL SINGLE_BYTE_TRANSFER;		// result in R0
	
	R5.H = 0x0;						// indicates AT45DB161
	R1 = 0x3C;						// mask bit 5.2
	R0 = R0 & R1;
	R1 = 0x2C;
	CC = R0 == R1;
	IF CC JUMP READ_STATUS_REGISTER.EXIT;
	R5.H = 0x1;						// indicates AT45DB041 or AT45DB081 
	
READ_STATUS_REGISTER.EXIT:
	RETS = [SP++];
	RTS;
	
READ_STATUS_REGISTER.END:
	


/*******************************************************************************/
/*******************************************************************************/
/*                                                                             */
/*  Boot from SPI host (SPI Slave Boot) thread                           .     */
/*                                                                             */
/*******************************************************************************/
/*******************************************************************************/

SPI_SLAVE_BOOT:

	R7 = 0x1;							// Instructs the common SPI_DMA routine 
										// not to disable DMA after DMA is 
										// complete for SPI Slave Boot mode

	R4.H = SPE | CPHA | (TIMOD & 2);	// 0x4402, SPI Control Register Value 						
										// SPI enable, Slave Mode, CPOL = 0, 
										// CPHA = 1, DMA receive mode (TIMOD = 10)
										
/*******************************************************************************/
/*                                                                             */
/*  Parse Boot Stream Block by Block                                     .     */
/*                                                                             */
/*  First DMA 10-bytes header in and evaluate flags                            */
/*                                                                             */
/*  Bit     0: ZEROFILL                                                        */
/*  Bit     1: RESVECT                                                         */
/*  Bit     2: reserved                                                        */
/*  Bit     3: INIT                                                            */
/*  Bit     4: IGNORE                                                          */
/*  Bits  8-5: PFLAG                                                           */
/*  Bits 14-9: reserved                                                        */
/*  Bits   15: FINAL                                                           */
/*                                                                             */
/*  Then program the reset vector as required per derivative                   */
/*                                                                             */
/*  ADSP-BF531: 0xFFA08000                                                     */
/*  ADSP-BF532: 0xFFA08000                                                     */
/*  ADSP-BF533: 0xFFA00000                                                     */
/*                                                                             */
/*  Note that all parts use the identical bootkernel                           */
/*                                                                             */
/*******************************************************************************/										
										
SSB_GRAB_HEADER:
	CALL DMA_HEADER;
	CALL SET_RESET_VECTOR;
	P0 = R1;							// P0 = Destination Address


/*******************************************************************************/
/*                                                                             */
/*  Test IGNORE Flag                                                     .     */
/*                                                                             */
/*  If set set the DMA target modifier to zero and patch the DMA target        */
/*  address in order to trash received data                                    */
/*                                                                             */
/*  Additionally evaluate the PFLAG field of the flag word to determine,       */
/*  PF pin is used to handshake with host device.                              */
/*                                                                             */
/*******************************************************************************/
	
SSB_CHECK_IGNORE_BIT:
	CC = BITTST(R5,4);
	IF !CC JUMP SSB_CHECK_ZERO_FILL_BIT;
	
	CALL SET_FLAG;						// Set appropriate flag for host feedback
	R0 = 0x0;
	W[P1+LO(DMA5_X_MODIFY)] = R0;		// Set SPI DMA Modify to 0
	R1.H = TrashCan;
	R1.L = TrashCan;					// Set the SPI DMA Start Address to 
										// 0xFF807FFE (2nd to last location of 
										// L1 Data Bank A)
	JUMP SSB_DO_DMA;					// Boot in Ignored Block and check if 
										// last section (only first location 
										// will be written to)


/*******************************************************************************/
/*                                                                             */
/*  Test ZEROFILL Flag                                                   .     */
/*                                                                             */
/*  If set use the Memory DMA of the Flash Boot Mode to zero number of bytes.  */
/*  Set Feedback PF Signal while MDMA is ongoing to signal host that device    */
/*  is busy for a while.                                                       */
/*                                                                             */
/*  Note that the ZEROFILL flag may coexist with the FINAL flag.               */
/*                                                                             */
/*******************************************************************************/
										
										
SSB_CHECK_ZERO_FILL_BIT:
	CC = BITTST(R5,0);		
	IF !CC JUMP SSB_DO_DMA;				// Zero Fill?
	
	CALL TOGGLE_FLAG;					// Set PF high to signal host not to 
										// send anymore bytes
	R0.H = ZERO;
	R0.L = ZERO;						// invoke MDMA, Source = ZERO Label
	CALL FDMA;							// R3 already contains a 0x1
	CALL TOGGLE_FLAG;					// Set PF low to signal to host to 
	JUMP SSB_CHECK_LAST_SECTION;		// continue sending bytes
	

/*******************************************************************************/
/*                                                                             */
/*  Perform DMA and restore Modify value again                           .     */
/*                                                                             */
/*******************************************************************************/

SSB_DO_DMA:
	CALL SPI_DMA;
	W[P1+LO(DMA5_X_MODIFY)] = R7;		// Reset SPI DMA Modify back to 1 
										// R7 already contains 0x1
	
/*******************************************************************************/
/*                                                                             */
/*  Test INIT Flag                                                       .     */
/*                                                                             */
/*  If set issue a CALL instruction to the target address, but signal the      */
/*  host that the device might be busy for a while.                            */
/*                                                                             */
/*  Note that the INIT flag may coexist with FINAL and IGNORE flags.           */
/*                                                                             */
/*******************************************************************************/
										
SSB_CHECK_INIT_BIT:
	CC = BITTST(R5,3);
	IF !CC JUMP SSB_CHECK_LAST_SECTION;
	
	CALL TOGGLE_FLAG;					// Set PF high to signal host not to 
										// send anymore bytes
	CALL(P0);							// Call Init Code
	CALL TOGGLE_FLAG;					// Set PF low to signal to host to 
										// continue sending bytes

										
/*******************************************************************************/
/*                                                                             */
/*  Test FINAL Flag                                                      .     */
/*                                                                             */
/*  If set jump to the reset vector. Otherwise load next header and continue.  */
/*                                                                             */
/*******************************************************************************/
										
SSB_CHECK_LAST_SECTION:
	CC = BITTST(R5,15);					// Last Section?
	IF CC JUMP BOOT_END;
	JUMP SSB_GRAB_HEADER;				// JUMP to GRAB_HEADER if not done

SPI_SLAVE_BOOT.END:	
	
	
/*******************************************************************************/
/*                                                                             */
/*  SET_FLAG Subroutine for SPI Slave Boot Mode                          .     */
/*                                                                             */
/*  Evaluates the PFLAG bit field in the FLAG word and enables the output      */
/*  driver of the respective PF pin.                                           */
/*                                                                             */
/*******************************************************************************/
	
SET_FLAG:
	R0 = 0x01E0;					// R5 folds the FLAG word
	R0 = R0 & R5;					// mask Bits 8:5
	R0 = R0 >> 5;					// shift to PF position
	R6.L = LSHIFT R3.L BY R0.L;		// R3 = 1
	R0 = W[P1+LO(FIO_DIR)](Z);		// enable output driver
	R0 = R0 | R6;					
	W[P1+LO(FIO_DIR)] = R0;			// R6 holds the PFLAG position
	W[P1+LO(FIO_FLAG_C)] = R6;		// Set the PF low initially
	RTS;
SET_FLAG.END:
	

/*******************************************************************************/
/*                                                                             */
/*  TOGGLE_FLAG Subroutine for SPI Slave Boot Mode                       .     */
/*                                                                             */
/*  Toggle the PFLAG pin to handshake with host device                         */
/*                                                                             */
/*******************************************************************************/		

TOGGLE_FLAG:
	W[P1+LO(FIO_FLAG_T)] = R6;
	RTS;
TOGGLE_FLAG.END:


/*******************************************************************************/
/*                                                                             */
/*  SET_RESET_VECTOR Subroutine for all Boot Modes                       .     */
/*                                                                             */
/*  The ADSP-BF533 device has a different reset vector address than the        */
/*  ADSP-BF531 and ADSP-BF532 processors. Since 1) all derivatives use the     */
/*  same boot kernel and 2) the EVT1 register is not reset by hardware, the    */
/*  reset vector is controlled by the RESVECT bit (Bit 1) of the FLAG word.    */
/*                                                                             */
/*  Although this routine is invoked often, it executes only once. This way    */
/*  an INIT code is allowed to cutomized the reset vector by patching the      */
/*  EVT1 register                                                              */
/*                                                                             */
/*  ADSP-BF531: RESVECT=0, default EVT1 = 0xFFA08000                           */
/*  ADSP-BF532: RESVECT=0, default EVT1 = 0xFFA08000                           */
/*  ADSP-BF533: RESVECT=1, default EVT1 = 0xFFA00000                           */
/*                                                                             */
/*******************************************************************************/

SET_RESET_VECTOR:
	CC = P4 == 0x0;						// P4 = 0xFFA00000 for the first block 
	IF CC JUMP SET_RESET_VECTOR.EXIT;	// and P4 = 0x0 for subsequent blocks
	
	CC = BITTST(R5,1);
	IF CC JUMP ADSP_BF533;				// Check if BF533
ADSP_BF531_32:
	P4.L = 0x8000;						// If BF531/BF532, set LSB address bits 
										// to 0x8000
ADSP_BF533:
	[P5] = P4;							// Store the start of L1 Instruction 
										// Memory Address into EVT1 for Software 
										// Reset case
	P4 = 0x0;							// Set P4 to 0x0 for subsequent blocks
	
SET_RESET_VECTOR.EXIT:
	RTS;
SET_RESET_VECTOR.END:


/*******************************************************************************/
/*                                                                             */
/*  Finally, Boot Process has finished.                                  .     */
/*  Jump to reset vector stored in EVT1 to exit.                               */
/*  This is also executed on software resets that don't require rebooting      */
/*                                                                             */
/*******************************************************************************/

BOOT_END:
SWRESET:
	P1 = [P5];							// P5 points to the RESET vector within 
										// the Event Vector Table
	JUMP(P1);							// Jump to the RESET vector



_bootkernel.END:

/*******************************************************************************/
/*                                                                             */
/*  end of boot kernel                                                   .     */
/*                                                                             */
/*******************************************************************************/

