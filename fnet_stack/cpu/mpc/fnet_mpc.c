/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2011 by Andrey Butok and Gordon Jahn. Freescale Semiconductor, Inc.
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
* @file fnet_mpc.c
*
* @author Andrey Butok
*
* @date Dec-17-2012
*
* @version 0.1.1.0
*
* @brief CPU-specific API implementation.
*
***************************************************************************/

#include "fnet_cpu.h"

#if FNET_MPC

/************************************************************************
* NAME: fnet_cpu_reset
*
* DESCRIPTION: Initiate software reset.
*************************************************************************/
void fnet_cpu_reset (void)
{
    FNET_MPC_SIU_SRCR = 0x80000000; /* Software System Reset */
}

/************************************************************************
* NAME: fnet_cpu_disable_irq
*
* DESCRIPTION: Disable IRQs
*************************************************************************/
fnet_cpu_irq_desc_t fnet_cpu_irq_disable(void)
{
    fnet_cpu_irq_desc_t oldlevel;


#if 0 /* Not the best way to do this... SWT might depend upon it */
	asm("wrteei 0");
	return 0;
#endif



#if FNET_CFG_CPU_INDEX==0
	oldlevel = FNET_MPC_INTC_CPR_PRC0;
	FNET_MPC_INTC_CPR_PRC0 = 15;
#else
	oldlevel = FNET_MPC_INTC_CPR_PRC1;
	FNET_MPC_INTC_CPR_PRC1 = 15;
#endif

    return oldlevel;
}   

/************************************************************************
* NAME: fnet_cpu_disable_irq
*
* DESCRIPTION: Enables IRQs at interrupt level mask value.
*************************************************************************/
void fnet_cpu_irq_enable(fnet_cpu_irq_desc_t irq_desc)
{	
	/* Enable processor recognition of interrupts. Just in case */
    asm("wrteei 1");                 /* Set MSR[EE]=1  */
    
#if FNET_CFG_CPU_INDEX==0
	FNET_MPC_INTC_CPR_PRC0 = irq_desc;
#else
	FNET_MPC_INTC_CPR_PRC1 = irq_desc;
#endif
	
}

#endif /*FNET_MPC*/
