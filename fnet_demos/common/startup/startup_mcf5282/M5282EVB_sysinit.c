
/*
 * File:		m5282evb_sysinit.c
 * Purpose:		Reset configuration of the M5282EVB
 */

#include "support_common.h"
#include "exceptions.h"

/********************************************************************/

#ifdef __cplusplus
#pragma cplusplus off
#endif // __cplusplus

void uart_init();
void pll_init();
void wtm_init();
void scm_init();
void cs_init();
void sdram_init();





/********************************************************************/

/********************************************************************/
void 
wtm_init()
{
	/*
	 * Disable Software Watchdog Timer
	 */
	MCF_WTM_WCR = 0;
}
/********************************************************************/
void 
pll_init()
{
	/*
	 * Multiply 8Mhz reference crystal by 8 to acheive system clock of 64Mhz
	 */
	MCF_CLOCK_SYNCR = MCF_CLOCK_SYNCR_MFD(2);
	
	while (!(MCF_CLOCK_SYNSR & MCF_CLOCK_SYNSR_LOCK))
	{
		
	}
}
/****************************************************************/
void
scm_init()
{
	/* 
	 * Enable on-chip modules to access internal SRAM 
	 */
	MCF_SCM_RAMBAR = (0 
		|	MCF_SCM_RAMBAR_BA(RAMBAR_ADDRESS)
		|	MCF_SCM_RAMBAR_BDE);
}


void
sdram_init()
{
	int i;

	/*
	 * Check to see if the SDRAM has already been initialized
	 * by a run control tool
	 */
	if (!(MCF_SDRAMC_DACR0 & MCF_SDRAMC_DACR_RE))
	{
		/* 
		 * Initialize DRAM Control Register: DCR 
		 */
		MCF_SDRAMC_DCR = (0
			| MCF_SDRAMC_DCR_RTIM_6
			| MCF_SDRAMC_DCR_RC((15 * SYSTEM_CLOCK_KHZ/1000)>>4));

		/* 
		 * Initialize DACR0
		 */
		MCF_SDRAMC_DACR0 = (0
			| MCF_SDRAMC_DACR_BA(SDRAM_ADDRESS)
			| MCF_SDRAMC_DACR_CASL(1)
			| MCF_SDRAMC_DACR_CBM(3)
			| MCF_SDRAMC_DACR_PS_32);
			
		/*
		 * Initialize DMR0
		 */
		MCF_SDRAMC_DMR0 = (0
			| MCF_SDRAMC_DMR_BAM_16M
			| MCF_SDRAMC_DMR_V);

		/*	
		 * Set IP (bit 3) in DACR 
		 */
		MCF_SDRAMC_DACR0 |= MCF_SDRAMC_DACR_IP;

		/* 
		 * Wait 30ns to allow banks to precharge 
		 */
		for (i = 0; i < 5; i++)
		{
			#ifndef __MWERKS__
				asm(" nop");
#else
				asm( nop);
#endif
	}

		/*	
		 * Write to this block to initiate precharge 
		 */
		*(uint32 *)(SDRAM_ADDRESS) = 0xA5A59696;

		/*	
		 * Set RE (bit 15) in DACR 
		 */
		MCF_SDRAMC_DACR0 |= MCF_SDRAMC_DACR_RE;
			
		/* 
		 * Wait for at least 8 auto refresh cycles to occur 
		 */
		for (i = 0; i < 2000; i++)
		{
			#ifndef __MWERKS__
				asm(" nop");
#else
				asm( nop);
#endif
}

		/*	
		 * Finish the configuration by issuing the IMRS. 
		 */
		MCF_SDRAMC_DACR0 |= MCF_SDRAMC_DACR_IMRS;
		
		/*
		 * Write to the SDRAM Mode Register 
		 */
		*(uint32 *)(SDRAM_ADDRESS + 0x400) = 0xA5A59696;
	}
}
/********************************************************************/
void
cs_init()
{
	/* 
	 * ChipSelect 1 - External SRAM 
	 */
	MCF_CS1_CSAR = MCF_CS_CSAR_BA(SDRAM_ADDRESS);
	MCF_CS1_CSCR = MCF_CS_CSCR_AA | MCF_CS_CSCR_PS_32;
	MCF_CS1_CSMR = MCF_CS_CSMR_BAM_512K | MCF_CS_CSMR_V;
    
    /* 
	 * ChipSelect 0 - External Flash 
	 */ 
	MCF_CS0_CSAR = MCF_CS_CSAR_BA(EXT_FLASH_ADDRESS);
	MCF_CS0_CSCR = (0
		| MCF_CS_CSCR_WS(6)
		| MCF_CS_CSCR_AA
		| MCF_CS_CSCR_PS_16);
	MCF_CS0_CSMR = MCF_CS_CSMR_BAM_2M | MCF_CS_CSMR_V;
}
/********************************************************************/


void __initialize_hardware(void)
{
	asm
	{
		

		/* Initialize IPSBAR */
		move.l	#(__IPSBAR + 1),d0
		move.l	d0,0x40000000
		
		/* Initialize FLASHBAR: locate internal Flash and validate it */
		/* Initialize RAMBAR0: This is the FLASHBAR */
		/** this sets bit 6 of the FLASHBAR.  Bit 6 is not documented, however,
		***	Freescale states that it is a workaround for the FLASH speculative
		*** read issues. See Errata data for the PCF5282, mask 0L95M. ***/ 
		move.l	#(__FLASHBAR + 0x161),d0
	    movec d0,FLASHBAR
	}

	/*
	 * Set Port UA to initialize URXD0/URXD1 UTXD0/UTXD1 
	 */
	MCF_PAD_PUAPAR = 0x0F;

    MCF_PAD_PBCDPAR = 0xC0; /* Get access to SDRAM */

	wtm_init();
	pll_init();
	scm_init();
	cs_init();
	sdram_init();

	initialize_exceptions();
}
