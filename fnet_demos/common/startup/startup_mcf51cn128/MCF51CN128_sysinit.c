/**************************************************************************
* 
* Copyright 2009 by Andrey Butok. Freescale Semiconductor, Inc. 
*     
**********************************************************************/ /*!
*
* @file MCF51CN128_sysinit.c
*
* @author Andrey Butok
*
* @date Oct-6-2010
*
* @version 0.0.2.0
*
* @brief Reset configuration of the MCF51CN128 (MDN TOWER board).
*
***************************************************************************/

#include "derivative.h"
#include "exceptions.h"

/* Include the board specific header file. */
#include "MCF51CN128_sysinit.h"
#include "fnet_mcf.h"

/********************************************************************/

#ifdef __cplusplus
#pragma cplusplus off
#endif 

void cop_init();
void mcg_init();

/********************************************************************
*       HW Initialization                     
********************************************************************/
void __initialize_hardware(void)
{
    cop_init();
    mcg_init();
    
    initialize_exceptions();
}

/********************************************************************
*       Computer Operating Properly (COP) Watchdog Initialization                     
********************************************************************/
static void cop_init()
{
	/* Disable COP Watchdog Timer, STOP mode and WAIT mode.
	 */
	SOPT1 = 0;
}

/********************************************************************
*       Multipurpose Clock Generator (MCG) Initialization                      
*********************************************************************/
static void mcg_init()
{


#if XTAL_25MHZ   /* 25MHz Oscilator. FLL Engaged External (FEE) */

    /* Enable oscillator pins 
     */
    PTDPF1_D4 = 0x3;   /* Enable EXTAL.External clock source is connected 
                        * to the EXTAL input pin.*/
    PTDPF1_D5 = 0x3;   /* Enable XTAL. */

    /*1. First, FEI must transition to FBE mode:
     *a) MCGC2 = 0x36 (%00110110)
     *– BDIV (bits 7 and 6) set to %00, or divide-by-1
     *– RANGE (bit 5) set to 1 because the frequency of 8 MHz is within the high frequency range
     *– HGO (bit 4) set to 1 to configure the crystal oscillator for high gain operation
     *– EREFS (bit 2) set to 1, because a crystal is being used
     *– ERCLKEN (bit 1) set to 1 to ensure the external reference clock is active.*/
    MCGC2 = MCGC2_EREFS_MASK | MCGC2_ERCLKEN_MASK | MCGC2_RANGE_MASK | MCGC2_HGO_MASK;        

    /*b) Loop until OSCINIT (bit 1) in MCGSC is 1, indicating the crystal selected by the EREFS bit
     * has been initialized.*/
#if 0  /* Does not work for Rev 0*/ 
    while (!(MCGSC & MCGSC_OSCINIT_MASK))      
#endif;
  
    /*c) Because RANGE = 1, set DIV32 (bit 4) in MCGC3 to allow access to the proper RDIV bits
     * while in an FLL external mode.*/
    MCGC3 |= MCGC3_DIV32_MASK;

    /*d) MCGC1 = 0x98 (%10011000)
     *– CLKS (bits 7 and 6) set to %10 to select external reference clock as system clock source
     *– RDIV (bits 5-3) set to %011, or divide-by-256 because 8MHz / 256 = 31.25 kHz that is in
     *the 31.25 kHz to 39.0625 kHz range required by the FLL
     *– IREFS (bit 2) cleared to 0, selecting the external reference clock.*/
    MCGC1 = (0b10  << MCGC1_CLKS_BITNUM)  /* CLKS = 10 -> external reference clock.*/
                     | (0b100 << MCGC1_RDIV_BITNUM); /* RDIV = 2^4 -> 25MHz/16 = 1.5625 MHz for PLL*/

#if 0  /* Stuck on double initialisation. */
    while (!(MCGSC & MCGSC_IREFST_MASK)) 
    {};
#endif  

    /* Switch from FBE to PBE (PLL bypassed internal) mode 
     * set PLL multi 50MHz amd select PLL. */
    MCGC3 = (0b1000<< MCGC3_VDIV_BITNUM) | MCGC3_PLLS_MASK;           
              
    /* Wait for PLL status bit and LOCK bit.*/
    while (!(MCGSC & MCGSC_PLLST_MASK) && !(MCGSC & MCGSC_LOCK_MASK))  
    {};
   
    /* Switch from PBE to PEE (PLL enabled external mode)*/
    MCGC1 &= ~(0b11 << MCGC1_CLKS_BITNUM);  

    /* Wait for clock status bits to update */
    while ((MCGSC & MCGSC_CLKST_MASK )!= (0b11<<MCGSC_CLKST_BITNUM))       
    {};
    /* FEE mode entered.*/

#elif XTAL_32KHZ  /*  32 MHz oscilator. FLL Engaged External (FEE) */
    /*  Set FLL Engaged External (FEE) mode.
     *  RM Table 6-10:
     *   MCGOUT is derived from the FLL clock, which is controlled by 
     *   the external reference clock. The FLL clock frequency locks to 
     *   1024 times the external reference frequency, as specified by 
     *   MCGC1[RDIV], MCGC2[RANGE], and MCGC3[DIV32]. 
     *   MCGLCLK is derived from the FLL, and the PLL is disabled in 
     *   a low-power state. 
     */ 

    /* Enable oscillator pins 
     */
    PTDPF1_D4 = 0x3;   /* Enable EXTAL.External clock source is connected 
                        * to the EXTAL input pin.*/
    PTDPF1_D5 = 0x3;   /* Enable XTAL. */
    
    /* The MCG comes out of reset configured for FEI (FLL engaged internal).
     * We need to switch to FEE (FLL engaged external) mode 
     */
    
    /* 1) Enable the external clock source by setting the appropriate bits in MCGC2. 
     */
    MCGC2 = 0
          | MCGC2_EREFS_MASK    /* External Clock Source requested. */
          | MCGC2_ERCLKEN_MASK; /* Enables the external reference clock for use as MCGERCLK. */


    /*A) Loop until OSCINIT (bit 1) in MCGSC is 1, indicating the crystal selected by the EREFS bit
     * has been initialized.*/
    while (!(MCGSC & MCGSC_OSCINIT_MASK))
    {};

    /* 2) Write to MCGC1 to select the clock mode. 
     */
    MCGC1 = (0x0<<6)            /* Leave the CLKS bits at %00 so that the output 
                                 * of the FLL is selected as the system clock source. */
          | (0x0<<3)            /* Set RDIV appropriately */
    #if 0          
          | MCGC1_IRCLKEN_MASK  /* Enables the internal reference clock for use as MCGIRCLK (for RTC). */
    #endif
          | (0x0<<2);           /* Clear the IREFS bit to switch to the external reference. */
   

    /* 3) Wait for the affected bits in the MCGSC register:
     *    Wait here for the OSCINIT bit to become set indicating 
     *    that the external clock source has finished its initialization 
     *    If in FEE mode, check to make sure the IREFST bit is cleared 
     */
    #if 0   /* Does not work for Rev A*/
        while (!MCGSC_OSCINIT || MCGSC_IREFST)      
        {};
    #endif

    /* 4) Write to the MCGC4 register to determine the DCO output (MCGOUT) 
     *    frequency range.
     *    RM. Table 6-9:
     *    DRS=10, DMX2=0, FLL factor=1536:
     *    Referance range = 32.768 kHz => DCO range = 50.33 MHz 
     */      
    MCGC4 = (0x2<<0);           /* FLL by 1536 = 50331648 Hz */
    
    /* 5) Wait for the LOCK bit in MCGSC to become set, indicating 
     *    that the FLL has locked to the new multiplier value designated 
     *    by the DRS and DMX32 bits.
     */
    while (!MCGSC_LOCK)  
    {};

#else /* FLL Engaged Internal (FEI). */

    /* Clock trim. */
	MCGTRM = NVMCGTRM;
	
	/* Select the internal reference clock. */
	MCGC1 = 0 | 
	        MCGC1_IRCLKEN_MASK|  /* Enables the internal reference clock for use as MCGIRCLK. */
	        MCGC1_IREFS_MASK;    /* Internal reference clock selected. */

	while(!MCGSC_IREFST)
	{};

    MCGC2 = 0;      /* Divides selected clock by 1.*/
                    /* Low frequency range selected for the crystal oscillator of 32 kHz to 100 kHz. */
    MCGC3 = 0;      /* Divide-by-32 is disabled
                     * FLL is selected.*/

	MCGC4 = 0x2;    /* DRS = 2; 32768 * 11536 = 50.33MHz */
	while( MCGC4_DRST_DRS | MCGSC_LOCK )
	{};   
#endif                                    
}





