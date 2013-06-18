/*
 * File:		m52235evb_sysinit.c
 * Purpose:		Reset configuration of the M52235EVB
 *
 * Notes:
 */
#include "support_common.h"
#include "exceptions.h"


 
/********************************************************************/

void wtm_init(void);
void pll_init(void);
void scm_init(void);
void gpio_init(void);

/********************************************************************/
void
wtm_init(void)
{
	/*
	 * Disable Software Watchdog Timer
	 */
	MCF_SCM_CWCR = 0;
}
/********************************************************************/
void
pll_init(void)
{

//MCF_CLOCK_CCHR =0x05; // The PLL pre divider - 25MHz / 5 = 5MHz 

	/* The PLL pre-divider affects this!!! 
	 * Multiply 25Mhz reference crystal /CCHR by 12 to acheive system clock of 60Mhz
	 */

	MCF_CLOCK_SYNCR = MCF_CLOCK_SYNCR_MFD(4) | MCF_CLOCK_SYNCR_CLKSRC| MCF_CLOCK_SYNCR_PLLMODE | MCF_CLOCK_SYNCR_PLLEN ;

	while (!(MCF_CLOCK_SYNSR & MCF_CLOCK_SYNSR_LOCK))
	{
	}
}
/********************************************************************/
void
scm_init(void)
{
	/*
	 * Enable on-chip modules to access internal SRAM
	 */
	MCF_SCM_RAMBAR = (0
		| MCF_SCM_RAMBAR_BA(RAMBAR_ADDRESS)
		| MCF_SCM_RAMBAR_BDE);
}
/********************************************************************/
void
gpio_init(void)
{
  


	/*
	 * Set Port UA to initialize URXD0/UTXD0
	 */
    MCF_GPIO_PUAPAR = 0
        | MCF_GPIO_PUAPAR_URXD0_URXD0
        | MCF_GPIO_PUAPAR_UTXD0_UTXD0;

    MCF_GPIO_PUBPAR = 0
        | MCF_GPIO_PUBPAR_URXD1_URXD1
        | MCF_GPIO_PUBPAR_UTXD1_UTXD1;

    MCF_GPIO_PUCPAR = 0
        | MCF_GPIO_PUCPAR_URXD2_URXD2
        | MCF_GPIO_PUCPAR_UTXD2_UTXD2;


	/*
	 * Initialize PLDPAR to enable Ethernet Leds
	 */
  MCF_GPIO_PLDPAR = (0
		 | MCF_GPIO_PLDPAR_ACTLED_ACTLED 
		 | MCF_GPIO_PLDPAR_LINKLED_LINKLED 
		 | MCF_GPIO_PLDPAR_SPDLED_SPDLED 
		 | MCF_GPIO_PLDPAR_DUPLED_DUPLED 
		 | MCF_GPIO_PLDPAR_COLLED_COLLED 
		 | MCF_GPIO_PLDPAR_RXLED_RXLED   
		 | MCF_GPIO_PLDPAR_TXLED_TXLED);  

#if 0 /* Implemented in the Ethernet driver.*/
{		  
    uint32 myctr; 		//generic counter variable
    // set phy address to zero
	MCF_EPHY_EPHYCTL1 = MCF_EPHY_EPHYCTL1_PHYADD(FEC_PHY0);

    //Enable EPHY module with PHY clocks disabled
    //Do not turn on PHY clocks until both FEC and EPHY are completely setup (see Below)
    MCF_EPHY_EPHYCTL0 = (uint8)(MCF_EPHY_EPHYCTL0_DIS100 | MCF_EPHY_EPHYCTL0_DIS10);
  
    //Enable auto_neg at start-up
    MCF_EPHY_EPHYCTL0 = (uint8)(MCF_EPHY_EPHYCTL0 & (MCF_EPHY_EPHYCTL0_ANDIS));

    //Enable EPHY module
    MCF_EPHY_EPHYCTL0 = (uint8)(MCF_EPHY_EPHYCTL0_EPHYEN | MCF_EPHY_EPHYCTL0);
    //Let PHY PLLs be determined by PHY
    MCF_EPHY_EPHYCTL0 = (uint8)(MCF_EPHY_EPHYCTL0  & ~(MCF_EPHY_EPHYCTL0_DIS100 | MCF_EPHY_EPHYCTL0_DIS10)); 
	
    //DELAY, Delay start-up
    for (myctr=200000; myctr >0; myctr--)
    {
    }
}
#endif  
}

  


/********************************************************************/
void __initialize_hardware(void)
{
	/*******************************************************
	*	Out of reset, the low-level assembly code calls this 
	*	routine to initialize the MCF52235 modules for the  
	*	M52235EVB board. 
	********************************************************/

	asm 
	{
	    /* Initialize IPSBAR */
	    move.l  #__IPSBAR,d0
	       andi.l  #0xC0000000,d0 // need to mask
	    add.l   #0x1,d0
	    move.l  d0,0x40000000

	    

	    /* Initialize FLASHBAR */
	    move.l  #__FLASHBAR,d0
	       andi.l  #0xFFF80000,d0 // need to mask
	    add.l   #0x61,d0
	    movec   d0,FLASHBAR

	}

    /* 
     * Allow interrupts from ABORT, SW1, SW2, and SW3 (IRQ[1,4,7,11]) 
     */
     
    /* Enable IRQ signals on the port */
    MCF_GPIO_PNQPAR = 0
        | MCF_GPIO_PNQPAR_IRQ1_IRQ1
        | MCF_GPIO_PNQPAR_IRQ4_IRQ4
        | MCF_GPIO_PNQPAR_IRQ7_IRQ7;
    
    MCF_GPIO_PGPPAR = 0
        | MCF_GPIO_PGPPAR_IRQ11_IRQ11;
    
    /* Set EPORT to look for rising edges */
    MCF_EPORT_EPPAR(0) = 0
        | MCF_EPORT_EPPAR_EPPA1_RISING
        | MCF_EPORT_EPPAR_EPPA4_RISING
        | MCF_EPORT_EPPAR_EPPA7_RISING;
        
    MCF_EPORT_EPPAR(1) = 0
        | MCF_EPORT_EPPAR_EPPA11_RISING;
        
    /* Clear any currently triggered events on the EPORT  */
    MCF_EPORT_EPIER(0) = 0
        | MCF_EPORT_EPIER_EPIE1
        | MCF_EPORT_EPIER_EPIE4
        | MCF_EPORT_EPIER_EPIE7;
       
    MCF_EPORT_EPIER(1) = 0
        | MCF_EPORT_EPIER_EPIE11;
       
    /* Enable interrupts in the interrupt controller */
    MCF_INTC0_IMRL &= ~(0
        | MCF_INTC_IMRL_INT_MASK1 
        | MCF_INTC_IMRL_INT_MASK4 
        | MCF_INTC_IMRL_INT_MASK7 
        | MCF_INTC_IMRL_MASKALL);

    MCF_INTC1_IMRH &= ~(0
        | MCF_INTC_IMRH_INT_MASK35);
        
	MCF_INTC1_ICR35 = MCF_INTC_ICR_IL(4);

	MCF_GPIO_PDDPAR = 0x0F;
	
	/* Set real time clock freq */

	MCF_CLOCK_RTCDR = 25000000;
	
	
	wtm_init();
	pll_init();
	scm_init();
	gpio_init();

	initialize_exceptions();
}
