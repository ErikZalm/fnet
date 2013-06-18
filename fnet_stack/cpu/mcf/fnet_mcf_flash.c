/**************************************************************************
* 
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License Version 3 
* or later (the "LGPL").
*
* As a special exception, the copyright holders of the FNET project give you
* permission to link the FNET sources with independent modules to produce an
* executable, regardless of the license terms of these independent modules,
* and to copy and distribute the resulting executable under terms of your 
* choice, provided that you also meet, for each linked independent module,
* the terms and conditions of the license of that module.
* An independent module is a module which is not derived from or based 
* on this library. 
* If you modify the FNET sources, you may extend this exception 
* to your version of the FNET sources, but you are not obligated 
* to do so. If you do not wish to do so, delete this
* exception statement from your version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* and the GNU Lesser General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_mcf_flash.c
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.2.0
*
* @brief ColdFire Flash Module driver.
*
***************************************************************************/
#include "fnet.h" 
#include "fnet_stdlib.h"


#if FNET_MCF && FNET_CFG_CPU_FLASH

#if (FNET_CFG_CPU_FLASH_PROGRAM_SIZE != 4) 
    #error "MCF Flash driver supports only 4 and 8 size of program-block"
#endif 

/************************************************************************
* NAME: cfm_command
*
* DESCRIPTION: CFM command 
************************************************************************/
/* == Should be in the RAM ==*/
#if FNET_CFG_COMP_CW
    __declspec(data) 
#endif
#if FNET_CFG_COMP_IAR 
    #pragma segment="FNET_RAMFUNC"
    #pragma location = "FNET_RAMFUNC"
#endif
/************************************************************************
* NAME: _cfm_command_lunch_inram
*
* DESCRIPTION: Launch the command. It must be in RAM.
************************************************************************/
static void _cfm_command_lunch_inram(void)
{
  
    /* Clear CBEIF flag by writing a 1 to CBEIF to launch the command.*/
	FNET_MCF_CFM_CFMUSTAT = FNET_MCF_CFM_CFMUSTAT_CBEIF;
	
	/* The CBEIF flag is set again indicating that the address, data, 
	 * and command buffers are ready for a new command write sequence to begin.*/
	while( !(FNET_MCF_CFM_CFMUSTAT & (FNET_MCF_CFM_CFMUSTAT_CBEIF|FNET_MCF_CFM_CFMUSTAT_CCIF)))
	{};
}

/************************************************************************
* NAME: cfm_command
*
* DESCRIPTION: CFM command 
************************************************************************/
static void cfm_command( unsigned char command, unsigned long *address, unsigned long data )
{
    fnet_cpu_irq_desc_t irq_desc;
    
    irq_desc = fnet_cpu_irq_disable();
    
    /* If the CFMCLKD register is written, the DIVLD bit is set. */
    if((FNET_MCF_CFM_CFMCLKD & FNET_MCF_CFM_CFMCLKD_DIVLD) == 0)
    {
        /* CFM initialization. */ 
        
        /* Prior to issuing any command, it is necessary to set 
	     * the CFMCLKD register to divide the internal bus frequency 
	     * to be within the 150- to 200-kHz range.
	     * NOTE: Setting CFMCLKD to a value such that FCLK < 150 KHz 
	     * can destroy the flash memory due to overstress. 
	     * Setting CFMCLKD to a value such that FCLK > 200 KHz can 
	     * result in incomplete programming or erasure of the
	     * flash memory array cells.*/
        if ((FNET_CFG_CPU_CLOCK_HZ/2) > 12800000) /* For bus frequencies greater than 12.8 MHz */
            FNET_MCF_CFM_CFMCLKD = FNET_MCF_CFM_CFMCLKD_DIV((FNET_CFG_CPU_CLOCK_HZ/2)/8/200000)| FNET_MCF_CFM_CFMCLKD_PRDIV8;
        else
            FNET_MCF_CFM_CFMCLKD = FNET_MCF_CFM_CFMCLKD_DIV((FNET_CFG_CPU_CLOCK_HZ/2)/200000);
    }
   
    /* Write to one or more addresses in the flash memory.*/
#if !FNET_CFG_MCF_V1    /* Use the backdoor address. */
    address = (unsigned long *)(__IPSBAR+0x04000000+(unsigned long)address);
#endif
    /* Use the frontdoor address. */
    *address = data;

	/* Write a valid command to the CFMCMD register. */
    FNET_MCF_CFM_CFMCMD = command;
	
    _cfm_command_lunch_inram();	
    
    fnet_cpu_irq_enable(irq_desc);
}

/************************************************************************
* NAME: fnet_cpu_flash_erase
*
* DESCRIPTION:
************************************************************************/
void fnet_cpu_flash_erase( void *flash_page_addr)
{
    cfm_command( FNET_MCF_CFM_CFMCMD_PAGE_ERASE, flash_page_addr, 0);
}

/************************************************************************
* NAME: fnet_cpu_flash_write
*
* DESCRIPTION:
************************************************************************/
void fnet_cpu_flash_write(unsigned char *dest, unsigned char *data)
{
    cfm_command(FNET_MCF_CFM_CFMCMD_WORD_PROGRAM, (unsigned long *)dest, *((unsigned long *)data));
}

#endif /* FNET_MCF && FNET_CFG_CPU_FLASH */
