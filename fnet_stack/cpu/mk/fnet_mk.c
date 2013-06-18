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
* @file fnet_mk.c
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.7.0
*
* @brief CPU-specific API implementation.
*
***************************************************************************/
#include "fnet.h"

#if FNET_MK 


/************************************************************************
* NAME: fnet_cpu_reset
*
* DESCRIPTION: Initiate software reset.
*************************************************************************/
void fnet_cpu_reset (void)
{
    /* Application Interrupt and Reset Control Register.*/
    /* To write to this register, you must write 0x5FA to the VECTKEY field.*/
    FNET_MK_SCB_AIRCR = FNET_MK_SCB_AIRCR_VECTKEY(0x5fa)|FNET_MK_SCB_AIRCR_SYSRESETREQ_MASK; /* Asserts a signal to the outer system that requests a reset.*/
 
}

/************************************************************************
* NAME: fnet_cpu_irq_disable
*
* DESCRIPTION: Disable IRQs
*************************************************************************/
fnet_cpu_irq_desc_t fnet_cpu_irq_disable(void)
{
     return fnet_mk_irq_disable();  
}

/************************************************************************
* NAME: fnet_cpu_irq_enable
*
* DESCRIPTION: Enables IRQs.
*************************************************************************/
void fnet_cpu_irq_enable(fnet_cpu_irq_desc_t irq_desc)
{
    fnet_mk_irq_enable(irq_desc);
}

/************************************************************************
* NAME: fnet_mk_periph_clk_khz
*
* DESCRIPTION: Kinetis peripheral clock in KHZ.
*************************************************************************/
unsigned long fnet_mk_periph_clk_khz(void)
{
    return (FNET_CPU_CLOCK_KHZ / (((FNET_MK_SIM_CLKDIV1 & FNET_MK_SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1));
}

#endif /*FNET_MK*/
